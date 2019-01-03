#ifndef ENT_SERIALIZATION_H
#define ENT_SERIALIZATION_H


#include "ent_common.h"
#include "w_common.h"


static char entity_section_start_tag[] = "[entity section start]";

struct entity_section_start_t
{
	/* this field has to be the first... */
	char tag[(sizeof(entity_section_start_tag) + 3) & (~3)];


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


static char entity_section_end_tag[] = "[entity section end]";

struct entity_section_end_t
{
	char tag[(sizeof(entity_section_end_tag) + 3) & (~3)];
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
	ENTITY_RECORD_FLAG_ON_DISK = 1 << 3,
	ENTITY_RECORD_FLAG_FILE_REF = 1 << 4,				/* if a record have this flag, it means it is referencing a file that has to be open... */
};


static char entity_record_start_tag[] = "[entity record start]";

struct entity_record_start_t
{
	char tag[(sizeof(entity_record_start_tag) + 3) & (~3)];
	char name[ENTITY_NAME_MAX_LEN];
	char def_name[ENTITY_NAME_MAX_LEN];
	short flags;
	short entity_flags;
	unsigned int data_skip_offset;
};

static char entity_record_end_tag[] = "[entity record end]";

struct entity_record_end_t
{
	char tag[(sizeof(entity_record_end_tag) + 3) & (~3)];
};

static char entity_file_record_tag[] = "[entity file record]";

struct entity_file_record_t
{
	char tag[(sizeof(entity_file_record_tag) + 3) & (~3)];
	char file_name[PATH_MAX];
};



/*
*******************************************
*******************************************
*******************************************
*/

enum COMPONENT_RECORD_FLAGS
{
    COMPONENT_RECORD_FLAG_NESTLED = 1,              /* this flag signals that a transform component should be nestled to another transform component
                                                    instead of being added to an entity... */

    COMPONENT_RECORD_FLAG_REF = 1 << 1,             /* this flag signals that this transform belongs to an entity def, and should be added to its
                                                    parent transform component... */
};


static char component_record_tag[] = "[component record]";

struct component_record_t
{
	char tag[(sizeof(component_record_tag) + 3) & (~3)];
	short type;
	short flags;
	//short nestled;

	union
	{
		struct
		{
			mat3_t orientation;
			vec3_t position;
			vec3_t scale;

			//int max_children;

			int flags;

			char instance_name[ENTITY_NAME_MAX_LEN];
		}transform_component;

		struct
		{
			char model_name[MODEL_NAME_MAX_LEN];
			char model_file_name[MODEL_NAME_MAX_LEN];
		}model_component;

		struct
		{
			char collider_def_name[COLLIDER_DEF_NAME_MAX_LEN];
			int flags;
		}physics_component;

		struct
		{
			int light_count;
		}light_component;

		struct
		{
			char script_name[SCRIPT_NAME_MAX_LEN];
			char script_file_name[SCRIPT_NAME_MAX_LEN];
		}script_component;

		struct
		{
			char camera_name[R_VIEW_NAME_MAX_LEN];
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
			float max_walk_speed;
			float mass;

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


/*
*******************************************
*******************************************
*******************************************
*/


struct trigger_filter_record_t
{
	char filter_name[ENTITY_PROP_NAME_MAX_LEN];
	int flags;
};



static char trigger_record_tag[] = "[trigger record]";

struct trigger_record_t
{
	char tag[(sizeof(trigger_record_tag) + 3) & (~3)];

	mat3_t orientation;
	vec3_t positon;
	vec3_t scale;

	char trigger_name[ENTITY_TRIGGER_NAME_MAX_LEN];
	char event_name[WORLD_EVENT_NAME_MAX_LEN];
	int filter_count;
	struct trigger_filter_record_t filters[1];
};





/*
*******************************************
*******************************************
*******************************************
*/



#ifdef __cplusplus
extern "C"
{
#endif

void entity_WriteComponent(void **buffer, struct component_t *component, int nestled, int ref, int dry_fire);

void entity_WriteProp(void **buffer, struct entity_prop_t *prop, int dry_fire);

void entity_WriteCollider(void **buffer, struct collider_def_t *collider_def, int dry_fire);

void entity_WriteEntity(void **buffer, struct entity_handle_t entity, struct component_handle_t referencing_transform, int write_def_as_file_ref, int dry_fire);

void entity_SerializeEntities(void **buffer, int *buffer_size, int serialize_defs);

void entity_SerializeEntity(void **buffer, int *buffer_size, struct entity_handle_t entity);

void entity_SerializeEntityDef(void **buffer, int *buffer_size, struct entity_handle_t entity_def);



void entity_ReadComponent(void **buffer, struct entity_handle_t parent_entity, struct entity_handle_t entity, struct entity_record_start_t *entity_record);

void entity_ReadProp(void **buffer, struct entity_handle_t entity, struct entity_record_start_t *entity_record);

void entity_ReadCollider(void **buffer, struct entity_handle_t entity);

struct entity_handle_t entity_ReadEntity(void **buffer, struct entity_handle_t parent);

void entity_DeserializeEntities(void **buffer, int deserialize_defs);

void entity_DeserializeEntityDef(void **buffer);


#ifdef __cplusplus
}
#endif



#endif







