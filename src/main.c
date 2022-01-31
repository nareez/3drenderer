#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"

//Declare an Array of Vector/points
#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[9 * 9 * 9]; // 9x9x9 cube
vec2_t projected_points[N_POINTS];

vec3_t camera_position = { .x = 0, .y = 0, .z = -5 };
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };

float fov_factor = 640; 

bool is_running = false;
int previous_frame_time = 0;

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
        case SDL_WINDOWEVENT_CLOSE:
            is_running = false;
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
    // calculate time to next frame
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    
    // delay time until next frame
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME){
        SDL_Delay(time_to_wait);
    }

    previous_frame_time = SDL_GetTicks();

    cube_rotation.x += 0.01;
    cube_rotation.y += 0.01; 
    cube_rotation.z += 0.01;

    for (int i = 0; i < N_POINTS; i++){
        vec3_t point = cube_points[i];

        vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
        transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
        transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

        // translate the points away from camera
        transformed_point.z -= camera_position.z;

        //project point
        vec2_t projected_point = project(transformed_point);
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
    // initialize renderer
    is_running = initialize_window();
    setup();

    // declare FPS Count variables
    Uint64 startTickCount;
    Uint64 endTickCount;
    long int frame_count = 0;

    while(is_running){
        // get start tick count to calculate FPS
        startTickCount = SDL_GetPerformanceCounter();

        // Rendering main loop
        process_input();
        update();
        render();

        //get end tick and calculate FPS
        endTickCount = SDL_GetPerformanceCounter();
        frame_count++;
        float elapsed = (endTickCount - startTickCount) / (float)SDL_GetPerformanceFrequency();
        if(frame_count % FPS == 0){
            printf("FPS: %f \n", (1.0f / elapsed));
        }
    }
    
    destroy_window();

    return 0;
}