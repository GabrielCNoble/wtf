#include "ent_serialization.h"
#include "camera.h"
#include "entity.h"
#include "script.h"

#include "stack_list.h"

#include "c_memory.h"

#include <assert.h>
#include <string.h>


extern struct stack_list_t ent_entities[2];
extern struct stack_list_t ent_components[2][COMPONENT_TYPE_LAST];
extern struct stack_list_t ent_triggers;


#ifdef __cplusplus
extern "C"
{
#endif

struct component_record_t *entity_SerializeComponent(void **buffer)
{
	struct component_record_t *component_record;
	char *in;
	in = *buffer;


	component_record = (struct component_record_t *)in;
	in += sizeof(struct component_record_t );

	*buffer = in;

	memset(component_record, 0, sizeof(struct component_record_t));

	return component_record;
}

void entity_WriteComponent(void **buffer, struct component_t *component, int nestled, int ref)
{
	struct component_record_t *component_record;
	struct transform_component_t *transform_component;
	struct transform_component_t *parent_transform_component;

	struct camera_component_t *camera_component;
	struct model_component_t *model_component;
	struct script_component_t *script_component;
	struct physics_component_t *physics_component;

	struct collider_t *collider;
	struct model_t *model;

	struct entity_t *entity;

	int i;


	camera_t *camera;

	char *in;

	in = *buffer;

	if(component->type != COMPONENT_TYPE_NONE)
	{

		if(component->flags & COMPONENT_FLAG_SERIALIZED)
		{
			return;
		}

		component->flags |= COMPONENT_FLAG_SERIALIZED;

		//component_record = (struct component_record_t *)in;
		//in += sizeof(struct component_record_t);
		//*buffer = in;

		//memset(component_record, 0, sizeof(struct component_record_t));

		component_record = entity_SerializeComponent(buffer);

		strcpy(component_record->tag, component_record_tag);

		nestled = nestled && 1;

		component_record->type = component->type;

		if(nestled)
        {
            component_record->flags |= COMPONENT_RECORD_FLAG_NESTLED;
        }

        if(ref)
        {
            component_record->flags |= COMPONENT_RECORD_FLAG_REF;
        }


		switch(component->type)
		{
			case COMPONENT_TYPE_TRANSFORM:
				transform_component = (struct transform_component_t *)component;

				component_record->component.transform_component.orientation = transform_component->orientation;
				component_record->component.transform_component.position = transform_component->position;
				component_record->component.transform_component.scale = transform_component->scale;

				if(transform_component->instance_name)
				{
					strcpy(component_record->component.transform_component.instance_name, transform_component->instance_name);
				}

			break;

			case COMPONENT_TYPE_CAMERA:
				camera_component = (struct camera_component_t *)component;

				//strcpy(component_record->component.camera_component.camera_name, camera_component->camera->name);
				//entity = entity_GetEntityPointerHandle(camera_component->base.entity);
				//transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
				/* write the transform linked to this camera component, flagging it as nestled so it
				won't get added to an entity when being read back... */
				//entity_WriteComponent(buffer, (struct component_t *)transform_component, 1);
			break;

			case COMPONENT_TYPE_PHYSICS:
				physics_component = (struct physics_component_t *)component;
				if(!component->entity.def)
				{
					collider = physics_GetColliderPointerHandle(physics_component->collider.collider_handle);
					strcpy(component_record->component.physics_component.collider_def_name, collider->def->name);
				}
				else
				{
					strcpy(component_record->component.physics_component.collider_def_name, physics_component->collider.collider_def->name);
					entity_WriteCollider(buffer, physics_component->collider.collider_def);
				}

				component_record->component.physics_component.flags = physics_component->flags;

			break;

			case COMPONENT_TYPE_SCRIPT:

				script_component = (struct script_component_t *)component;

				if(script_component->script)
				{
					strcpy(component_record->component.script_component.script_name, script_component->script->name);
					strcpy(component_record->component.script_component.script_file_name, script_component->script->file_name);
				}
			break;

			case COMPONENT_TYPE_MODEL:

				model_component = (struct model_component_t *)component;

				model = model_GetModelPointerIndex(model_component->model_index);

				if(model)
				{
					strcpy(component_record->component.model_component.model_name, model->name);
					strcpy(component_record->component.model_component.model_file_name, model->file_name);
				}
			break;
		}

	}
}

struct component_record_t *entity_DeserializeComponent(void **buffer)
{
	struct component_record_t *component_record;
	char *in;

	in = (char *)*buffer;


	while(1)
	{
		if(!strcmp(in, component_record_tag))
		{
			component_record = (struct component_record_t *)in;
			in += sizeof(struct component_record_t);
			break;
		}
		in++;
	}

	*buffer = in;

	return component_record;
}

void entity_ReadComponent(void **buffer, struct entity_handle_t parent_entity, struct entity_handle_t entity, struct entity_record_start_t *entity_record)
{
	char *in;
	struct component_record_t *component_record;
	struct component_record_t *camera_transform_component_record;

	int model_index;

	struct script_t *script;

	struct component_handle_t handle;
	struct component_t *component;
	struct camera_component_t *camera_component;
	struct transform_component_t *transform_component;

	struct physics_component_t *physics_component;
	struct model_component_t *model_component;
	struct script_component_t *script_component;

	struct collider_def_t *collider_def;

	struct entity_t *entity_ptr;
	struct entity_t *parent_entity_ptr;

	in = *buffer;

	component_record = entity_DeserializeComponent(buffer);


	entity_ptr = entity_GetEntityPointerHandle(entity);
	parent_entity_ptr = entity_GetEntityPointerHandle(parent_entity);

	if(component_record->flags & COMPONENT_RECORD_FLAG_NESTLED)
	{
		/* this component record belongs to a child transform component of an entity's transform component.
		Parent it to the entity's transform component instead of adding to the entity itself... */

		/* transform components belonging to entity def refs will also be flagged as nestled, given that
		they also have to be linked to their parent entity's transform component... */
		handle = entity_AllocComponent(component_record->type, entity.def);
		entity_ParentTransformComponent(parent_entity_ptr->components[COMPONENT_TYPE_TRANSFORM], handle);
	}
	else
	{
		if((entity_record->flags & ENTITY_RECORD_FLAG_DEF) || (entity_record->flags & ENTITY_RECORD_FLAG_MODIFIED))
		{
			/* if this is an entity definition or if this entity got modified after being spawned... */
			handle = entity_AddComponent(entity, component_record->type);
		}
		else
		{
			/* this is an unmodified entity, which means that it's components were already allocated, and
			we'll just be setting their values... */
			handle = entity_ptr->components[component_record->type];
		}

	}

	component = entity_GetComponentPointer(handle);

	switch(component_record->type)
	{
		case COMPONENT_TYPE_TRANSFORM:
			transform_component = (struct transform_component_t *)component;
			/*transform_component->orientation = component_record->component.transform_component.orientation;
			transform_component->position = component_record->component.transform_component.position;
			transform_component->scale = component_record->component.transform_component.scale;*/



			//if(entity_record->flags & ENTITY_RECORD_FLAG_DEF_REF)
			if(component_record->flags & COMPONENT_RECORD_FLAG_REF)
			{
				transform_component->orientation = component_record->component.transform_component.orientation;
				transform_component->position = component_record->component.transform_component.position;
				transform_component->scale = component_record->component.transform_component.scale;

				if(!transform_component->instance_name)
				{
					transform_component->instance_name = memory_Malloc(ENTITY_NAME_MAX_LEN);
				}

				strcpy(transform_component->instance_name, component_record->component.transform_component.instance_name);


				/* if this is a reference to a def we need to make sure this
				newly allocated transform component points to the original
				def... */
				transform_component = entity_GetComponentPointer(parent_entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				component = entity_GetComponentPointer(transform_component->child_transforms[transform_component->children_count - 1]);
				component->flags |= COMPONENT_FLAG_ENTITY_DEF_REF;
				component->entity = entity_GetEntityHandle(entity_record->name, entity_record->flags & ENTITY_RECORD_FLAG_DEF);
			}
			else
			{
				entity_SetTransform(entity, &component_record->component.transform_component.orientation,
											 component_record->component.transform_component.position,
											 component_record->component.transform_component.scale, 0);
			}

		break;

		case COMPONENT_TYPE_CAMERA:
			if(!entity.def)
			{
				camera_component->camera = camera_CreateCamera("camera", vec3_t_c(0.0, 0.0, 0.0), NULL, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
			}
		break;

		case COMPONENT_TYPE_PHYSICS:
			physics_component = (struct physics_component_t *)component;

			if(!entity.def)
			{
				collider_def = physics_GetColliderDefPointer(component_record->component.physics_component.collider_def_name);
				transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				physics_component->collider.collider_handle = physics_CreateCollider(&transform_component->orientation, transform_component->position, transform_component->scale, collider_def, 0);
				physics_component->flags = component_record->component.physics_component.flags;
			}

		break;

		case COMPONENT_TYPE_SCRIPT:
			script_component = (struct script_component_t *)component;

			script = script_GetScript(component_record->component.script_component.script_name);

			if(!script)
			{
				script = (struct script_t *)entity_LoadScript(component_record->component.script_component.script_file_name, component_record->component.script_component.script_name);

				if(!script)
				{
					printf("entity_ReadComponent: couldn't find script file [%s] for script component of entity [%s]\n", component_record->component.script_component.script_file_name, entity_ptr->name);
				}
			}

			script_component->script = script;

		break;

		case COMPONENT_TYPE_MODEL:
			model_component = (struct model_component_t *)component;

			model_index = model_GetModel(component_record->component.model_component.model_name);

			if(model_index < 0)
			{
				/* .ent files are supposed to be self-contained, which means they'll try to load by themselves the information they need... */
                model_index = model_LoadModel(component_record->component.model_component.model_file_name, component_record->component.model_component.model_name);

                if(model_index < 0)
				{
					printf("entity_ReadComponent: couldn't find model file [%s] for model component of entity [%s]\n", component_record->component.model_component.model_file_name, entity_ptr->name);
				}
			}

			model_component->model_index = model_index;
		break;
	}
}



void entity_WriteProp(void **buffer, struct entity_prop_t *prop)
{
	char *in;
	struct entity_prop_record_t *prop_record;
	void *value;
	in = *buffer;

	if(prop)
	{
		prop_record = (struct entity_prop_record_t *)in;
		in += sizeof(struct entity_prop_record_t);

		memset(prop_record, 0, sizeof(struct entity_prop_record_t));
		strcpy(prop_record->tag, entity_prop_record_tag);
		strcpy(prop_record->name, prop->name);

		prop_record->size = prop->size;

		/* the prop value gets written right after the record... */
		value = (char *)prop_record + sizeof(struct entity_prop_record_t);

		memcpy(value, prop->memory, prop_record->size);

		in += prop_record->size;
		*buffer = in;
	}

}


void entity_ReadProp(void **buffer, struct entity_handle_t entity, struct entity_record_start_t *entity_record)
{
	char *in;
	struct entity_prop_record_t *prop_record;
	void *value;

	in = *buffer;

	prop_record = (struct entity_prop_record_t *)in;
	in += sizeof(struct entity_prop_record_t) + prop_record->size;

	*buffer = in;

	value = (char *)prop_record + sizeof(struct entity_prop_record_t);

	if((entity_record->flags & ENTITY_RECORD_FLAG_DEF) || (entity_record->flags & ENTITY_RECORD_FLAG_MODIFIED))
	{
		/* if this is an entity def or this is an post-spawn modified entity, we need to add this prop to it... */
		entity_AddProp(entity, prop_record->name, prop_record->size);
	}

	/* spawned entities always get their props updated... */
	entity_SetProp(entity, prop_record->name, value);
}


void entity_WriteCollider(void **buffer, struct collider_def_t *collider_def)
{
	struct physics_component_t *physics_component;
	struct entity_t *entity_ptr;

	struct collider_record_start_t *collider_record_start;
	struct collider_record_end_t *collider_record_end;

	struct collision_shape_record_t *collision_shape_record;

	char *out;
	int i;

	out = *buffer;

	if(collider_def)
	{
		collider_record_start = (struct collider_record_start_t *)out;
		out += sizeof(struct collider_record_start_t );

		memset(collider_record_start, 0, sizeof(struct collider_record_start_t ));
		strcpy(collider_record_start->tag, collider_record_start_tag);

		collider_record_start->type = collider_def->type;
		strcpy(collider_record_start->name, collider_def->name);

		collider_record_start->collider.collider_data.mass = collider_def->mass;

		switch(collider_def->type)
		{
			case COLLIDER_TYPE_CHARACTER_COLLIDER:
				collider_record_start->collider.collider_data.radius = collider_def->radius;
				collider_record_start->collider.collider_data.height = collider_def->height;
				collider_record_start->collider.collider_data.max_slope_angle = collider_def->max_slope_angle;
				collider_record_start->collider.collider_data.max_step_height = collider_def->max_step_height;
				collider_record_start->collider.collider_data.jump_height = 0.0;
				collider_record_start->collider.collider_data.max_walk_speed = collider_def->max_walk_speed;
				collider_record_start->collider.collider_data.mass = collider_def->mass;
			break;

			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
				for(i = 0; i < collider_def->collision_shape_count; i++)
				{
					collision_shape_record = (struct collision_shape_record_t *)out;
					out += sizeof(struct collision_shape_record_t );

					memset(collision_shape_record, 0, sizeof(struct collision_shape_record_t ));
					strcpy(collision_shape_record->tag, collision_shape_record_tag);

					collision_shape_record->type = collider_def->collision_shape[i].type;
					collision_shape_record->orientation = collider_def->collision_shape[i].orientation;
					collision_shape_record->position = collider_def->collision_shape[i].position;
					collision_shape_record->scale = collider_def->collision_shape[i].scale;
				}
			break;
		}

		collider_record_end = (struct collider_record_end_t *)out;
		out += sizeof(struct collider_record_end_t );

		memset(collider_record_end, 0, sizeof(struct collider_record_end_t ));
		strcpy(collider_record_end->tag, collider_record_end_tag);

		*buffer = out;
	}
}

void entity_ReadCollider(void **buffer, struct entity_handle_t entity)
{
	struct collider_def_t *def;

	struct collider_record_start_t *collider_record_start;
	struct collider_record_end_t *collider_record_end;
	struct collision_shape_record_t *collision_shape_record;

	struct entity_t *entity_ptr;
	struct physics_component_t *physics_component;

	char *in;

	int i;

	in = *buffer;
	collider_record_start = (struct collider_record_start_t *)in;
	in += sizeof(struct collider_record_start_t );


	switch(collider_record_start->type)
	{
		case COLLIDER_TYPE_CHARACTER_COLLIDER:
			def = physics_CreateCharacterColliderDef(collider_record_start->name, collider_record_start->collider.collider_data.height,
																			      collider_record_start->collider.collider_data.crouch_height,
																				  collider_record_start->collider.collider_data.radius,
																				  collider_record_start->collider.collider_data.max_step_height,
																				  collider_record_start->collider.collider_data.max_slope_angle,
																				  collider_record_start->collider.collider_data.max_walk_speed,
																				  collider_record_start->collider.collider_data.mass);
		break;

		case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
			def = physics_CreateRigidBodyColliderDef(collider_record_start->name);
			def->mass = collider_record_start->collider.collider_data.mass;
			while(1)
			{
				if(!strcmp(in, collision_shape_record_tag))
				{
					collision_shape_record = (struct collision_shape_record_t *)in;
					in += sizeof(struct collision_shape_record_t);

					physics_AddCollisionShape(def, collision_shape_record->scale, collision_shape_record->position, &collision_shape_record->orientation, collision_shape_record->type);

				}
				else if(!strcmp(in, collider_record_end_tag))
				{
					break;
				}
				else
				{
					in++;
				}

			}

		break;
	}


	entity_ptr = entity_GetEntityPointerHandle(entity);
	physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

	assert(physics_component);

	physics_component->collider.collider_def = def;


	collider_record_end = (struct collider_record_end_t *)in;
	in += sizeof(struct collider_record_end_t);

	*buffer = in;
}



void entity_WriteEntity(void **buffer, struct entity_handle_t entity, struct component_handle_t referencing_transform, int write_def_as_file_ref)
{
	struct entity_record_start_t *ent_record_start;
	struct entity_record_end_t *ent_record_end;
	struct entity_file_record_t *ent_file_record;
	struct entity_source_file_t *ent_source_file;

	struct entity_t *entity_def_ptr;
	struct entity_t *entity_ptr;

	struct component_t *component;

	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform_component;

	struct stack_list_t *list;

	struct entity_prop_t *props;

	int i;
	int j;
	int k;

	char *out;
	char *record_start;
	out = *buffer;

	entity_ptr = entity_GetEntityPointerHandle(entity);

	static int depth_level = -1;

	depth_level++;

	int write_entity_data = 1;

	if(entity_ptr)
	{
		if(entity_ptr->flags & ENTITY_FLAG_SERIALIZED)
		{
			if(entity.def)
			{
				/* this is a reference to an entity def... */
				if(depth_level < 1)
				{
					/* if this ref isn't inside a hierarchy, don't serialize anything (an entity def has to be referenced FROM another entity...) */
					depth_level--;
					return;
				}

				/* Otherwise, we just need to serialize its
				transform without worrying about any children at all... */
			//	write_entity_data = 0;
			}
			else
			{
				/* this is an entity instance, and it was already
				serialized, so don't do it again... */
				depth_level--;
				return;
			}

		}

		/* write record start... */
		ent_record_start = (struct entity_record_start_t *)out;
		out += sizeof(struct entity_record_start_t);

		record_start = out;

		memset(ent_record_start, 0, sizeof(struct entity_record_start_t));
		strcpy(ent_record_start->tag, entity_record_start_tag);
		strcpy(ent_record_start->name, entity_ptr->name);

		ent_record_start->entity_flags = entity_ptr->flags;

		transform_component = entity_GetComponentPointer(referencing_transform);

	    if(transform_component)
        {
            if(transform_component->flags & COMPONENT_FLAG_ENTITY_DEF_REF)
            {
            //	if(entity_ptr->ref_count > 1)
			//	{
				/* if the transform that brought us here is used to only
				reference an entity def, mark the record as belonging to an
				entity def ref... */
				ent_record_start->flags |= ENTITY_RECORD_FLAG_DEF_REF;
			//	}

            }
        }

		if(!entity.def)
		{
			entity_def_ptr = entity_GetEntityPointerHandle(entity_ptr->def);
			strcpy(ent_record_start->def_name, entity_def_ptr->name);
		}
		else
		{
			ent_record_start->flags |= ENTITY_RECORD_FLAG_DEF;
		}

		if(entity_ptr->flags & ENTITY_FLAG_MODIFIED)
		{
			ent_record_start->flags |= ENTITY_RECORD_FLAG_MODIFIED;
		}



		if((ent_record_start->flags & ENTITY_RECORD_FLAG_DEF) && (!(ent_record_start->flags & ENTITY_RECORD_FLAG_DEF_REF)) && write_def_as_file_ref && (entity_ptr->flags & ENTITY_FLAG_ON_DISK))
		{
			/* don't serialize this entity def data, but instead just the file name
			where it exists... */

			ent_record_start->flags |= ENTITY_RECORD_FLAG_FILE_REF;

			ent_file_record = (struct entity_file_record_t *)out;
			out += sizeof(struct entity_file_record_t);

			memset(ent_file_record, 0, sizeof(struct entity_file_record_t));

			ent_source_file = entity_GetSourceFile(entity);
			strcpy(ent_file_record->tag, entity_file_record_tag);
            strcpy(ent_file_record->file_name, ent_source_file->file_name);
		}
		else
		{

			entity_ptr->flags |= ENTITY_FLAG_SERIALIZED;

			props = (struct entity_prop_t *)entity_ptr->props.elements;

			/* Always write props, regardless of the entity having been modified or not... */
			for(i = 0; i < entity_ptr->props.element_count; i++)
			{
				entity_WriteProp((void **)&out, props + i);
			}

			if(entity.def || (entity_ptr->flags & ENTITY_FLAG_MODIFIED))
			{
				/* If this is a def or it is a post-spawn modified entity,
				write its stuff down... */

				/* write components... */
				for(i = 0; i < COMPONENT_TYPE_LAST; i++)
				{
					if(entity_ptr->components[i].type != COMPONENT_TYPE_NONE)
					{
						component = entity_GetComponentPointer(entity_ptr->components[i]);
						entity_WriteComponent((void **)&out, component, 0, 0);
					}
				}


				k = (ent_record_start->flags & ENTITY_RECORD_FLAG_DEF_REF) && 1;

				transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

				/* write nestled transforms... */
				for(j = 0; j <= k; j++)
				{
					for(i = 0; i < transform_component->children_count; i++)
					{
						component = entity_GetComponentPointer(transform_component->child_transforms[i]);

						if(component->entity.entity_index != INVALID_ENTITY_INDEX)
						{
						   /* this child transform component points to an entity,
						   so write it to the buffer before continuing with the
						   current entity... */
						   entity_WriteEntity((void **)&out, component->entity, transform_component->child_transforms[i], write_def_as_file_ref);
						}
						else if(entity_ptr->flags & ENTITY_FLAG_MODIFIED)
						{
							/* This nestled transform belongs to some component
							that needs to keep spatial information (light, camera,
							particle system, etc).

							This just gets written if this isn't a reference to
							an entity def or it is a post-spawn modified entity... */
							entity_WriteComponent((void **)&out, component, 1, k);
						}
					}

					if(!j)
					{
						/* .ent files are supposed to be self-contained, meaning that
						they should not rely on other files to properly load their
						contents. To allow this we serialize the full entity def they
						reference, so if it isn't already loaded when this ref gets
						deserialized, it has the data it needs.

						In the case where the entity def is already loaded when this
						ref gets deserialized, we only skip all the data from the
						def it references, and only load the data this ref has.

						This is also useful to avoid loading the same entity def
						data twice. If the entity def already exists, the loader
						will simply skip over the data to the end record tag... */
						ent_record_start->data_skip_offset = out - record_start;
					}

					if(j < k)
					{
						/* We write this ref's transform component, so we can have both
						nestled transforms from the original def and from this ref... */
						transform_component = entity_GetComponentPointer(referencing_transform);
						entity_WriteComponent((void **)&out, (struct component_t *)transform_component, 1, 1);
					}
				}
			}
			else if(depth_level == 0)
			{
				/* if this is the root of the hierarchy of an unmodified entity, write
				only it's transform... */
				component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				entity_WriteComponent((void **)&out, component, 0, 0);
			}
		}





		/* write record end... */
		ent_record_end = (struct entity_record_end_t *)out;
		out += sizeof(struct entity_record_end_t );

		memset(ent_record_end, 0, sizeof(struct entity_record_end_t));
		strcpy(ent_record_end->tag, entity_record_end_tag);

		*buffer = out;
	}

	depth_level--;
}

struct entity_handle_t entity_ReadEntity(void **buffer, struct entity_handle_t parent)
{
	struct entity_record_start_t *ent_record_start;
	struct entity_record_start_t *nestled_ent_record_start;
	struct entity_record_end_t *ent_record_end;
	struct entity_file_record_t *ent_file_record;

	struct entity_handle_t handle;
	struct entity_handle_t entity_def;
	struct entity_t *entity;
	struct entity_t *parent_entity;

	struct component_t *component;
	struct transform_component_t *parent_transform_component;

	int load_entity_data = 1;

	char *in;
	int i;
	int loop = 1;

	in = *buffer;

	ent_record_start = (struct entity_record_start_t *)in;
	in += sizeof(struct entity_record_start_t );

	i = 0;

	static int depth_level = -1;

	depth_level++;


	if(ent_record_start->flags & ENTITY_RECORD_FLAG_DEF_REF)
	{

	    entity = entity_GetEntityPointer(ent_record_start->name, 1);

	    if(entity)
        {
            /* The entity def being referenced already exists,
            so we just load this ref specific data... */
            in += ent_record_start->data_skip_offset;


            /* This record belongs to a reference to an entity def, which means that we'll be reading only a transform component
            that will point to the original entity def. This transform won't be added to the original def, but instead will be to the
            child list of the parent entity of this reference. Here we make the handle to the entity be invalid because we won't be
            accessing the original def anyway... */
            handle = INVALID_ENTITY_HANDLE;
        }
        else
        {
        	//handle = entity_GetEntityHandle(ent_record_start->name, 1);

        	//if(handle.entity_index == INVALID_ENTITY_INDEX)
			//{
			handle = entity_CreateEntity(ent_record_start->name, ent_record_start->flags & ENTITY_RECORD_FLAG_DEF);
			//}
			//else
			//{
			//	in += ent_record_start->def_ref_skip_offset;
			//}
        }


	}
	else
	{
		if((ent_record_start->flags & ENTITY_RECORD_FLAG_DEF) || (ent_record_start->flags & ENTITY_RECORD_FLAG_MODIFIED))
		{
			/* if this entity record is belongs to a file reference, we don't create a entity
			here, but instead just load it from the disk... */
			if(!(ent_record_start->flags & ENTITY_RECORD_FLAG_FILE_REF))
			{
				/* This record belongs to an entity def or to post-spawned modified entity, so we create a new entity here... */

				handle = entity_GetEntityHandle(ent_record_start->name, 1);

				if(handle.entity_index == INVALID_ENTITY_INDEX)
				{
					handle = entity_CreateEntity(ent_record_start->name, ent_record_start->flags & ENTITY_RECORD_FLAG_DEF);
				}
				else
				{
					in += ent_record_start->data_skip_offset;
					//load_entity_data = 0;
				}

			}
			else
			{
				handle = INVALID_ENTITY_HANDLE;
			}
		}
		else
		{
			/* this record belong to an unmodified entity... */
			if(!depth_level)
			{
				/* ...if we're at the top of the hierarchy,
				we use it's def to spawn it... */
				entity_def = entity_GetEntityHandle(ent_record_start->def_name, 1);
				handle = entity_SpawnEntity(NULL, vec3_t_c(0.0, 0.0, 0.0), vec3_t_c(1.0, 1.0, 1.0), entity_def, ent_record_start->name);
			}
			else
			{
				/* if we're deeper in the hierarchy, this entity was already spawned, so we just get a handle to it, and we'll
				be pretty much just reading props... */
				handle = entity_GetNestledEntityHandle(parent, ent_record_start->name);
			}

		}

	}

	while(loop)
	{
		if(*in == '[')
		{
			if(!strcmp(in, entity_record_end_tag))
			{
				ent_record_end = (struct entity_record_end_t *)in;
				in += sizeof(struct entity_record_end_t);

				loop = 0;
				continue;
			}
			else if(!strcmp(in, entity_record_start_tag))
			{
				entity_ReadEntity((void **)&in, handle);
			}
			else if(!strcmp(in, component_record_tag))
			{
				entity_ReadComponent((void **)&in, parent, handle, ent_record_start);
			}
			else if(!strcmp(in, collider_record_start_tag))
			{
				entity_ReadCollider((void **)&in, handle);
			}
			else if(!strcmp(in, entity_prop_record_tag))
			{
				entity_ReadProp((void **)&in, handle, ent_record_start);
			}
			else if(!strcmp(in, entity_file_record_tag))
			{
				ent_file_record = (struct entity_file_record_t *)in;
				in += sizeof(struct entity_file_record_t);
				handle = entity_LoadEntityDef(ent_file_record->file_name);
			}
			else
			{
				in++;
			}
		}
		else
		{
			in++;
		}


	}

//	_gtfo:

	entity = entity_GetEntityPointerHandle(handle);
    parent_entity = entity_GetEntityPointerHandle(parent);

	if(parent_entity)
	{
		if((ent_record_start->flags & ENTITY_RECORD_FLAG_DEF) && (!(ent_record_start->flags & ENTITY_RECORD_FLAG_DEF_REF)))
		{
		    /* if this is an entity def ref, there's no need
		    to parent it to it's parent entity def since it
		    was already parented when it's parent read it's
		    nestled transforms... */
			entity_ParentEntity(parent, handle);

			/* this def was loaded from a buffer that's likely come
			from a file, so mark the entity as existing in a file... */
			//entity->flags |= ENTITY_FLAG_ON_DISK;

		}
	}



//	parent_entity = entity_GetEntityPointerHandle(parent);



	*buffer = in;

	depth_level--;

	return handle;
}


/*
*******************************************
*******************************************
*******************************************
*/



void entity_SerializeEntities(void **buffer, int *buffer_size, int serialize_defs)
{
	struct entity_section_start_t *section_start;
	struct entity_section_end_t *section_end;
	struct trigger_record_t *trigger_record;
	struct entity_t *entity;
	struct entity_handle_t handle;
	struct component_t *component;

	struct trigger_t *triggers;
	struct trigger_t *trigger;

	struct trigger_filter_t *filters;
	struct trigger_filter_t *filter;

	struct transform_component_t *transform_component;

	struct stack_list_t *list;

	struct entity_prop_t *props;

	int entity_def_count = 0;
	int entity_count = 0;

	int write_entity_def;

	int i;
	int j;
	int k;
	int l;
	char *out;
	int out_size = 0;

	out_size = sizeof(struct entity_section_start_t) + sizeof(struct entity_section_end_t);


	/* clear the COMPONENT_FLAG_SERIALIZED from every component... */
	for(i = 0; i < 2; i++)
	{
		for(j = 0; j < COMPONENT_TYPE_LAST; j++)
		{
			l = ent_components[i][j].element_count;

			for(k = 0; k < l; k++)
			{
				component = (struct component_t *)((char *)ent_components[i][j].elements + ent_components[i][j].element_size * k);
				component->flags &= ~COMPONENT_FLAG_SERIALIZED;
			}
		}
	}



	serialize_defs = serialize_defs && 1;

	for(write_entity_def = 0; write_entity_def <= serialize_defs; write_entity_def++)
	{
		list = &ent_entities[write_entity_def];
		j = list->element_count;

		handle.def = write_entity_def;

		for(i = 0; i < j; i++)
		{
			handle.entity_index = i;

			entity = entity_GetEntityPointerHandle(handle);

			if(!entity)
			{
				continue;
			}

			entity->flags &= ~ENTITY_FLAG_SERIALIZED;

			if(entity)
			{
				/* record start + end... */
				out_size += sizeof(struct entity_record_start_t) + sizeof(struct entity_record_end_t);

				/* components... */
				for(k = 0; k < COMPONENT_TYPE_LAST; k++)
				{
					if(entity->components[k].type != COMPONENT_TYPE_NONE)
					{
						out_size += sizeof(struct component_record_t);
					}
				}

				props = (struct entity_prop_t *)entity->props.elements;

				for(k = 0; k < entity->props.element_count; k++)
				{
					/* props... */
					out_size += sizeof(struct entity_prop_record_t) + props[k].size;
				}




				/* nestled transforms... */
				transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
				out_size += sizeof(struct component_record_t) * transform_component->children_count;
			}
		}
	}

	triggers = (struct trigger_t *)ent_triggers.elements;

	for(i = 0; i < ent_triggers.element_count; i++)
	{
		trigger = triggers + i;

		if(trigger->flags & TRIGGER_FLAG_INVALID)
		{
			continue;
		}

        out_size += sizeof(struct trigger_record_t) + sizeof(struct trigger_filter_record_t) * (trigger->trigger_filters.element_count - 1);
	}

	out = memory_Calloc(2, out_size);

	*buffer = out;
	*buffer_size = out_size;
	section_start = (struct entity_section_start_t *)out;
	out += sizeof(struct entity_section_start_t);

	//memset(header, 0, sizeof(struct entity_section_header_t));
	strcpy(section_start->tag, entity_section_start_tag);

	section_start->entity_count = entity_count;
	section_start->entity_def_count = entity_def_count;



	for(i = 0; i < ent_triggers.element_count; i++)
	{
        trigger = triggers + i;

		if(trigger->flags & TRIGGER_FLAG_INVALID)
		{
			continue;
		}


        trigger_record = (struct trigger_record_t *)out;
        out += sizeof(struct trigger_record_t) + sizeof(struct trigger_filter_record_t) * (trigger->trigger_filters.element_count - 1);

        strcpy(trigger_record->tag, trigger_record_tag);

        strcpy(trigger_record->trigger_name, trigger->name);

        if(trigger->event_name)
		{
			strcpy(trigger_record->event_name, trigger->event_name);
		}

		trigger_record->orientation = trigger->orientation;
		trigger_record->positon = trigger->position;
		trigger_record->scale = trigger->scale;

		trigger_record->filter_count = trigger->trigger_filters.element_count;

		filters = (struct trigger_filter_t *)trigger->trigger_filters.elements;

		for(j = 0; j < trigger->trigger_filters.element_count; j++)
		{
			filter = filters + j;
			strcpy(trigger_record->filters[j].filter_name, filter->prop_name);
			trigger_record->filters[j].flags = filter->flag;
		}
	}




	for(k = serialize_defs; k >= 0; k--)
	{
		list = &ent_entities[k];
		j = list->element_count;
		handle.def = k;

		for(i = 0; i < j; i++)
		{
			handle.entity_index = i;

			entity = entity_GetEntityPointerHandle(handle);

			if(entity)
			{
				transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

				if(transform_component)
				{
					if(transform_component->parent.type == COMPONENT_TYPE_NONE && (!(entity->flags & ENTITY_FLAG_SERIALIZED)))
					{
						//if(!(entity->flags & ENTITY_FLAG_ON_DISK))
						entity_WriteEntity((void **)&out, handle, INVALID_COMPONENT_HANDLE, 1);
					}
				}
			}
		}
	}

	section_end = (struct entity_section_end_t *)out;
	out += sizeof(struct entity_section_end_t );

	//memset(tail, 0, sizeof(struct entity_section_tail_t));
	strcpy(section_end->tag, entity_section_end_tag);



	printf("entity_SerializeEntities - allocd space: %d bytes --- used space: %d bytes\n", out_size, out - (char *)*buffer);
}



void entity_CalculateBufferSize(int *buffer_size, struct entity_handle_t entity, struct component_handle_t transform)
{
	struct entity_t *entity_ptr;
	struct physics_component_t *physics_component;
	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform_component;
	struct collider_def_t *collider_def;
	struct component_t *component;
	int size = 0;
	int i;
	int j;
	int k;
	static int depth_level = -1;

	if(!entity.def)
	{
		return;
	}

	entity_ptr = entity_GetEntityPointerHandle(entity);

//	transform_component = entity_GetComponentPointer(transform);
//
//    if(!transform_component)
//	{
//		entity_ptr = entity_GetEntityPointerHandle(entity);
//	}
//	else
//	{
//		entity_ptr = entity_GetEntityPointerHandle(transform_component->base.entity);
//	}

	if(entity_ptr)
	{
		depth_level++;

		if(!depth_level)
		{
			*buffer_size = 0;
		}

		size += sizeof(struct entity_record_start_t);

		entity_ptr->flags &= ~ENTITY_FLAG_SERIALIZED;

		for(i = 0; i < COMPONENT_TYPE_NONE; i++)
		{
			if(entity_ptr->components[i].type != COMPONENT_TYPE_NONE)
			{
				size += sizeof(struct component_record_t);
				component = entity_GetComponentPointer(entity_ptr->components[i]);

				component->flags &= ~COMPONENT_FLAG_SERIALIZED;
			}
		}

		size += sizeof(struct entity_prop_record_t) * entity_ptr->props.element_count;

		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

		if(physics_component)
		{
			size += sizeof(struct collider_record_start_t);
			size += sizeof(struct collision_shape_record_t) * physics_component->collider.collider_def->collision_shape_count;
			size += sizeof(struct collider_record_end_t);
		}


		/* transform contains the transform component that brought us
		here.

		If this is a ref, then transform is the transform component that
		points to the entity def it references.

		If this isn't a ref, then transform is equal to the transform component
		this entity def has. */


		transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);

		for(j = 0; j <= 1; j++)
		{
			for(i = 0; i < transform_component->children_count; i++)
			{
				child_transform_component = entity_GetComponentPointer(transform_component->child_transforms[i]);

				if(child_transform_component->base.entity.entity_index != INVALID_ENTITY_INDEX)
				{
					entity_CalculateBufferSize(buffer_size, child_transform_component->base.entity, transform_component->child_transforms[i]);
				}
				else
				{
					size += sizeof(struct component_record_t);
				}
			}

			transform_component = entity_GetComponentPointer(transform);

			if(!transform_component)
			{
				break;
			}

			size += sizeof(struct component_record_t);


			//j = (transform_component == NULL);
		}



		size += sizeof(struct entity_record_end_t);
		(*buffer_size) += size;
		depth_level--;
	}
}


void entity_SerializeEntityDef(void **buffer, int *buffer_size, struct entity_handle_t entity_def)
{
	int out_size;
	void *out_buffer;
	void *write_buffer;

	entity_CalculateBufferSize(&out_size, entity_def, INVALID_COMPONENT_HANDLE);
	out_buffer = memory_Calloc(out_size, 1);

	write_buffer = out_buffer;

	entity_WriteEntity(&write_buffer, entity_def, INVALID_COMPONENT_HANDLE, 0);

	*buffer = out_buffer;
	*buffer_size = out_size;

	printf("entity_SerializeEntityDef - alloc space: %d bytes --- used space: %d bytes\n", out_size, write_buffer - out_buffer);
}






void entity_DeserializeEntities(void **buffer, int deserialize_defs)
{
	char *in;
	struct entity_section_start_t *section_start;
	struct entity_section_end_t *section_end;
	struct entity_handle_t handle;


	struct trigger_record_t *trigger_record;
	int trigger_index;

	int i;

	in = *buffer;

	handle.entity_index = INVALID_ENTITY_INDEX;

	while(1)
	{
		if(!strcmp(in, entity_section_start_tag))
		{
			section_start = (struct entity_section_start_t *)in;
			in += sizeof(struct entity_section_start_t);
		}
		if(!strcmp(in, trigger_record_tag))
		{
            trigger_record = (struct trigger_record_t *)in;
			in += sizeof(struct trigger_record_t) + sizeof(struct trigger_filter_record_t) * (trigger_record->filter_count - 1);

			trigger_index = entity_CreateTrigger(&trigger_record->orientation, trigger_record->positon, trigger_record->scale, trigger_record->event_name, trigger_record->trigger_name);

			for(i = 0; i < trigger_record->filter_count; i++)
			{
				entity_AddTriggerFilter(trigger_index, trigger_record->filters[i].filter_name);
			}

			continue;

		}
		if(!strcmp(in, entity_record_start_tag))
		{
			entity_ReadEntity((void **)&in, handle);
			continue;
		}
		if(!strcmp(in, entity_section_end_tag))
		{
			section_end = (struct entity_section_end_t *)in;
			in += sizeof(struct entity_section_end_t);
			break;
		}
		else
		{
			in++;
		}
	}

	*buffer = in;
}



#ifdef __cplusplus
}
#endif


