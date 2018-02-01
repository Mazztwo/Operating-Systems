// Library file for Project1
// Alessio Mazzone

#include <stdio.h>
#include "library.h"
#include <fcntl.h>              // contains open() syscall
#include <sys/mman.h>           // contains mmap() syscall
#include <sys/ioctl.h>          // contains ioctl() syscall
#include <sys/select.h>         // contains select() syscall
#include <unistd.h>             // contains write() syscall
#include <time.h>               // contains nanosleep() syscall
#include <termios.h>            // contains terminal struct
#include <linux/fb.h>           // contains fb structs
#include <sys/types.h>
#include <sys/stat.h>




int bufferFile;
int bufferSize;
struct fb_var_screeninfo virtualResolution;
struct fb_fix_screeninfo bitDepth;

void *framebuffer;
struct termios terminalSettings;


// Initialize graphics library
void init_graphics()
{
    // Open frame buffer
    bufferFile = open("/dev/fb0", O_RDWR);
    
    // Check to make sure buffer accessed without error.
    if(bufferFile < 0)
    {
        printf("Framebuffer failed to open.\n");
    }
    else
    {
        printf("Framebuffer opened successfully.\n");
    }
    
    // Grab screen info to determine buffersize
    int var = ioctl(bufferFile, FBIOGET_VSCREENINFO, &virtualResolution);
    int fix = ioctl(bufferFile, FBIOGET_FSCREENINFO, &bitDepth);
    
    // Check to make sure screen info accessed without error.
    if(var != -1 || fix != -1)
    {
        printf("Var and fixed screen info accessed successfully.\n");
        
        // Calculate buffer size
        bufferSize = virtualResolution.yres_virtual * bitDepth.line_length;
    }
    else
    {
        printf("Fixed and var screen info failed to be accessed.\n");
    }
    
    // Map frame buffer into memory
    framebuffer = mmap(0, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, bufferFile, 0);
    
    // Disable echo and buffering of keypresses
    // TCGETS & TCSETS
    // Get current terminal settings
    ioctl(0, TCGETS, terminalSettings);
    
    // unset the ICANON bit
    // unset the ECHO bit
    terminalSettings.c_lflag &= ~ICANON;
    terminalSettings.c_lflag &= ~ECHO;
    
    // Set new terminal settings
    ioctl(0, TCSETS, terminalSettings);
}

// Close graphics
void exit_graphics()
{
    // Close all files
    // Un-memmap buffer
    // Reset terminal settings
    close(bufferFile);
    munmap(bufferFile, bufferSize);
    
    // Get current terminal settings
    ioctl(0, TCGETS, terminalSettings);
    
    // Turn on echo and buffering again
    terminalSettings.c_lflag |= ~ICANON;
    terminalSettings.c_lflag |= ~ECHO;
    
    // Set new terminal settings
    ioctl(0, TCSETS, terminalSettings);
}

// Clear terminal
void clear_screen()
{
    //printf("%u", sizeof('c'));
    //printf("%u", sizeof("\033[2J"));
    
    //printf(framebuffer);
    
    
    write(1,"\033[2J", 5);
}

char get_key()
{
    char input;
    
    // File descriptor to be watched to see if
    // a char is available for read()-ing
    fd_set rfds;
    struct timeval timevalue;
    
    // Clears sets
    FD_ZERO(&rfds);
    
    // Adds stdin file descriptor to set
    FD_SET(0, &rfds);
    
    int canRead = select(1, &rfds, NULL, NULL, &timevalue);
    
    // If canRead is positive, it means a character is ready
    // to be read.
    if(canRead > 0)
    {
        // sizeof(char) in C is 4
        read(0, &input, 4);
    }
    
    
    return input;
}


// Call this to make program sleep between frames of
// graphics being drawn.
void sleep_ms(long ms)
{
    struct timespec tim;
    
    tim.tv_sec = 0;
    tim.tv_nsec = ms * (1000000L);
    
    nanosleep(&tim, NULL);
    
}



void draw_pixel(int x, int y, color_t color)
{
    // SCREEN:
    // 0 ...... 639
    // .
    // .
    // .
    // .
    // 479
    
    // Start at frame buffer
    // To get correct row, add y value multiplied by the virtual x
    // To get correct col, add the input x
    // Dereference and place the color bits at that address.
    
    //int offset = (y * virtualResolution.xres_virtual) + x;
    //int offset = (x * bitDepth.line_length) + (2*y);
    
    int offset = (bitDepth.line_length * y) + (2*x);
    color_t* addr = framebuffer + offset;
    *addr = color;
    
}

void draw_rect(int x1, int y1, int width, int height, color_t c)
{
    
}

void draw_circle(int x, int y, int r, color_t color)
{
    
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
    rgb = rgb | (red<<11);

    // Set middle 6 bits
    rgb = rgb | (green << 5);

    // Set last 5 bits
    rgb = rgb | blue;

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
