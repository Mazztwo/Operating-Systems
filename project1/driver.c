// Driver file for Project1
// Alessio Mazzone

#include "library.h"
#include <stdio.h>

int main()
{
    char key = 'm';
    int x, y, width, height, radius;
    int MAX_X = 639;
    int MAX_Y = 479;
    
    
    init_graphics();
    clear_screen();
    
    color_t color, reset_black;
    color = encode_color(0, 60, 0);
    reset_black = encode_color(0, 0, 0);
    
    
    x = 50, y = 50, width = 100, height = 100;
    draw_rect(x, y, width, height, color);
    
    x = 200, y = 200, radius = 30;
    draw_circle(x, y, radius, color);
    
    while(key != 'q')
    {
        if(key == 'w')
        {
            if( !(y-radius <= 0) )
            {
                draw_circle(x, y, radius, reset_black);
                y--;
                sleep_ms(5000);
                draw_circle(x, y, radius, color);
            }
            
        }
        
        
    	key = get_key();
    }
    
    
    exit_graphics();

    return 0;
}
