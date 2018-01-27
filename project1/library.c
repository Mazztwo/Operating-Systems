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
    
    // Grab screen info to determine buffersize
    ioctl(bufferFile, "FBIOGET_VSCREENINFO", virtualResolution);
    ioctl(bufferFile, "FBIOGET_FSCREENINFO", bitDepth);
    
    // Calculate buffer size
    bufferSize = virtualResolution.yres_virtual * bitDepth.line_length;
    
    // Map frame buffer into memory
    framebuffer = mmap(NULL, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, bufferFile, 0);
    
    // Disable echo and buffering of keypresses
    // TCGETS & TCSETS
    // Get current terminal settings
    ioctl(0, TCGETS, terminalSettings);
    
    // unset the ICANON bit
    // unset the ECHO bit
    terminalSettings.c_cflag &= ~ICANON;
    terminalSettings.c_cflag &= ~ECHO;
    
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
    
    terminalSettings.c_cflag |= ~ICANON;
    terminalSettings.c_cflag |= ~ECHO;
}

// Clear terminal
void clear_screen()
{
    printf("%u", sizeof('c'));
    //write(1,"\033[2J", 5);
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
// graphics beinng drawn.
void sleep_ms(long ms)
{
    struct timespec tim;
    
    tim.tv_sec = 0;
    tim.tv_nsec = ms * (1000000L);
    
    nanosleep(&tim, NULL);
    
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
