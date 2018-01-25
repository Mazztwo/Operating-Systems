// Header file for library
// Alessio Mazzone

// 16 unsigned bits to represent color.
// Index  |  Color  |  Value
//  0-4   |   blue  |   0-31 
//  5-10  |   green |   0-63
//  11-15 |   red   |   0-31
typedef unsigned short color_t;

void init_graphics();

color_t encode_color(int r, int g, int b);