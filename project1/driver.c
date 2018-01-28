// Driver file for Project1
// Alessio Mazzone

#include "library.h"
#include <stdio.h>

int main()
{
    init_graphics();
    clear_screen();
    
    color_t color;
    color = encode_color(30, 0, 0);
    
    draw_pixel(50, 50, color);
    
    while(get_key() != 'q')
    {
        
    }
    exit_graphics();

    return 0;
}
