#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE
};

enum render_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
};

bool initialize_window(void);
void draw_grid(int grid_size);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int dx, int dy, int width, int height, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void destroy_window(void);
void clear_z_buffer(void);

int get_window_width(void);
int get_window_height(void);
void set_render_method(int rm);
void set_cull_method(int cm);
bool is_cull_backface(void);

bool should_render_filled_triangles(void);
bool should_render_textured_triangles(void);
bool should_render_wireframe_triangles(void);

float get_zbuffer_at(int x, int y);
void set_zbuffer_at(int x, int y, float value);

#endif