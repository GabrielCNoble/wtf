#ifndef MPK_FILE_H
#define MPK_FILE_H


#include "bsp_file.h"
#include "model.h"
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

struct mpk_lod_t
{
	int indice_count;
};



struct mpk_triangle_t
{
	int verts[3];
	char *material_name;
};

struct input_params_t
{
    vertex_t *in_vertices;
    int in_vertices_count;

    int *in_indices;
    int in_indices_count;

    struct mpk_batch_t *in_batches;
    int in_batches_count;

    struct mpk_triangle_t *in_triangles;
    int in_triangles_count;
};

struct output_params_t
{
	vertex_t *out_vertices;
	int out_vertices_count;
	int out_batches_count;

	int *out_indices;
	int out_indices_count;

    struct mpk_lod_t *out_lods;
    int out_lods_count;
    int **out_lods_indices;
    struct mpk_batch_t **out_lods_batches;
};


void read_mpk(char *file_name);

void write_mpk(char *file_name);


#endif




