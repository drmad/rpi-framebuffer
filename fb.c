#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#include <linux/fb.h>
#include <linux/kd.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/ioctl.h> // for ioctl

#include "fb.h"

/** Inicializa el framebuffer, y un buffer intermedio.
 */ 
int fb_init( char bpp ) {
    fb_fd = open ( "/dev/fb0", O_RDWR );
    if (fb_fd == -1) {
        printf("Error: No puede abrir el framebuffer!\n");
        return(1);
    }
    
    // Obtenemos info 
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
        printf("No pude obtener la información variable del framebuffer!\n");
    }
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_finfo)) {
        printf("No pude obtener la información fija del framebuffer!\n");
    }
    
    // Cambiamos al BPP pedido
    fb_vinfo.bits_per_pixel = bpp;
    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &fb_vinfo)) {
        printf("No pude cambiar el tamaño del pixel.\n");
    }

    // Guardamos la resolución y los bpp
    fb_sx = fb_vinfo.xres;
    fb_sy = fb_vinfo.yres;
    fb_bpp = fb_vinfo.bits_per_pixel;
    

    printf("Pantalla: %dx%d, %d bpp\n", 
         fb_vinfo.xres, fb_vinfo.yres, 
         fb_vinfo.bits_per_pixel );

    fb_p = (char *)mmap (0, fb_finfo.smem_len,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fb_fd, 0 );

    // Creamos un buffer
    fb_buffer = malloc ( fb_finfo.smem_len );

    // Lo limpiamos a negro
    memset ( fb_buffer, 0, fb_finfo.smem_len ) ;
    
    return 0;
}

/** Coloca una paleta de colores en el framebuffer. Solo para 8 bpp
 */
void fb_putpal ( unsigned short r[], unsigned short g[], unsigned short b[] ) { 
    struct fb_cmap pal;
    pal.start  = 0;
    pal.len = 256;
    
    pal.red = r;    
    pal.green = g;
    pal.blue = b;
    pal.transp = 0; // No lo usuamos.

    if (ioctl(fb_fd, FBIOPUTCMAP, &pal)) {
        printf("Error setting palette.\n");
    }
}


void fb_close() {
    // Cerramos todo;
    free ( fb_buffer );
 
    munmap(fb_p, fb_finfo.smem_len);
    close(fb_fd);

}

/**
 *  Dibuja un pixel en 32 bpp
 */
inline void fb_pixel32 ( int x, int y, unsigned char color[] ) {
    int ofs = y * fb_finfo.line_length + (x << 2);

    // Copiamos los 4 bytes sin asco.
    memcpy ( (char *)(fb_buffer + ofs), color, 4 );
}
inline void fb_getpixel32 ( int x, int y, unsigned char *color ) {
    int ofs = y * fb_finfo.line_length + (x << 2);

    // Copiamos los 4 bytes sin asco.
    memcpy ( color, (char *)(fb_buffer + ofs), 4 );
}

inline void fb_pixel8 ( int x, int y, unsigned char color ) {
    *( (char *)(fb_buffer + y * fb_finfo.line_length + x) ) = color;
}
inline char fb_getpixel8 ( int x, int y ) {
    int ofs = y * fb_finfo.line_length + x;

    return *( (char *)(fb_buffer + ofs) ) ;
}

/** 
 *  Mueve el buffer al framebuffer. Esto necesita optimización
 */
inline void fb_draw() {
    
    memcpy ( fb_p, fb_buffer, fb_finfo.smem_len );
}



