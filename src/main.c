#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };
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

    // load the cube values in the mesh data structure
    load_obj_file_data("./assets/f22.obj");
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
            }
            break;
        case SDL_WINDOWEVENT_CLOSE:
            printf("\n%i\n", event.type);
            is_running = false;
            break;
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

    // initialize the array to render
    triangles_to_render = NULL;

    cube_rotation.x += 0.01;
    cube_rotation.y += 0.01; 
    cube_rotation.z += 0.01;

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];



        vec3_t transformed_vertices[3];

        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, cube_rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, cube_rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, cube_rotation.z);

            // translate away from the camera
            transformed_vertex.z += 5;

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // check backface culling
        vec3_t vector_a = transformed_vertices[0]; /*    A   */
        vec3_t vector_b = transformed_vertices[1]; /*  /  \  */
        vec3_t vector_c = transformed_vertices[2]; /* B---C  */

        // get vector subtraction of B-A and C-A
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);

        // Compute the face normal (using cross product to find perpendicular)
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // find the vector ray between camera position and a point in triangle
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // Calculate how aligned the camera ray is with the face normal (using dot product)
        float dot_normal_camera = vec3_dot(normal, camera_ray);
        // bypass the triangles that are looking away from camera
        if (dot_normal_camera < 0) continue;

        // loop all tree vertices to perform projection
        triangle_t projected_triangle;
        for (int j = 0; j < 3; j++) {
            // project current vertex
            vec2_t projected_point = project(transformed_vertices[j]);

            // scale and translate to the middle of the screen
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }
        array_push(triangles_to_render, projected_triangle);
        
    }
}

void render(void){
    draw_grid(50);

    //loop all triangles and render
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++){
        triangle_t triangle = triangles_to_render[i];
        
        draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);
        draw_triangle(triangle.points[0].x, triangle.points[0].y
                     ,triangle.points[1].x, triangle.points[1].y
                     ,triangle.points[2].x, triangle.points[2].y
                     , 0xff00ff00);
    }

    //clear array of triangles to render
    array_free(triangles_to_render);
    render_color_buffer(); 
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

// free the memory that was dynamically allocated by the program
void free_resources(void){
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
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
    free_resources();

    return 0;
}