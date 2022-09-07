#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"
#include "upng.h"

// defines a struct for dynamic size meshes, with array of vectors and meshes
typedef struct {
    vec3_t* vertices;   // dynamic array of vertices
    face_t* faces;      // dynamic array of faces
    vec3_t rotation;    // rotation with x, y and z
    vec3_t scale;       // scale with x,y,z
    vec3_t translation; // translation with x,y,z
    upng_t* texture;    // mesh png texture  
} mesh_t;

void load_obj_file_data(mesh_t* mesh, char* fileName);
void load_mesh_png_data(mesh_t* mesh, char* filename);
void load_mesh(char* obj_filename, char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);
int get_num_meshes(void);
mesh_t* get_mesh(int index);
void free_meshes(void);

#endif