#ifndef MODEL_H
#define MODEL_H

#include "r_common.h"
#include "vector.h"
#include "w_common.h"


#include <stdint.h>
#include <limits.h>

#define MODEL_NAME_MAX_LEN 24


enum MODEL_FLAGS
{
	MODEL_INVALID = 1,

};

typedef struct vertex_t
{
	vec3_t position;
	vec3_t normal;
	vec3_t tangent;
	vec2_t tex_coord;
}vertex_t;

typedef struct
{
	vec3_t position;
	int normal;
	int tangent;
	int align0;
	vec2_t tex_coord;
	
}compact_vertex_t;

/* TODO: merge mesh_t and model_t into the same thing. There's 
no point in having both... */
typedef struct mesh_t
{
	struct mesh_t *next;
	vertex_t *vertices;
	vec3_t aabb_max;
	unsigned int gpu_handle;
	unsigned int vert_start;
	unsigned int vert_count;
	unsigned short draw_mode;	
	unsigned short align0;											
	char *name;
}mesh_t;						/* this could go... */


struct model_t
{
	mesh_t *mesh;
	
	vec3_t aabb_max;
	
	//struct model_t *next;
	//struct model_t *prev;
	
	int vert_start;				/* this is replicated here to avoid potential cache misses from dereferencing the mesh pointer... */
	int vert_count;				/* this is replicated here to avoid potential cache misses from dereferencing the mesh pointer... */
	unsigned short draw_mode;	/* this is replicated here to avoid potential cache misses from dereferencing the mesh pointer... */
	unsigned short batch_count;
	batch_t *batches;
	
	int flags;
	
	char *name;
	char *file_name;
};




/*
=======================================================================
=======================================================================
=======================================================================
*/


typedef struct
{
	char tag[16];					/* strlen("[model_section]"); */
	
	uint32_t model_count;
	
	uint32_t reserved0;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;
	uint32_t reserved5;
	uint32_t reserved6;
	uint32_t reserved7;
	
}model_section_header_t;


typedef struct
{
	/* other fields may be added, but this union has to remain as is... */
	/* this union has to be the last field of this struct... */
	//union
//	{
//		struct
//		{
//			char model_name[PATH_MAX];
//			char file_name[PATH_MAX];
//		}separate_names;					/* this is here to allow the allocation of the maximum theoretical size... */
		
//		char names[1];						/* this is here to allow the usage of only the necessary amount... */
//	};

	char model_name[MODEL_NAME_MAX_LEN];
	char file_name[1];

}model_record_t;


/*
=======================================================================
=======================================================================
=======================================================================
*/




#ifdef __cplusplus
extern "C"
{
#endif

int model_Init();

void model_Finish();

mesh_t *model_CreateMesh(char *name, vertex_t *vertices, unsigned int vert_count, unsigned short draw_mode);

int model_DestroyMesh(char *name);

int model_CreateModel(char *file_name, char *name, mesh_t *mesh, batch_t *batches, int batch_count);

int model_DestroyModel(char *name);

int model_DestroyModelIndex(int model_index);

int model_LoadModel(char *file_name, char *model_name);

int model_GetModel(char *model_name);

struct model_t *model_GetModelPointer(char *model_name);

struct model_t *model_GetModelPointerIndex(int model_index);

mesh_t *model_GetModelMesh(char *model_name);

mesh_t *model_GetModelMeshIndex(int model_index);


int model_IncModelMaterialsRefs(int model_index);

int model_DecModelMaterialsRefs(int model_index);

/*model_t *model_GetModel(char *model_name);

model_t *model_GetModelIndex(int model_index);*/

void model_GenerateIcoSphere(float radius, int sub_divs, float **verts, int *face_count);

void model_CalculateTangents(vertex_t *vertices, int vertice_count);

void model_CalculateTangentsIndexes(vertex_t *vertices, int *indexes, int index_count);

void model_GenerateIndexes(struct model_t *model);

//void mesh_LoadWorldModel(char *file_name);

compact_vertex_t *model_ConvertVertices(vertex_t *vertices, int vert_count);


/*
=======================================================================
=======================================================================
=======================================================================
*/


void model_SerializeModels(void **buffer, int *buffer_size);

void model_DeserializeModels(void **buffer);

/*
=======================================================================
=======================================================================
=======================================================================
*/



#ifdef __cplusplus
}
#endif


#endif
