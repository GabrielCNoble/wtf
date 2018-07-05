#ifndef BSP_FILE_H
#define BSP_FILE_H


#define BSP_FILE_VERSION 0
#define BSP_FILE_MAX_NAME_LEN 64
#define MAX_NAME_LEN 64
#define BSP_MAX_NAME_LEN 512

#include <stdint.h>
#include "material.h"
#include "matrix.h"
#include "texture.h"

/*
===================================================================
===================================================================
===================================================================
*/

typedef struct
{
	uint32_t version;
	uint32_t world_vertice_count;
	uint32_t world_batch_count;
	uint32_t world_nodes_count;
	uint32_t world_leaves_count;
	
	//uint32_t collision_nodes_count;
	
	
	uint32_t light_section_offset;
	
	uint32_t texture_section_offset;
	uint32_t material_section_offset;
	
	uint32_t model_section_offset;
	uint32_t entity_section_offset;
	
	
	//uint32_t light_count;
	//uint32_t material_count;
	//uint32_t model_count;
	//uint32_t entity_def_count;
	//uint32_t entity_count;
	//uint32_t sound_emitter_count;
	//uint32_t particle_system_count;
	//uint32_t texture_count;
	//uint32_t spawn_point_count;
	
	
	uint32_t reserved0;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;
	uint32_t reserved5;
	uint32_t reserved6;
	uint32_t reserved7;
	uint32_t reserved8;
	uint32_t reserved9;
	uint32_t reserved10;
	uint32_t reserved11;
	uint32_t reserved12;
	uint32_t reserved13;
	uint32_t reserved14;
	uint32_t reserved15;
	
}bsp_header_t;





/*
===================================================================
===================================================================
===================================================================
*/


void bsp_SaveBsp(char *file_name);

void bsp_LoadBsp(char *file_name);


#endif








