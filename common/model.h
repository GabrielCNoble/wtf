#ifndef MODEL_H
#define MODEL_H

#include "vector.h"
#include "w_common.h"
#include "bsp_common.h"


enum MODEL_FLAGS
{
	MODEL_INVALID = 1,

};

typedef struct
{
	vec3_t position;
	vec3_t normal;
	vec3_t tangent;
	vec2_t tex_coord;
}vertex_t;

typedef struct mesh_t
{
	struct mesh_t *next;
	vertex_t *vertices;
	unsigned int gpu_handle;
	unsigned int vert_start;
	unsigned int vert_count;
	unsigned short draw_mode;	
	unsigned short align0;											
	char *name;
}mesh_t;	


typedef struct
{
	mesh_t *mesh;
	
	int vert_start;				/* this is replicated here to avoid potential cache misses from dereferencing the mesh pointer... */
	int vert_count;				/* this is replicated here to avoid potential cache misses from dereferencing the mesh pointer... */
	
	unsigned short triangle_group_count;
	unsigned short draw_mode;	/* this is replicated here to avoid potential cache misses from dereferencing the mesh pointer... */
	triangle_group_t *triangle_groups;
	
	int flags;
	
	char *name;
	char *file_name;
}model_t;


int model_Init();

void model_Finish();

mesh_t *model_CreateMesh(char *name, vertex_t *vertices, unsigned int vert_count, unsigned short draw_mode);

int model_DestroyMesh(char *name);

int model_CreateModel(char *name, mesh_t *mesh, triangle_group_t *triangle_groups, int triangle_group_count);

int model_DestroyModel(char *name);

int model_DestroyModelIndex(int model_index);

int model_LoadModel(char *file_name, char *model_name);

int model_GetModel(char *model_name);

model_t *model_GetModelPointer(char *model_name);

model_t *model_GetModelPointerIndex(int model_index);


int model_IncModelMaterialsRefs(int model_index);

int model_DecModelMaterialsRefs(int model_index);

/*model_t *model_GetModel(char *model_name);

model_t *model_GetModelIndex(int model_index);*/

void model_GenerateIcoSphere(float radius, int sub_divs, float **verts, int *face_count);

void model_CalculateTangents(vertex_t *vertices, int vertice_count);

void model_CalculateTangentsIndexes(vertex_t *vertices, int *indexes, int index_count);

//void mesh_LoadWorldModel(char *file_name);




#endif
