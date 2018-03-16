#ifndef MATERIAL_H
#define MATERIAL_H

#include "vector_types.h"

/* may be too little... */
#define MAX_MATERIALS 128

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	
	unsigned char roughness;
	unsigned char metalness;
	
	short diffuse_texture;
	short normal_texture;
	short height_texture;
	short metalness_texture;
	short roughness_texture;
	
	//short material_index;
	short shader_index;
	
	unsigned short flags;
	short ref_count;
	short draw_group;
	
}material_t;

enum MATERIAL_FLAGS
{
	MATERIAL_INVALID = 1,
	
	MATERIAL_USE_DIFFUSE_TEXTURE = 1 << 1,
	MATERIAL_USE_NORMAL_TEXTURE = 1 << 2,
	MATERIAL_USE_HEIGHT_TEXTURE = 1 << 3,
	MATERIAL_USE_ROUGHNESS_TEXTURE = 1 << 4,
	MATERIAL_USE_METALNESS_TEXTURE = 1 << 5,
	
	MATERIAL_INVERT_NORMAL_X = 1 << 6,
	MATERIAL_INVERT_NORMAL_Y = 1 << 7,
	MATERIAL_USE_CUSTOM_SHADER = 1 << 8,
	MATERIAL_TRANSLUCENT = 1 << 9
};


int material_Init();

void material_Finish();

int material_CreateMaterial(char *name, vec4_t base_color, float metalness, float roughness, short shader_index, short diffuse_texture, short normal_texture);

int material_LoadMaterial(char *file_name);

int material_MaterialIndex(char *material_name);

int material_MaterialIndexRef(char *material_name);

void material_IncRefCount(int material_index);

void material_DecRefCount(int material_index);

void material_DestroyMaterialIndex(int material_index);

void material_SetMaterial(int material_index);

int material_SetMaterialName(char *name, int material_index);

void material_DestroyAllMaterials();





#endif
