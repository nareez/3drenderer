#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "array.h"
#include "mesh.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = {0, 0, 0}
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
    { .a = 1, .b = 2, .c = 3 },
    { .a = 1, .b = 3, .c = 4 },
    // right
    { .a = 4, .b = 3, .c = 5 },
    { .a = 4, .b = 5, .c = 6 },
    // back
    { .a = 6, .b = 5, .c = 7 },
    { .a = 6, .b = 7, .c = 8 },
    // left
    { .a = 8, .b = 7, .c = 2 },
    { .a = 8, .b = 2, .c = 1 },
    // top
    { .a = 2, .b = 7, .c = 5 },
    { .a = 2, .b = 5, .c = 3 },
    // bottom
    { .a = 6, .b = 8, .c = 1 },
    { .a = 6, .b = 1, .c = 4 }
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

void push_vertice(char* line){
    char* type;
    char* v1;
    char* v2;
    char* v3;
    vec3_t cube_vertex;

    type = strtok(line, " ");
    v1 = strtok(NULL, " ");
    v2 = strtok(NULL, " ");
    v3 = strtok(NULL, " ");

    cube_vertex.x = atof(v1);
    cube_vertex.y = atof(v2);
    cube_vertex.z = atof(v3);

    array_push(mesh.vertices, cube_vertex);
}


void push_face(char* line){
    char* type;
    char* v1;
    char* v2;
    char* v3;
    face_t cube_face;

    type = strtok(line, " ");  
    v1 = strtok(NULL, " ");
    v2 = strtok(NULL, " ");
    v3 = strtok(NULL, " ");

    v1 = strtok(v1, "/");
    v2 = strtok(v2, "/");
    v3 = strtok(v3, "/");

    cube_face.a = atof(v1);
    cube_face.b = atof(v2);
    cube_face.c = atof(v3);

    array_push(mesh.faces, cube_face);
}

void load_obj_file_data(char* fileName){
    FILE* file;
    char buff[255];
    file = fopen(fileName,"r");
    if (file == NULL) return;

    while (fgets(buff, sizeof(buff), file) != NULL){
        switch (buff[0]){
        case 'v':
            if(buff[1] == ' ') push_vertice(buff);
            break;
        case 'f':
            push_face(buff);
            break;
        }   
    }

    fclose(file);
}
