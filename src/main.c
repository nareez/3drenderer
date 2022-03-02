#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };

float fov_factor = 640; 

bool is_running = false;
int previous_frame_time = 0;

void setup(void){
    //initialize the render mode

    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

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
    // load_obj_file_data("./assets/cube.obj");
    load_cube_mesh_data();
}

void process_input(void){ 
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type){
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN: 
            switch(event.key.keysym.sym){
                case SDLK_ESCAPE:
                    is_running = false;
                    break;
                case SDLK_1:
                    render_method = RENDER_WIRE_VERTEX;
                    break;
                case SDLK_2:
                    render_method = RENDER_WIRE;
                    break;
                case SDLK_3:
                    render_method = RENDER_FILL_TRIANGLE;
                    break;
                case SDLK_4:
                    render_method = RENDER_FILL_TRIANGLE_WIRE;
                    break;
                case SDLK_c:
                    cull_method = CULL_BACKFACE;
                    break;
                case SDLK_d:
                    cull_method = CULL_NONE;
                    break;
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

    // change the mesh scale/rotation values per animation frame
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;
    mesh.scale.x += 0.003;
    mesh.scale.y += 0.001;

    // create a scale matrix that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec4_t transformed_vertices[3];

        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);

            // translate away from the camera
            transformed_vertex.z += 5;

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }
         if(cull_method == CULL_BACKFACE){
                // check backface culling
            vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*    A   */
            vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  /  \  */
            vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* B---C  */

            // get vector subtraction of B-A and C-A
            vec3_t vector_ab = vec3_sub(vector_b, vector_a);
            vec3_t vector_ac = vec3_sub(vector_c, vector_a);

            // Normalize
            vec3_normalize(&vector_ab);
            vec3_normalize(&vector_ac);

            // Compute the face normal (using cross product to find perpendicular)
            vec3_t normal = vec3_cross(vector_ab, vector_ac);

            // Normalize the face normal vector
            vec3_normalize(&normal);

            // find the vector ray between camera position and a point in triangle
            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            // Calculate how aligned the camera ray is with the face normal (using dot product)
            float dot_normal_camera = vec3_dot(normal, camera_ray);
            // bypass the triangles that are looking away from camera
            if (dot_normal_camera < 0) continue;
        }

        // loop all tree vertices to perform projection
        vec2_t projected_points[3];
        for (int j = 0; j < 3; j++) {
            // project current vertex
            projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));

            // scale and translate to the middle of the screen
            projected_points[j].x += (window_width / 2);
            projected_points[j].y += (window_height / 2);
        }

        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        triangle_t projected_triangle = {
            .points = {
                { projected_points[0].x, projected_points[0].y },
                { projected_points[1].x, projected_points[1].y },
                { projected_points[2].x, projected_points[2].y },
            },
            .color = mesh_face.color,
            .avg_depth = avg_depth
        };
        
        array_push(triangles_to_render, projected_triangle);
    }
    // Sort the triangles to render by their avg_depth
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        for (int j = i; j < num_triangles; j++) {
            if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth) {
                // Swap the triangles positions in the array
                triangle_t temp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = temp;
            }
        }
    }
    
}

void render(void){
    draw_grid(50);

    // loop all triangles and render
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++){
        triangle_t triangle = triangles_to_render[i];
        
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE){
            draw_filled_triangle(triangle.points[0].x, triangle.points[0].y
                        ,triangle.points[1].x, triangle.points[1].y
                        ,triangle.points[2].x, triangle.points[2].y
                        , triangle.color);
        }
        if (render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX){
            draw_triangle(triangle.points[0].x, triangle.points[0].y
                    ,triangle.points[1].x, triangle.points[1].y
                    ,triangle.points[2].x, triangle.points[2].y
                    , 0xffffffff);
        }
        if (render_method == RENDER_WIRE_VERTEX){
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0x00ff0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0x00ff0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0x00ff0000);
        }
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