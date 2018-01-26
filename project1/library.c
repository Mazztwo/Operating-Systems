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

// 16 unsigned bits to represent color.
// Index  |  Color  |  Value
//  0-4   |   blue  |   0-31 
//  5-10  |   green |   0-63
//  11-15 |   red   |   0-31
color_t encode_color(int r, int g, int b)
{
    // Could error check to make sure
    // r & b are between 0-31 and g is
    // between 0 - 63...

    color_t rgb = 0;

    // Extract first few bits of each color
    char red = r & 0x1F;
    char green = g & 0x3F;
    char blue = b & 0x1F;
    
    // Set upper 5 bits
    unsigned short mask = 0xFFFF;
    rgb = (red << 11) & mask;

    // Set middle 6 bits
    rgb = (green << 5) & mask;

    // Set last 5 bits
    rgb = rgb & blue;

    return rgb;
}

// DEBUG FUNCTIONS:
void print_binary(color_t number)
{
    if (number) {
        print_binary(number >> 1);
        putc((number & 1) ? '1' : '0', stdout);
    }
}
