#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "model.h"
#include "mpk_file.h"
#include "material.h"


typedef struct
{
	int vertex_index;
	int tex_coord_index;
	int normal_index;
	int used;
	char *material_name;
}face_index_t;


typedef struct
{
	material_record_t record;
	char diffuse_texture[MPK_MAX_NAME_LEN];
	char normal_texture[MPK_MAX_NAME_LEN]; 
	char material_full_name[MPK_MAX_NAME_LEN];
}extended_material_record_t;


typedef struct
{
	texture_record_t record;
	char texture_name[MPK_MAX_NAME_LEN];
}extended_texture_record_t;


void calculate_tangents(vertex_t *vertices, int vertice_count);

int convert(char *file_name);

void load(char *file_name);
