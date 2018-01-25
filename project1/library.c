// Library file for Project1
// Alessio Mazzone

#include <stdio.h>
#include <fcntl.h> // contains open() syscall

// Initialize graphics library
void init_graphics()
{
    // Use open() syscall to access frame buffer for read/write
    int framebuffer = open("/dev/fb0", O_RDWR);

    if(framebuffer < 0)
    {
        printf("\nError opening framebuffer.\n");
        return;
    }
    
   printf("\nHelloooo\n");


}
