#ifndef MPK_FILE_H
#define MPK_FILE_H

#include "vector_types.h"
#include <limits.h>


//#include "bsp_file.h"
//#include "model.h"
#include <limits.h>

#define MPK_CONSTANT0 0x006b706d
#define MPK_CONSTANT1 0x6d706b00

#define MPK_VERSION 0

static char mpk_header_tag[] = "MPK";

struct mpk_header_t
{
	char tag[(sizeof(mpk_header_tag) + 3) & (~3)];

	int vertice_count;
	int indice_count;
	int batch_count;
	int lod_count;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;
};

struct mpk_batch_t
{
	char material_name[(PATH_MAX + 3) & (~3)];
	int indice_start;
	int indice_count;
};


typedef struct mpk_vertex_t
{
	vec3_t position;
	vec3_t normal;
	vec3_t tangent;
	vec2_t tex_coord;
}mpk_vertex_t;

struct mpk_triangle_t
{
	//int verts[3];
	int *verts;
	char *material_name;
};

struct input_params_t
{
    mpk_vertex_t *vertices;
    int vertices_count;

    int *indices;
    int indices_count;

    struct mpk_batch_t *batches;
    int batches_count;

    //struct mpk_triangle_t *in_triangles;
    //int in_triangles_count;
};

struct mpk_lod_t
{
    int batch_start;
    int indice_start;
	int indice_count;
};

struct lod_indices_t
{
    struct mpk_batch_t *batches;
    int *indices;
};

struct mpk_material_t
{
	vec4_t base;
	float roughness;
	float metalness;
	int flags;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;

	char material_name[PATH_MAX];
	char diffuse_texture_name[PATH_MAX];
	char normal_texture_name[PATH_MAX];
	char height_texture_name[PATH_MAX];
	char metalness_texture_name[PATH_MAX];
	char roughness_texture_name[PATH_MAX];
};


struct output_params_t
{
	mpk_vertex_t *vertices;
	int vertices_count;

	struct mpk_batch_t *batches;
	int batches_count;

	int *indices;
	int indices_count;

	int lods_count;
	struct mpk_lod_t *lods;
};


void read_mpk(char *file_name);

void write_mpk(char *file_name);


#endif




