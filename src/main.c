#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "camera.h"

// Globals
#define MAX_TRIANGLES_PER_MESH 10000

triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;


bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;

//Global transformations
mat4_t proj_matrix;
mat4_t view_matrix;
mat4_t world_matrix;

void setup(void){
    //initialize the render mode

    render_method = RENDER_TEXTURED;
    cull_method = CULL_BACKFACE;

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

    //Initialize the perspective projection matrix
    float fov = M_PI / 3.0; //180/3 or 60deg
    float aspect = window_height / (float) window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    // load the cube values in the mesh data structure
    load_obj_file_data("./assets/f22.obj");
    // load_cube_mesh_data();

    // load the texture information from an external PNG file
    load_png_texture_data("./assets/f22.png");
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
                case SDLK_5:
                    render_method = RENDER_TEXTURED;
                    break;
                case SDLK_6:
                    render_method = RENDER_TEXTURED_WIRE;
                    break;
                case SDLK_c:
                    cull_method = CULL_BACKFACE;
                    break;
                case SDLK_x:
                    cull_method = CULL_NONE;
                    break;
                case SDLK_UP:
                    camera.position.y += 3.0 * delta_time;
                    break;
                case SDLK_DOWN:
                    camera.position.y -= 3.0 * delta_time;
                    break;
                case SDLK_w:
                    camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
                    camera.position = vec3_add(camera.position, camera.forward_velocity);
                    break;
                case SDLK_s:
                    camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
                    camera.position = vec3_sub(camera.position, camera.forward_velocity);
                    break;
                case SDLK_d:
                    camera.yaw += 1.0 * delta_time;
                    break;
                case SDLK_a:
                    camera.yaw -= 1.0 * delta_time;
                    break;
            }
            break;
        case SDL_WINDOWEVENT_CLOSE:
            printf("\n%i\n", event.type);
            is_running = false;
            break;
    }
}

void update(void){
    // calculate time to next frame
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    
    // delay time until next frame
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME){
        SDL_Delay(time_to_wait);
    }

    // Get a delta time factor to sync movement
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

    previous_frame_time = SDL_GetTicks();

    // initialize the array to render
    num_triangles_to_render = 0;

    // change the mesh scale/rotation values per animation frame
    // mesh.rotation.x += 0.01 * delta_time;
    // mesh.rotation.y += 0.01 * delta_time;
    // mesh.rotation.z += 0.01 * delta_time;
    // mesh.scale.x += 0.003 * delta_time;
    // mesh.scale.y += 0.001 * delta_time;
    // mesh.translation.x += 0.01 * delta_time;
    mesh.translation.z = 4.0;

    // Create the view matrix
    vec3_t up_direction = {0, 1, 0};

    // Initialize target looking at the positive z-axis
    vec3_t target = {0, 0, 1};
    mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
    camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

    // Offset the camera position in the direction where the camera is pointing at

    target = vec3_add(camera.position, camera.direction);
    
    view_matrix = mat4_look_at(camera.position, target, up_direction);

    // create a rotation, scale and translation matrix that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
    
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a];
        face_vertices[1] = mesh.vertices[mesh_face.b];
        face_vertices[2] = mesh.vertices[mesh_face.c];

        vec4_t transformed_vertices[3];

        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // create a World Matrix combining scale, rotation and translation matrices
            world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);
            
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // Multiply the view matrix by the vector to transform the scene do camera view
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }
         
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
            vec3_t origin = {0, 0, 0};
            vec3_t camera_ray = vec3_sub(origin, vector_a);

            // Calculate how aligned the camera ray is with the face normal (using dot product)
            float dot_normal_camera = vec3_dot(normal, camera_ray);
        if(cull_method == CULL_BACKFACE){
            // bypass the triangles that are looking away from camera
            if (dot_normal_camera < 0) continue;
        }

        // loop all tree vertices to perform projection
        vec4_t projected_points[3];
        for (int j = 0; j < 3; j++) {
            // project current vertex
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            //scale into the view
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            // Invert the Y values to account for flipped screen y coordinate
            projected_points[j].y *= -1;

            //translate to the middle of the screen
            projected_points[j].x += (window_width / 2.0);
            projected_points[j].y += (window_height / 2.0);
        }

        // calculate the average depth for each face based on the vertices after the transformation

        //calculate the shade intensity based on how aliged is the face normal and the inverse of the light ray
        float light_intensity_factor = -vec3_dot(normal, light.direction);
        
        //calculate the triangle color based on the light angle
        uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

        triangle_t projected_triangle = {
            .points = {
                { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
            },
            .texcoords = {
                {mesh_face.a_uv.u, mesh_face.a_uv.v},
                {mesh_face.b_uv.u, mesh_face.b_uv.v},
                {mesh_face.c_uv.u, mesh_face.c_uv.v}
            },
            .color = triangle_color
        };
        
        if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH){
            triangles_to_render[num_triangles_to_render] = projected_triangle;
            num_triangles_to_render++;
        }
        
    }
    
}

void render(void){
    draw_grid(50);

    // loop all triangles and render
    for (int i = 0; i < num_triangles_to_render; i++){
        triangle_t triangle = triangles_to_render[i];
        
        // Draw filled triangle
        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE){
            draw_filled_triangle(
                         triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w
                        ,triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w
                        ,triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w
                        ,triangle.color);
        }

        // Draw textured triangle
        if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE){
            draw_textured_triangle(
                     triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v
                    ,triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v
                    ,triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v
                    ,mesh_texture);
        }

        // Draw wireframe triangle
        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRE){
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0x00ff0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0x00ff0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0x00ff0000);
            draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y,
                           triangle.points[2].x, triangle.points[2].y, 0x00ffffff);
        }
    }

    //clear array of triangles to render
    render_color_buffer(); 
    clear_color_buffer(0xFF000000);
    clear_z_buffer();

    SDL_RenderPresent(renderer);
}

// free the memory that was dynamically allocated by the program
void free_resources(void){
    free(color_buffer);
    free(z_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
    upng_free(png_texture);
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