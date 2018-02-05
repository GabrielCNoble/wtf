#ifndef BSP_FILE_H
#define BSP_FILE_H


#define BSP_FILE_VERSION 0
#define BSP_FILE_MAX_NAME_LEN 64
#define MAX_NAME_LEN 64


typedef struct
{
	int version;
	int world_vertice_count;
	int world_triangle_group_count;
	int world_nodes_count;
	int world_leaves_count;
	int collision_nodes_count;
	int light_count;
	int material_count;
}bsp_header_t;

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t color;
	float radius;
	float energy;
	char name[MAX_NAME_LEN];
	short type;
	short bm_flags;
}light_lump_t;

typedef struct
{
	vec4_t base;
	char name[MAX_NAME_LEN];
	char shader_name[MAX_NAME_LEN];
	int bm_textures;
}material_lump_t;

typedef struct
{
	int vertice_count;
	char material_name[MAX_NAME_LEN];
}triangle_group_lump_t;

#endif




