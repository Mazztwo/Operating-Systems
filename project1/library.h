// Header file for library
// Alessio Mazzone

// 16 unsigned bits to represent color.
// Index  |  Color  |  Value
//  0-4   |   blue  |   0-31 
//  5-10  |   green |   0-63
//  11-15 |   red   |   0-31
typedef unsigned short color_t;


// DEBUG FUNCTIONS:
void print_binary(color_t number);
//////////////////


void init_graphics();
void exit_graphics();

color_t encode_color(int r, int g, int b);


