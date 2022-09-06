#include "display.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static uint32_t* color_buffer = NULL;
static SDL_Texture* color_buffer_texture = NULL;
static float* z_buffer = NULL;

static int window_width = 1000;
static int window_height = 800;

static int render_method = RENDER_WIRE;
static int cull_method = CULL_BACKFACE;

int get_window_width(void){
    return window_width;
}
int get_window_height(void){
    return window_height;
}

bool initialize_window(void){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error Initializing SDL.\n");
        return false;
    }

    //uncomment to fullscreen
    //Get max resolution
    // SDL_DisplayMode display_mode;
    // SDL_GetCurrentDisplayMode(0, &display_mode);
    // window_width = display_mode.w;
    // window_height = display_mode.h;

    //Create SQL Window
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        0 // SDL_WINDOW_BORDERLESS //uncomment to remove borders 
    );
    if(!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    //Create SDL Renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    //uncomment to full screen
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // Allocate color buffer
    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
    z_buffer = (float*) malloc(sizeof(float) * window_width * window_height);
    clear_z_buffer();

    // Create a SDL Texture that is used to draw the buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    return true;
}

void draw_grid(int grid_size){
    uint32_t color_white = 0xFFFFFFFF;
    for(int y = 0; y < window_height; y++){
        for(int x = 0; x < window_width; x++){
            if (y % grid_size == 0 && x % grid_size == 0){
                draw_pixel(x, y, color_white);
            }          
        }
    }
}

void draw_pixel(int x, int y, uint32_t color){
    if (x >= 0 && y >=0 && x < window_width && y < window_height){
        color_buffer[(window_width * y) + x] = color;
    }
}

void draw_rect(int dx, int dy, int width, int height, uint32_t color){
    for(int y = dy; y < (dy + height); y++){
        for(int x = dx; x < (dx + width); x++){
            draw_pixel(x, y, color);
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color){
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    int longest_side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // Find how increment in both x and y each step
    float x_inc = delta_x / (float)longest_side_length;
    float y_inc = delta_y / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;

    for (int i = 0; i <= longest_side_length; i++) {
        draw_pixel(round(current_x), round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color){
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

void render_color_buffer(void){
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (window_width * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void clear_color_buffer(uint32_t color){
    for(int i = 0; i < window_width * window_height; i++){
        color_buffer[i] = color;
    }
}

void destroy_window(void){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void clear_z_buffer(void){
    for(int i = 0; i < window_width * window_height; i++){
        z_buffer[i] = 1.0;
    }
}

float get_zbuffer_at(int x, int y){
    if (x >= 0 && y >=0 && x < window_width && y < window_height){
        return z_buffer[(window_width * y) + x];
    }
    return 1.0;
}

void set_zbuffer_at(int x, int y, float value){
    if (x >= 0 && y >=0 && x < window_width && y < window_height){
        z_buffer[(window_width * y) + x] = value;
    }
}

void set_render_method(int rm){
    render_method = rm;
}

void set_cull_method(int cm){
    cull_method = cm;
}

bool is_cull_backface(){
    return cull_method == CULL_BACKFACE;
}

bool should_render_filled_triangles(void){
    return render_method == RENDER_FILL_TRIANGLE 
        || render_method == RENDER_FILL_TRIANGLE_WIRE;
}

bool should_render_textured_triangles(void){
    return render_method == RENDER_TEXTURED 
        || render_method == RENDER_TEXTURED_WIRE;
}

bool should_render_wireframe_triangles(void){
    return render_method == RENDER_WIRE 
        || render_method == RENDER_WIRE_VERTEX 
        || render_method == RENDER_FILL_TRIANGLE_WIRE 
        || render_method == RENDER_TEXTURED_WIRE;
}

