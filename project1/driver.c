// Driver file for Project1
// Alessio Mazzone

#include "library.h"
#include <stdio.h>

int main()
{
    init_graphics();

    color_t test = encode_color(30,20,12);

   
    print_binary(test);
    printf("\n");

    return 0;
}
