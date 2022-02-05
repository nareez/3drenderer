#include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;

int window_width = 1000;
int window_height = 800;


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

    int side_lenght = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // Find how increment in both x and y each step
    float x_inc = delta_x / (float)side_lenght;
    float y_inc = delta_y / (float)side_lenght;

    float current_x = x0;
    float current_y = y0;

    for (int i = 0; i <= side_lenght; i++) {
        draw_pixel(round(current_x), round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void render_color_buffer(void){
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (window_width * sizeof(uint32_t))
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color){
    for(int y = 0; y < window_height; y++){
        for(int x = 0; x < window_width; x++){
            color_buffer[(window_width * y) + x] = color;
        }
    }
}

void destroy_window(void){
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}