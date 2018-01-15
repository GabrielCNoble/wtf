#ifndef BSP_FILE_H
#define BSP_FILE_H


#define BSP_FILE_VERSION 0


typedef struct
{
	int version;
	int world_vertice_count;
	int world_triangle_group_count;
	int world_nodes_count;
	int world_leaves_count;
	int collision_nodes_count;
	int light_count;
}bsp_header_t;

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t color;
	float radius;
	float energy;
	short type;
	short bm_flags;
}light_lump_t;

typedef struct
{
	int vertice_count;
	char material_name[32];
}triangle_group_lump_t;

#endif