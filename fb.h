#include <linux/fb.h>

int fb_sx;
int fb_sy;
int fb_size;
int fb_bpp;

// Buffer. Siempre usamos buffer
unsigned char *fb_buffers[3];

// Puntero al buffer donde afectará las macros
unsigned char *fb_buffer_pointer;

int fb_fd;
struct fb_var_screeninfo fb_vinfo;
struct fb_fix_screeninfo fb_finfo;

inline void fb_pixel32 ( int x, int y, unsigned char color[] );

int fb_init( char bpp );
void fb_draw();
void fb_close();
void fb_putpal ( unsigned short r[], unsigned short g[], unsigned short b[] );

inline void fb_pixel8 ( int x, int y, unsigned char color );
char fb_getpixel8 ( int x, int y );

// Versiones en línea. Mucho más rápido
#define GETPIXEL8(x,y) *((char *)(fb_buffer_pointer+(y) * fb_finfo.line_length+(x)))
#define PIXEL8(x,y,c) *((char *)(fb_buffer_pointer+(y) * fb_finfo.line_length+(x))) = (c)
#define DRAW() memcpy (fb_buffers[0], fb_buffer_pointer, fb_finfo.smem_len)
