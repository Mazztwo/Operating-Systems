// Driver file for Project1
// Alessio Mazzone

#include "library.h"
#include <stdio.h>

int main()
{
    char key = 'p';
    
    init_graphics();
    clear_screen();
    
    
    color_t color;
    color = encode_color(30, 0, 0);
    
    
    //draw_pixel(50, 50, color);
    //draw_pixel(50, 60, color);
    //draw_pixel(50, 70, color);
    //draw_pixel(50, 80, color);
    //draw_pixel(50, 90, color);
    
    
    
    while(key != 'q')
    {
        key = get_key();
    }
     
    
    exit_graphics();

    return 0;
}
