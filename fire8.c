#include <stdio.h>
#include <string.h>

#include <sys/time.h>

#include "fb.h"

#include <stdlib.h> // rand()

#include <fcntl.h>  // open()
#include <sys/stat.h> 
#include <unistd.h>


// Cuando se descomenta la siguiente línea, muestra en la consola el tiempo
// en microsegundos que toma ejecutar ciertas partes de este programa. 
// #define PROFILING

// Cantidad de numeros random para el lookup table
#define MAXRAND 10240


int main(int argc, char* argv[]) {

    // Rampa de colores. Unsigned Short, por que eso es lo que quiere el 
    // framebuffer
    unsigned short r[256], g[256], b[256];
    
    // Variables de apoyo.
    int i, x, y;   
    int tmpval;
    char *p;

    // Inicializamos el framebuffer.
    fb_init(8);

    // Creamos un buffer de trabako
    unsigned char *buffer = malloc(fb_size);
    fb_buffer_pointer = buffer;

    // Creamos una rampa de colores bonita.
    for (i = 0; i < 64; i++) {
        // Negro a rojo
        b[i] = 0;
        g[i] = 0;
        r[i] = (i * 4) << 8;
        
        // rojo a naranja
        b[64+i] = 0;
        g[64+i] = (i * 2) << 8;
        r[64+i] = 255 << 8;

        // Naranja a amarillo
        b[128+i] = 0;
        g[128+i] = (128  + i * 2) << 8;
        r[128+i] = 255 << 8;

        // amarillo a blanco
        b[192+i] = ( i * 4) << 8;
        g[192+i] = 255 << 8;
        r[192+i] = 255 << 8;
    }
    
    // El 255 es negro, para que se note el logo
    r[254] = 0;
    g[254] = 0;
    b[254] = 0;

    // Colocamos el color_ramp en el framebuffer
    fb_putpal ( r, g, b );

    // Un lookup table de valores randoms, 
    unsigned int rf[MAXRAND];
    int rf_c = 0;
   
    for ( i = 0; i < MAXRAND; i++ ) {
        rf[i] = rand();
    }

    // Puntero a la memoria donde almacenaremos el loguito mono.
    unsigned char * logo;
    
    // Tamaño fijo. Por ahora.
    int logo_w = 445;
    int logo_h = 522; 
    
    // Obtenemos un pedazo de memoria
    logo = malloc(logo_w * logo_h);
    
    // Abriemos el fichero de datos, y lo leemos
    int f = open ( "fury-road.dat", O_RDONLY);
    read (f, logo, logo_w * logo_h);
    close (f);
    

    // Posición inicial de logo
    int logo_x = fb_sx / 2 - logo_w / 2;
    int logo_y = fb_sy / 2 - logo_h / 2;
    int lc;             // Esta variable la usamos para promediar los colores del ogo
    int logo_dir = 0;   // Dirección de la animación. Empieza quieto.
    
    
    // Valores para animar el fuego inicialmente
    int fire_anim = 1;
    int fire_anim_delay = 0;
    int fire_anim_top = 100;
    
    // También animamos el logo
    unsigned char logo_state = 0;
    int logo_timer = 0;

    #ifdef PROFILING    
    struct timeval stop,start;
    #endif
    
    while (1) {

        #ifdef PROFILING    
        gettimeofday (&start, NULL );
        #endif

        
        // Dibujamos los pixeles que generarán el fuego
        for (i = 0; i < 100; i++) {
            x = rf[rf_c] % fb_sx;
            rf_c = ++rf_c % MAXRAND;

            // Esto le da un poquito de randomness a la velocidad de
            // crecimiento del fuego.
            if (fire_anim < fb_sx) {
                if (fire_anim_delay++ > fire_anim_top ) {
                    fire_anim_delay = 0;

                    fire_anim_top = rf[rf_c] % 100;
                    rf_c = ++rf_c % MAXRAND;

                    fire_anim++;                
                }
            }

            // Evitamos que dibuje el fuego más allá del límite de la animación
            if (x > fire_anim) continue;

            // Dibujamos 3 pixeles directo a la memoria. Más rápido que
            // usar tres PIXEL8            
            memset ((char *)(buffer + (fb_sy - 1) * fb_finfo.line_length + x - 1), 255, 3);
        }
        
        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Noise: %lu -", stop.tv_usec - start.tv_usec );
        #endif
        
        #ifdef PROFILING    
        gettimeofday (&start, NULL );
        #endif

        // Dibujamos el logo si está activo.
        if (logo_state == 1) {
            for (y = 0; y < logo_h; y++) {
                for (x = 0; x < logo_w; x++) {

                    // Al color del fondo le añadimos el color del logo. Esto
                    // lo hará transparente, con algo de alias.
                    lc = GETPIXEL8(x + logo_x, y + logo_y) + *(logo + y * logo_w + x);
                    if ( lc > 255 ) lc = 255;
                    
                    // Regresamos el pixel
                    PIXEL8(x + logo_x, y + logo_y, (char)lc);
                    
                }
            }
        }
        
        // Movemos el logo
        logo_x += logo_dir;
        
        // Evitamos que salga de los límites.
        if (logo_x < 0) {
            logo_x = 0;
            logo_dir = -logo_dir;
        }
        if (logo_x > fb_sx - logo_w) {
            logo_x = fb_sx - logo_w;
            logo_dir = -logo_dir;
        }
        
        // Cada 300 frames, cambiamos el estado del logo
        if (logo_timer++ > 300) {
            logo_timer = 0;
            logo_state = !logo_state; 
            
            // También le cambiamos de posición
            logo_x = rf[rf_c] % (fb_sx - logo_w);
            rf_c = ++rf_c & MAXRAND;
            
            // Y de dirección
            logo_dir = (rand() % 2) ? 1 : -1;
            
            // Añadimos un poco de brillo a la pantalla
            for (y = 0; y < fb_sy; y++) {
                for (x = 0; x < fb_sx; x++) {
                    
                    lc = GETPIXEL8(x, y) + 50 + (rf[rf_c] % 128);
                    rf_c = ++rf_c % MAXRAND;

                    if ( lc > 254 ) lc = 254;   // 254 para no usar el negro de 255
                    
                    PIXEL8 ( x, y, (char)lc);
                }
            }    
        }
               
        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Logo: %lu -", stop.tv_usec - start.tv_usec );
        #endif

  
        #ifdef PROFILING
        gettimeofday (&start, NULL );
        #endif
        
        // Ahora analizamos todo el buffer para calcular el 
        // efecto de fuego. Esto necesita más optimación
        for ( y = 0; y < fb_sy; y++ ) {
            for ( x = 0; x < fb_sx; x++ ) {
            
                // Solía tener un IF aqui para evitar leer los píxeles fuera
                // de los límites de la pantalla. Pero al quitarlos funcionó
                // bien, y la velocidad de este bucle bajó de 28ms a 18ms en
                // el Raspberry Pi :)
                
                // También solia usar 3 veces la función GETPIXEL8. Pero usando
                // aritmética de punteros, la velocidad bajó 1ms más. A parte,
                // es más paja :D
                
                p = (char *)(buffer + y * fb_finfo.line_length + x);
                tmpval = *p;
                p += fb_finfo.line_length - 1;
                tmpval += *(p++);
                tmpval += *(p++);
                tmpval += *(p++);
                
                // Le restamos 1 para ir oscureciendo
                if (tmpval) tmpval--;
                
                // Y dividimos todo entre 4,
                tmpval >>= 2;

                // Dibujamos el pixel de vuelta al buffer
                PIXEL8(x,y,tmpval);
            }

        }

        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Draw: %lu -", stop.tv_usec - start.tv_usec );
        #endif

        
        #ifdef PROFILING
        gettimeofday (&start, NULL );
        #endif

        // Y copiamos el buffer al framebuffer. Esto hace un memcpy(), pero
        // creo que hay una forma más eficiente de hacerlo ("imageblit"?)
        
        // TODO
        DRAW();

        #ifdef PROFILING
        gettimeofday (&stop, NULL );
        printf ( "Blit: %lu \n", stop.tv_usec - start.tv_usec );
        #endif
    }
    
    // Bu-bye!
    fb_close();
    return 0;
   
}
