// Driver file for Project1
// Alessio Mazzone

#include "library.h"
#include <stdio.h>

int main()
{
    char key = 'm';
    int x, y, width, height, radius;
    
    init_graphics();
    
    clear_screen();
    
    
    color_t color;
    color = encode_color(0, 60, 0);
    
   /*
    draw_pixel(50, 50, color);
    draw_pixel(50, 60, color);
    draw_pixel(50, 70, color);
    draw_pixel(50, 80, color);
    draw_pixel(50, 90, color);
    */


    x = 50, y = 50, width = 100, height = 100;
    draw_rect(x, y, width, height, color);
    
    
    x = 200, y = 200, radius = 30;
    draw_circle(x, y, radius, color);
    
    
    while(key != 'q')
    {
        
    	key = get_key();
    }
    
    
    exit_graphics();

    return 0;
}
