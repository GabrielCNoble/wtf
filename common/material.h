#ifndef MATERIAL_H
#define MATERIAL_H

#include "vector_types.h"

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	
	short diffuse_texture;
	short normal_texture;
	
	short material_index;
	short shader_index;
	
}material_t;


int material_Init();

void material_Finish();

int material_CreateMaterial(char *name, vec4_t base_color, float glossiness, float roughness, short shader_index, short diffuse_texture, short normal_texture);

int material_LoadMaterial(char *file_name);

int material_GetMaterialIndex(char *material_name);

void material_DestroyMaterialIndex(int material_index);

void material_SetMaterial(int material_index);

void material_DestroyAllMaterials();





#endif
