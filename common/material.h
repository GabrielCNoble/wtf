#ifndef MATERIAL_H
#define MATERIAL_H

#include "vector_types.h"
#include "r_common.h"
#include "camera_types.h"
#include <limits.h>

/* may be too little... */
#define MAX_MATERIALS 128


enum MATERIAL_TEXTURE_TYPES
{
    MATERIAL_TEXTURE_TYPE_FIRST = 0,
    MATERIAL_TEXTURE_TYPE_DIFFUSE = MATERIAL_TEXTURE_TYPE_FIRST,
    MATERIAL_TEXTURE_TYPE_NORMAL,
    MATERIAL_TEXTURE_TYPE_HEIGHT,
    MATERIAL_TEXTURE_TYPE_ROUGHNESS,
    MATERIAL_TEXTURE_TYPE_METALNESS,
    MATERIAL_TEXTURE_TYPE_LAST,
};


typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;

	unsigned char roughness;
	unsigned char metalness;

    union
    {
        struct
        {
            short diffuse_texture;
            short normal_texture;
            short height_texture;
            short metalness_texture;
            short roughness_texture;
        };

        short textures[MATERIAL_TEXTURE_TYPE_LAST];
    };


	//short material_index;
	short shader_index;					/* this should go away... */

	unsigned short flags;
	short ref_count;
	short draw_group;

	unsigned int frame_ref_count;
	unsigned int last_ref_frame;

	draw_command_group_t *r_draw_group;

	char *name;

}material_t;


/* level editor specific
kludge... */
typedef struct
{
	char *base_name;
	char used_suffixes[MAX_MATERIALS >> 3];
}material_name_record_t;





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



/*
===================================================================
===================================================================
===================================================================
*/

static char material_section_start_tag[] = "[material section start]";

struct material_section_start_t
{
	char tag[(sizeof(material_section_start_tag) + 3) & (~3)];				/* strlen("[material_section]\0\0"); */

	int material_count;

	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;
};

static char material_section_end_tag[] = "[material section end]";

struct material_section_end_t
{
	char tag[(sizeof(material_section_end_tag) + 3) & (~3)];
};


static char material_record_tag[] = "[material record]";

struct material_record_t
{

	char tag[(sizeof(material_record_tag) + 3) & (~3)];

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



/*
===================================================================
===================================================================
===================================================================
*/


#ifdef __cplusplus
extern "C"
{
#endif

int material_Init();

void material_Finish();

int material_CreateMaterial(char *name, vec4_t base_color, float metalness, float roughness, short shader_index, short diffuse_texture, short normal_texture);

//void material_GetNameBaseAndSuffix(char *name, char *base, char *suffix, int *suffix_pos, int *suffix_byte_index, int *suffix_bit_index);

//char *material_AddNameRecord(char *name);

//void material_RemoveNameRecord(char *name);

int material_MaterialIndex(char *material_name);

int material_MaterialIndexRef(char *material_name);


int material_IncRefCount(int material_index);

int material_DecRefCount(int material_index);

int material_OpRefCount(int material_index, int count);


int material_IncCurrentFrameRefCount(int material_index);

//int material_IncCurrentFrameRefCountView(int material_index, camera_t *view);



void material_DestroyMaterialIndex(int material_index);

int material_SetMaterialName(char *name, int material_index);

void material_SetMaterialTexture(int material_index, int texture_type, int texture_index);

//char *material_GetMaterialName(int material_index);

int material_GetMaterial(char *material_name);

material_t *material_GetMaterialPointer(char *material_name);

material_t *material_GetMaterialPointerIndex(int material_index);

void material_DestroyAllMaterials();




/*
===============================================
===============================================
===============================================
*/

/* this function modifies the buffer parameter... */
//void material_WriteMaterialRecord(material_t *material, char *material_name, void **buffer);

void material_SerializeMaterials(void **buffer, int *buffer_size);


/* this function modifies the buffer parameter... */
//void material_ReadMaterialRecord(material_record_t *record, void **buffer);

void material_DeserializeMaterials(void **buffer);
/*
===============================================
===============================================
===============================================
*/


#ifdef __cplusplus
}
#endif




#endif
