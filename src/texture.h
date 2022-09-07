#ifndef TEXTURE_H
#define TEXTURE_H
#include "vector.h"

typedef struct {
    float u;
    float v;
} tex2_t;

tex2_t tex2_clone(tex2_t* t);
vec3_t get_triangle_normal(vec4_t vertices[3]);

#endif