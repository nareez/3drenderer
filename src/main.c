#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"

//Declare an Array of Vector/points
const int N_POINTS = 9 * 9 * 9;
vec3_t cube_points[9 * 9 * 9]; // 9x9x9 cube
vec2_t projected_points[N_POINTS];

vec3_t camera_position = { .x = -1, .y = -1, .z = -5 };
float fov_factor = 640; 

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
        for(float y = -1; y <= 1; y += 0.25){
            for(float z = -1; z <= 1; z += 0.25){
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
            if (event.key.keysym.sym == SDLK_w){
                camera_position.z -= 0.4;
                break;
            }
            if (event.key.keysym.sym == SDLK_s){
                camera_position.z += 0.4;
                break;
            }
            if (event.key.keysym.sym == SDLK_e){
                camera_position.y += 0.3;
                break;
            }
            if (event.key.keysym.sym == SDLK_d){
                camera_position.y -= 0.3;
                break;
            }
            if (event.key.keysym.sym == SDLK_r){
                camera_position.x -= 0.3;
                break;
            }
            if (event.key.keysym.sym == SDLK_f){
                camera_position.x += 0.3;
                break;
            }
    }
}

// Function that receives a 3D vector and returns a projected 2D points
vec2_t project(vec3_t point){
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
    };
    return projected_point;
}

void update(void){
    for (int i = 0; i < N_POINTS; i++){
        vec3_t point = cube_points[i];

        point.z -= camera_position.z;
        point.x -= camera_position.x;
        point.y -= camera_position.y;
        //project point
        vec2_t projected_point = project(point);
        //Save the projected 2D vector in the array of projected points
        projected_points[i] = projected_point;
    }
}

void render(void){

    draw_grid(50);

    //loop all projected points and render
    for (int i = 0; i < N_POINTS; i++){
        vec2_t projected_point = projected_points[i];
        draw_rect(
            projected_point.x + (window_width / 2),
            projected_point.y + (window_height / 2),
            4,
            4,
            0xFFFFFF00);
    }

    render_color_buffer(); 
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

int main(void) {
    is_running = initialize_window();
    
    setup();

    while(is_running){
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}