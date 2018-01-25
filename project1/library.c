// Library file for Project1
// Alessio Mazzone

#include <stdio.h>
#include "library.h"
#include <fcntl.h> // contains open() syscall
#include <sys/mman.h> // contains mmap() syscall

// Initialize graphics library
void init_graphics()
{
    // Use open() syscall to access frame buffer for read/write
    int bufferFile;
    void *framebuffer;
    
    bufferFile = open("/dev/fb0", O_RDWR);

    if(bufferFile < 0)
    {
        printf("\nError opening framebuffer.\n");
        return;
    }

    // Need len of buffer --> need resolution for this
    //framebuffer = mmap(0, len, MAP_SHARED)


    
   printf("\nHelloooo\n");


}

color_t encode_color(int r, int g, int b)
{
    color_t rgb = 0;

    return rgb;
}
