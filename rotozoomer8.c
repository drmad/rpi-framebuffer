#include <stdio.h>
#include <string.h>

#include <sys/time.h>

#include "fb.h"

#include <stdlib.h> // rand()
#include <math.h>   
#include <fcntl.h>  // open()
#include <sys/stat.h> 
#include <unistd.h>

#include <sys/ioctl.h>

#ifdef PNG
#include <png.h>
#include <zlib.h>
#endif


// Cuando se descomenta la siguiente línea, muestra en la consola el tiempo
// en microsegundos que toma ejecutar ciertas partes de este programa. 
// #define PROFILING

// Cantidad de numeros random para el lookup table
#define MAXRAND 10240

// Teh PI
#define PI 3.141592653589793

// lookup tables
int lt_sin[256];
int lt_cos[256];

#ifdef PNG
int pngserial = 0;
#endif

float decrad(float dec) 
{
    return dec * PI / 180;
}
float raddec(float rad)
{
    return rad * 180 / PI;
}

int main(int argc, char* argv[]) {

    // Variables de apoyo.
    int i, x, y;   
    int randptr = 0;

    fb_init(8);

    // Limpiamos la pantalla
    memset(fb_buffers[0], 0, fb_size);
    
    // Creamos un par de buffers de trabajo
    unsigned char * buffer1 = malloc(fb_size);
    unsigned char * buffer2 = malloc(fb_size);

    unsigned char *buf_work = buffer1;
    unsigned char *buf_back = buffer2;



    printf("Calculando lookup tables...\n");
    for (i = 0; i < 256; i++) {
        lt_sin[i] = sin(decrad(i * 360/256)) * 4096;
        lt_cos[i] = cos(decrad(i * 360/256)) * 4096;
    }

    // Rampa de colores. Unsigned Short, por que eso es lo que quiere el 
    // framebuffer
    unsigned short r[256], g[256], b[256];
   
    // lookuptable para el rotozoomer
    unsigned int roto_f[fb_size];
    unsigned int roto_b[fb_size];
    unsigned int *roto;

    // Centro de la pantalla
    int cx = fb_sx / 2;
    int cy = fb_sy / 2;
    float d;
    for (i = 0; i< fb_size; i++) {
        x = i % fb_sx;
        y = i / fb_sx;

        // coordenadas centradas
        int xx = x - cx;
        int yy = y - cy;  

        d = sqrt(pow(xx, 2) + pow(yy, 2));
        d /= 1.05;

        float ang = atan2(xx, yy);
        float rotate_factor = d / 150 * 0.0175433;
        float f_ang = ang + rotate_factor;
        
        x = (int)(d * sin(f_ang)) + cx;
        y = (int)(d * cos(f_ang)) + cy;
   
        roto_f[i] = y * fb_sx + x;

        float b_ang = ang - rotate_factor;

        x = (int)(d * sin(b_ang)) + cx; 
        y = (int)(d * cos(b_ang)) + cy; 

        roto_b[i] = y * fb_sx + x;

        if (roto_f[i] > fb_size){
            roto_f[i] = fb_size;
        }
        if (roto_b[i] > fb_size) {
            roto_b[i] = fb_size;
        }


    }

    roto = roto_f;


    // Creamos una rampa de colores bonita.
    for ( i = 0; i < 64; i++ ) {
        // Negro a rojo
        b[i] = (i * 4) << 8;
        g[i] = 0;
        r[i] = 0;
        
        // rojo a naranja
        b[64+i] = 255 << 8;
        g[64+i] = (i * 2) << 8;
        r[64+i] = 0;

        // Naranja a amarillo
        b[128+i] = 255 << 8;
        g[128+i] = (128  + i * 2) << 8;
        r[128+i] = 0;

        // amarillo a blanco
        b[192+i] = 255 << 8;
        g[192+i] = 255 << 8;
        r[192+i] = (i * 4) << 8;
    }
    
    // Colocamos el color_ramp en el framebuffer
    fb_putpal (r, g, b);

    // Un lookup table de valores randoms
    int R[MAXRAND];
    
    for (i = 0; i < MAXRAND; i++) {
        R[i] = (rand() % fb_size);
    }

    #ifdef PROFILING    
    struct timeval stop,start;
    #endif
    
    printf("Go!\n");

    char count = 0;

    // Pointers a seno y coseno de cada coordenada, para lissajousear un poco
    unsigned char ptr_x = 0;
    unsigned char ptr_y = 0;

    while (1) {

        #ifdef PROFILING    
        gettimeofday (&start, NULL );
        #endif

        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Noise: %lu -", stop.tv_usec - start.tv_usec );
        #endif
        
        #ifdef PROFILING    
        gettimeofday (&start, NULL );
        #endif

        // Unos pixelito al azar
        for (i = 0; i < 100; i++){
            memset(buf_work + R[randptr], 255, 10);
            randptr = (randptr + 1) % MAXRAND;
        }
       
        // TEH BOX
        int lx = cx + ((180 * lt_sin[ptr_x]) >> 12);
        int ly = cy + ((100 * lt_cos[ptr_y]) >> 12);
    
        ptr_x += 4;
        ptr_y += 6;

        int boxsize = 50 + (R[randptr] % 20 - 10);

        unsigned char c1 = R[randptr];
        randptr = (randptr + 1) % MAXRAND;
        unsigned char c2 = R[randptr];
        randptr = (randptr + 1) % MAXRAND;

        for (i = 0; i < boxsize; i++) {
            memset(buf_work + (ly + i) * fb_sx + lx, c1, boxsize);
            memset(buf_work + (fb_sy - ly + i) * fb_sx + (fb_sx - lx), c2, boxsize);
        }

        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Logo: %lu -", stop.tv_usec - start.tv_usec );
        #endif

  
        #ifdef PROFILING
        gettimeofday (&start, NULL );
        #endif
        
        for (i = 0; i < fb_size; i++) {
            // ROTOZOOMAH
            char *p = (char *)(buf_work + *(roto + i));

            // BLUR
            int c = *(p-1) + *(p) + *(p+1) + *(p+fb_sx); // Menos fade
            c >>= 2;

            *(buf_back + i) = (unsigned char)c;
        }

        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Draw: %lu -", stop.tv_usec - start.tv_usec );
        #endif

        
        #ifdef PROFILING
        gettimeofday (&start, NULL );
        #endif

        memcpy(fb_buffers[0], buf_work, fb_size);

        #ifdef PNG
        char filename[100];
        sprintf (filename, "output/%09u.png", pngserial++);

        FILE *fp = fopen(filename, "wb");

        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
        NULL, NULL, NULL);

        png_infop info = png_create_info_struct(png);

        setjmp(png_jmpbuf(png));

        png_init_io(png, fp);

        png_set_IHDR(png, info, 
            fb_sx,
            fb_sy,
            8, 
            PNG_COLOR_TYPE_PALETTE,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );

        // La paleta de colores
        unsigned paletteSize = 256;
        //png_color* palette = (png_color*)png_malloc(png, paletteSize  * sizeof(png_color));
        png_color palette[paletteSize];
    
        for (i = 0; i < paletteSize; i++) {
            palette[i].red = r[i] >> 8;
            palette[i].green = g[i] >> 8;
            palette[i].blue = b[i] >> 8;
        }
        png_set_PLTE(png, info, palette, paletteSize);
       
        png_bytep *row_pointers[fb_sy];

        for (i = 0; i < fb_sy; i++) {
            row_pointers[i] = (png_bytep *)(buf_work + i * fb_sx);
        }
        
        png_write_info(png, info);

        png_write_image(png, row_pointers);
        png_write_end(png, NULL);
        fclose(fp);

        if (png && info) {
            png_destroy_write_struct(&png, &info);
        }

        printf("%s\n", filename);

        #endif

        /*
        fb_buffer_idx = (fb_buffer_idx + 1) % 3;

        // Rotamos los buffers
        buf_screen = fb_buffers[fb_buffer_idx];
        buf_work = fb_buffers[(fb_buffer_idx + 1) % 3];
        buf_back = fb_buffers[(fb_buffer_idx + 2) % 3];

        // Aaaannd Panning!
        fb_vinfo.yoffset = fb_sy * fb_buffer_idx;
        ioctl(fb_fd, FBIOPAN_DISPLAY, &fb_vinfo);
        */


        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Blit: %lu \n", stop.tv_usec - start.tv_usec );
        #endif

        // Swap!
        unsigned char *b;
        b = buf_work;
        buf_work = buf_back;
        buf_back = b;

        count++;
        if (count == 100) {
            roto = roto_b;
        }

        if (count == 200) {
            roto = roto_f;
            count = 0;
        }
    }
    
    // Bu-bye!
    fb_close();
    return 0;
   
}
