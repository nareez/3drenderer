#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"

//Declare an Array of Vector/points
const int N_POINTS = 9 * 9 * 9;
vec3_t cube_points[9 * 9 * 9]; // 9x9x9 cube

bool is_running = false;

void setup(void){
    // Allocate color buffer
    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    // Create a SDL Texture that is used to draw the buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    //load my array of vectors
    // from -1 to 1 in 9x9x9 cube
    int point_count = 0;
    for (float x = -1; x <= 1; x += 0.25){
        for(float y = -1; y <= 1; x += 0.25){
            for(float z = -1; z <= 1; x += 0.25){
                vec3_t new_point = {.x = x, .y = y, .z = z};
                cube_points[point_count++] = new_point;
            }
        }
    }
}

void process_input(void){ 
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type){
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN: 
            if (event.key.keysym.sym == SDLK_ESCAPE){
                is_running = false;
                break;
            }
    }
}

void update(void){

}

void render(void){
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_grid(50);

    draw_pixel(20, 20, 0xFFFFFF00);

    render_color_buffer(); 
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

int main(void) {
    is_running = initialize_window();
    
    setup();

    vec3_t myvec3 = {2.0, 2.1, 3.6};
    printf("teste: %f", myvec3.x);

    while(is_running){
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}