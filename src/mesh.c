#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "array.h"
#include "mesh.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = {0, 0, 0},
    .scale = {1.0, 1.0, 1.0},
    .translation = {0, 0, 0}
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 }, // 1
    { .x = -1, .y =  1, .z = -1 }, // 2
    { .x =  1, .y =  1, .z = -1 }, // 3
    { .x =  1, .y = -1, .z = -1 }, // 4
    { .x =  1, .y =  1, .z =  1 }, // 5
    { .x =  1, .y = -1, .z =  1 }, // 6
    { .x = -1, .y =  1, .z =  1 }, // 7
    { .x = -1, .y = -1, .z =  1 }  // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    { .a = 1, .b = 3, .c = 4, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    // right
    { .a = 4, .b = 3, .c = 5, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    { .a = 4, .b = 5, .c = 6, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    // back
    { .a = 6, .b = 5, .c = 7, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    { .a = 6, .b = 7, .c = 8, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    // left
    { .a = 8, .b = 7, .c = 2, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    { .a = 8, .b = 2, .c = 1, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    // top
    { .a = 2, .b = 7, .c = 5, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    { .a = 2, .b = 5, .c = 3, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    // bottom
    { .a = 6, .b = 8, .c = 1, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    { .a = 6, .b = 1, .c = 4, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF }
};

void load_cube_mesh_data(void){
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        vec3_t cube_vertex = cube_vertices[i];
        array_push(mesh.vertices, cube_vertex);
    }
    for (int i = 0; i < N_CUBE_FACES; i++){
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}

void push_vertice_from_obj_line(char* line){
    vec3_t vertex;
    sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
    array_push(mesh.vertices, vertex);
}

tex2_t* texcoords;
void push_texture_coords_from_obj_line(char* line){
    tex2_t texcoord;
    sscanf(line, "vt %f %f", &texcoord.u, &texcoord.v);
    array_push(texcoords, texcoord);
}

void push_face_from_obj_line(char* line){
    int vertex_indices[3];
    int texture_indices[3];
    int normal_indices[3];

    sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
           &vertex_indices[0], &texture_indices[0], &normal_indices[0],
           &vertex_indices[1], &texture_indices[1], &normal_indices[1],
           &vertex_indices[2], &texture_indices[2], &normal_indices[2]);

    face_t face = { 
        .a = vertex_indices[0] - 1,
        .b = vertex_indices[1] - 1,
        .c = vertex_indices[2] - 1,
        .a_uv = texcoords[texture_indices[0] - 1],
        .b_uv = texcoords[texture_indices[1] - 1],
        .c_uv = texcoords[texture_indices[2] - 1],
        .color = 0xffffffff
    };
    array_push(mesh.faces, face);
}

void load_obj_file_data(char* fileName){
    FILE* file;
    char buff[1024];
    file = fopen(fileName,"r");
    if (file == NULL) return;

    while (fgets(buff, sizeof(buff), file) != NULL){
        if (strncmp(buff, "v ", 2) == 0){
            push_vertice_from_obj_line(buff);
        } else if (strncmp(buff, "f ", 2) == 0){
            push_face_from_obj_line(buff);
        } else if (strncmp(buff, "vt ", 3) == 0){
            push_texture_coords_from_obj_line(buff);
        }
    }

    array_free(texcoords);

    fclose(file);
}
