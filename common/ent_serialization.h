#ifndef ENT_SERIALIZATION_H
#define ENT_SERIALIZATION_H


#include "ent_common.h"



static char entity_section_header_tag[] = "[entity section start]";

struct entity_section_header_t
{
	/* this field has to be the first... */
	char tag[(sizeof(entity_section_header_tag) + 3) & (~3)];
	
	
	unsigned int entity_count;
	unsigned int entity_def_count;
	
	
	unsigned int reserved0;
	unsigned int reserved1;
	unsigned int reserved2;
	unsigned int reserved3;
	unsigned int reserved4;
	unsigned int reserved5;
	unsigned int reserved6;
	unsigned int reserved7;
};


static char entity_section_tail_tag[] = "[entity section end]";

struct entity_section_tail_t
{
	char tag[(sizeof(entity_section_tail_tag) + 3) & (~3)];
};


/*
*******************************************
*******************************************
*******************************************
*/

enum ENTITY_RECORD_FLAGS
{
	ENTITY_RECORD_FLAG_DEF = 1,
	ENTITY_RECORD_FLAG_DEF_REF = 1 << 1,
	ENTITY_RECORD_FLAG_MODIFIED = 1 << 2,
};


static char entity_record_start_tag[] = "[entity record start]";

struct entity_record_start_t
{
	char tag[(sizeof(entity_record_start_tag) + 3) & (~3)];
	char name[ENTITY_NAME_MAX_LEN];
	char def_name[ENTITY_NAME_MAX_LEN];
	int flags;
};

static char entity_record_end_tag[] = "[entity record end]";

struct entity_record_end_t
{
	char tag[(sizeof(entity_record_end_tag) + 3) & (~3)];
};


/*
*******************************************
*******************************************
*******************************************
*/



static char component_record_tag[] = "[component record]";

struct component_record_t
{
	char tag[(sizeof(component_record_tag) + 3) & (~3)];
	short type;
	short nestled;
	
	union
	{
		struct
		{
			mat3_t orientation;
			vec3_t position;
			vec3_t scale;
			
			//int max_children;
			
			int flags;
		}transform_component;
		
		struct
		{
			char model_name[MODEL_NAME_MAX_LEN];
		}model_component;
		
		struct
		{
			char collider_def_name[COLLIDER_DEF_NAME_MAX_LEN];
		}physics_component;
		
		struct
		{
			int light_count;
		}light_component;
		
		struct
		{
			char script_name[SCRIPT_NAME_MAX_LEN];
		}script_component;
		
		struct
		{
			char camera_name[CAMERA_NAME_MAX_LEN];
			int transform_index;
		}camera_component;
		
		struct
		{
			unsigned char max_data[256];
		}max_component_data;
		
	}component;
};


/*
*******************************************
*******************************************
*******************************************
*/

static char entity_prop_record_tag[] = "[entity prop record]";

struct entity_prop_record_t
{
	char tag[(sizeof(entity_prop_record_tag) + 3) & (~3)];
	char name[ENTITY_NAME_MAX_LEN];
	int size;
};

/*
*******************************************
*******************************************
*******************************************
*/

static char collider_record_start_tag[] = "[collider record start]";

struct collider_record_start_t
{
	char tag[(sizeof(collider_record_start_tag) + 3) & (~3)];
	int type;
	char name[COLLIDER_DEF_NAME_MAX_LEN];
	
	union
	{
		struct
		{
			float radius;
			float height;
			float crouch_height;
			float max_slope_angle;
			float max_step_height;
			float jump_height;
			
		}collider_data;
		
		struct
		{
			char data[128];
		}max_data;
	}collider;
	
};


static char collision_shape_record_tag[] = "[collision shape record]";

struct collision_shape_record_t
{
	char tag[(sizeof(collision_shape_record_tag) + 3) & (~3)];
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	int type;
};


static char collider_record_end_tag[] = "[collider record end]";

struct collider_record_end_t
{
	char tag[(sizeof(collider_record_end_tag) + 3) & (~3)];
};



#ifdef __cplusplus
extern "C"
{
#endif

void entity_WriteComponent(void **buffer, struct component_t *component, int nestled);

void entity_WriteProp(void **buffer, struct entity_prop_t *prop);

void entity_WriteCollider(void **buffer, struct collider_def_t *collider_def);

void entity_SerializeEntities(void **buffer, int *buffer_size, int serialize_defs);



void entity_ReadComponent(void **buffer, struct entity_handle_t entity, struct entity_record_start_t *entity_record);

void entity_ReadProp(void **buffer, struct entity_handle_t entity, struct entity_record_start_t *entity_record);

void entity_ReadCollider(void **buffer);

void entity_DeserializeEntities(void **buffer, int deserialize_defs);


#ifdef __cplusplus
}
#endif



#endif







