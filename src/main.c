#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "camera.h"
#include "clipping.h"

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
    set_render_method(RENDER_WIRE);
    set_cull_method(CULL_BACKFACE);

    // Initializa scene light direction
    init_light(vec3_new(0, 0, 1));

    //Initialize the perspective projection matrix
    float fovY = M_PI / 3.0; //180/3 or 60deg
    float aspectY = get_window_height() / (float) get_window_width();
    float aspectX = get_window_width() / (float) get_window_height();
    float fovX = 2 * atan(tan(fovY/2) * aspectX);
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fovY, aspectY, znear, zfar);

    // Initialize frustum planes with a point and a normal
    init_frustum_planes(fovX, fovY, znear, zfar);

   // Loads mesh entities
    load_mesh("./assets/runway.obj", "./assets/runway.png", vec3_new(1, 1, 1), vec3_new(0, -1.5, +23), vec3_new(0, 0, 0));
    load_mesh("./assets/f22.obj", "./assets/f22.png", vec3_new(1, 1, 1), vec3_new(0, -1.3, +5), vec3_new(0, -M_PI/2, 0));
    load_mesh("./assets/efa.obj", "./assets/efa.png", vec3_new(1, 1, 1), vec3_new(-2, -1.3, +9), vec3_new(0, -M_PI/2, 0));
    load_mesh("./assets/f117.obj", "./assets/f117.png", vec3_new(1, 1, 1), vec3_new(+2, -1.3, +9), vec3_new(0, -M_PI/2, 0));
}

void process_input(void){ 
    SDL_Event event;
    while(SDL_PollEvent(&event)){
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
                        set_render_method(RENDER_WIRE_VERTEX);
                        break;
                    case SDLK_2:
                        set_render_method(RENDER_WIRE);
                        break;
                    case SDLK_3:
                        set_render_method(RENDER_FILL_TRIANGLE);
                        break;
                    case SDLK_4:
                        set_render_method(RENDER_FILL_TRIANGLE_WIRE);
                        break;                
                    case SDLK_5:
                        set_render_method(RENDER_TEXTURED);
                        break;
                    case SDLK_6:
                        set_render_method(RENDER_TEXTURED_WIRE);
                        break;
                    case SDLK_c:
                        set_cull_method(CULL_BACKFACE);
                        break;
                    case SDLK_x:
                        set_cull_method(CULL_NONE);
                        break;
                    case SDLK_UP:
                        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                        update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
                        break;
                    case SDLK_DOWN:
                        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                        update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
                        break;
                    case SDLK_w:
                        rotate_camera_pitch(3.0 * delta_time);
                        break;
                    case SDLK_s:
                        rotate_camera_pitch(-3.0 * delta_time);
                        break;
                    case SDLK_RIGHT:
                        rotate_camera_yaw(1.0 * delta_time);
                        break;
                    case SDLK_LEFT:
                        rotate_camera_yaw(-1.0 * delta_time);
                        break;
                }
                break;
            case SDL_WINDOWEVENT_CLOSE:
                printf("\n%i\n", event.type);
                is_running = false;
                break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline stages for all the mesh triangles
///////////////////////////////////////////////////////////////////////////////
// +-------------+
// | Model space |  <-- original mesh vertices
// +-------------+
// |   +-------------+
// `-> | World space |  <-- multiply by world matrix
//     +-------------+
//     |   +--------------+
//     `-> | Camera space |  <-- multiply by view matrix
//         +--------------+
//         |    +------------+
//         `--> |  Clipping  |  <-- clip against the six frustum planes
//              +------------+
//              |    +------------+
//              `--> | Projection |  <-- multiply by projection matrix
//                   +------------+
//                   |    +-------------+
//                   `--> | Image space |  <-- apply perspective divide
//                        +-------------+
//                        |    +--------------+
//                        `--> | Screen space |  <-- ready to render
//                             +--------------+
///////////////////////////////////////////////////////////////////////////////
void process_graphics_pipeline_stages(mesh_t* mesh){
    // change the mesh scale/rotation values per animation frame
    // mesh->rotation.x += 0.5 * delta_time;
    // mesh->rotation.y += 0.5 * delta_time;
    // mesh->rotation.z += 0.5 * delta_time;
    // mesh.scale.x += 0.003 * delta_time;
    // mesh.scale.y += 0.001 * delta_time;
    // mesh.translation.x += 0.01 * delta_time;
    // mesh->translation.z = 4.0;

    // Offset the camera position in the direction where the camera is pointing at
    vec3_t target = get_camera_lookat_target();
    vec3_t up_direction = {0, 1, 0};
    view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

    // create a rotation, scale and translation matrix that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);
    
    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++) {

        face_t mesh_face = mesh->faces[i];
        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

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
        
        // Calculate the triangle face normal
        vec3_t face_normal = get_triangle_normal(transformed_vertices);
        
        // backface culling test to see if the current face should be projected
        if(is_cull_backface()){

            // find the vector ray between camera position and a point in triangle
            vec3_t origin = {0, 0, 0};
            vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
            vec3_t camera_ray = vec3_sub(origin, vector_a);

            // Calculate how aligned the camera ray is with the face normal (using dot product)
            float dot_normal_camera = vec3_dot(face_normal, camera_ray);

            // bypass the triangles that are looking away from camera
            if (dot_normal_camera < 0) continue;
        }

        // Clipping
        polygon_t polygon = polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv
        );

        // Clip the polygon and return a new polygon with potential new vertices
        clip_polygon(&polygon);

        // break the clipped polygon into triangles

        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        for(int t = 0; t < num_triangles_after_clipping; t++){
            triangle_t triangle_after_clipping = triangles_after_clipping[t];
            
            // loop all tree vertices to perform projection
            vec4_t projected_points[3];
            for (int j = 0; j < 3; j++) {
                // project current vertex
                projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

                //scale into the view
                projected_points[j].x *= (get_window_width() / 2.0);
                projected_points[j].y *= (get_window_height() / 2.0);

                // Invert the Y values to account for flipped screen y coordinate
                projected_points[j].y *= -1;

                //translate to the middle of the screen
                projected_points[j].x += (get_window_width() / 2.0);
                projected_points[j].y += (get_window_height() / 2.0);
            }

            // calculate the average depth for each face based on the vertices after the transformation

            //calculate the shade intensity based on how aliged is the face normal and the inverse of the light ray
            float light_intensity_factor = -vec3_dot(face_normal, get_light_direction());
            
            //calculate the triangle color based on the light angle
            uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

            triangle_t triangle_to_render = {
                .points = {
                    { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                    { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                    { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
                },
                .texcoords = {
                    {triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v},
                    {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v},
                    {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v}
                },
                .color = triangle_color,
                .texture = mesh->texture
            };
            
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH){
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render++;
            }
        }
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

    for(int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++){
        mesh_t* mesh = get_mesh(mesh_index);

        // Process the graphics pipeline stages for every mesh
        process_graphics_pipeline_stages(mesh);
    }
}

void render(void){
    
    clear_color_buffer(0xFF000000);
    clear_z_buffer();
    draw_grid(50);

    // loop all triangles and render
    for (int i = 0; i < num_triangles_to_render; i++){
        triangle_t triangle = triangles_to_render[i];
        
        // Draw filled triangle
        if (should_render_filled_triangles()){
            draw_filled_triangle(
                         triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w
                        ,triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w
                        ,triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w
                        ,triangle.color);
        }

        // Draw textured triangle
        if (should_render_textured_triangles()){
            draw_textured_triangle(
                     triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v
                    ,triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v
                    ,triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v
                    ,triangle.texture);
        }

        // Draw wireframe triangle
        if (should_render_wireframe_triangles()){
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0x00ff0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0x00ff0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0x00ff0000);
            draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y,
                           triangle.points[2].x, triangle.points[2].y, 0x00ffffff);
        }
    }

    //clear array of triangles to render
    render_color_buffer(); 
}

// free the memory that was dynamically allocated by the program
void free_resources(void){
    free_meshes();
    destroy_window();

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