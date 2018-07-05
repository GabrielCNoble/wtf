#include "bsp_file.h"
#include "texture.h"
#include "l_main.h"
#include "entity.h"

#include "memory.h"


#include <stdio.h>



void bsp_SaveBsp(char *file_name)
{
	FILE *file;
	bsp_header_t *header;
	texture_section_header_t *texture_section;
	light_section_header_t *light_section;
	entity_section_header_t *entity_section;
	model_section_header_t *model_section;
	
	int file_size;
	
	int texture_buffer_size;
	void *texture_buffer;
	
	int light_buffer_size;
	void *light_buffer;
	
	int entity_buffer_size;
	void *entity_buffer;
	
	int model_buffer_size;
	void *model_buffer;
	
	texture_SerializeTextures(&texture_buffer, &texture_buffer_size);
	light_SerializeLights(&light_buffer, &light_buffer_size);
	entity_SerializeEntities(&entity_buffer, &entity_buffer_size);
	model_SerializeModels(&model_buffer, &model_buffer_size);
	
	texture_section = texture_buffer;
	light_section = light_buffer;
	entity_section = entity_buffer;
	model_section = model_buffer;
	
	file_size = sizeof(bsp_header_t) + texture_buffer_size + light_buffer_size + entity_buffer_size + model_buffer_size;
	header = memory_Malloc(file_size, "bsp_SaveBsp");
	
		
	header->reserved0 = 0;
	header->reserved1 = 0;
	header->reserved2 = 0;
	header->reserved3 = 0;
	header->reserved4 = 0;
	header->reserved5 = 0;
	header->reserved6 = 0;
	header->reserved7 = 0;
	header->reserved8 = 0;
	header->reserved9 = 0;
	header->reserved10 = 0;
	header->reserved11 = 0;
	header->reserved12 = 0;
	header->reserved13 = 0;
	header->reserved14 = 0;
	header->reserved15 = 0;
	
}

void bsp_LoadBsp(char *file_name)
{
	
}
