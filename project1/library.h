// Header file for library
// Alessio Mazzone

// 16 unsigned bits to represent color.
// Index  |  Color  |  Value
//  0-4   |   blue  |   0-31 
//  5-10  |   green |   0-63
//  11-15 |   red   |   0-31
typedef unsigned short color_t;


// DEBUG FUNCTIONS:
//void print_binary(color_t number);
//////////////////


void init_graphics();
void exit_graphics();
void clear_screen();
char get_key();
void sleep_ms(long ms);
void draw_pixel(int x, int y, color_t color);
void draw_rect(int x1, int y1, int width, int height, color_t c);
void draw_circle(int x, int y, int r, color_t color);
color_t encode_color(int r, int g, int b);


