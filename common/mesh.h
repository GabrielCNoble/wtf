#ifndef MESH_H
#define MESH_H

#include "vector.h"

typedef struct
{
	vec3_t position;
	vec3_t normal;
	vec3_t tangent;
	vec2_t tex_coord;
}vertex_t;

typedef struct
{
	vertex_t *vertices;
	int start;
	unsigned short vert_count;
	short draw_mode;												
	char *name;
}mesh_t;	



int mesh_Init();

void mesh_Finish();

void mesh_LoadModel(char *file_name, char *model_name);

mesh_t *mesh_GetModel(char *model_name);

void mesh_GenerateIcoSphere(float radius, int sub_divs, float **verts, int *face_count);

//void mesh_LoadWorldModel(char *file_name);




#endif
