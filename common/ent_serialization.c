#include "ent_serialization.h"
#include "camera.h"
#include "entity.h"
#include "script.h"

#include "stack_list.h"

#include "memory.h"


struct stack_list_t ent_entities[2];

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

void entity_WriteComponent(void **buffer, struct component_t *component, int nestled)
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
		component_record->nestled = nestled;
		
		switch(component->type)
		{
			case COMPONENT_TYPE_TRANSFORM:
				transform_component = (struct transform_component_t *)component;
				
				component_record->component.transform_component.orientation = transform_component->orientation;
				component_record->component.transform_component.position = transform_component->position;
				component_record->component.transform_component.scale = transform_component->scale;
			break;
			
			case COMPONENT_TYPE_CAMERA:
				camera_component = (struct camera_component_t *)component;
				
				//strcpy(component_record->component.camera_component.camera_name, camera_component->camera->name);
				entity = entity_GetEntityPointerHandle(camera_component->base.entity);
				transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
				/* write the transform linked to this camera component, flagging it as nestled so it
				won't get added to an entity when being read back... */
				entity_WriteComponent(buffer, (struct component_t *)transform_component, 1);		
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
			break;
			
			case COMPONENT_TYPE_SCRIPT:
				script_component = (struct script_component_t *)component;
				strcpy(component_record->component.script_component.script_name, script_component->script->name);
			break;
			
			case COMPONENT_TYPE_MODEL:
				model_component = (struct model_component_t *)component;
				model = model_GetModelPointerIndex(model_component->model_index);
				strcpy(component_record->component.model_component.model_name, model->name);
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

void entity_ReadComponent(void **buffer, struct entity_handle_t entity, struct entity_record_start_t *entity_record)
{
	char *in;
	struct component_record_t *component_record;
	struct component_record_t *camera_transform_component_record;
	
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
	
	if(component_record->nestled)
	{
		/* this component record belongs to a child transform component of an entity's transform component. 
		Parent it to the entity's transform component instead of adding to the entity itself... */
		handle = entity_AllocComponent(component_record->type, entity.def);
		entity_ParentTransformComponent(entity_ptr->components[COMPONENT_TYPE_TRANSFORM], handle);
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
			/*transform_component = (struct transform_component_t *)component;	
			transform_component->orientation = component_record->component.transform_component.orientation;
			transform_component->position = component_record->component.transform_component.position;
			transform_component->scale = component_record->component.transform_component.scale;*/
			
			
			
			if(entity_record->flags & ENTITY_RECORD_FLAG_DEF_REF)
			{
				transform_component->orientation = component_record->component.transform_component.orientation;
				transform_component->position = component_record->component.transform_component.position;
				transform_component->scale = component_record->component.transform_component.scale;
				
				
				/* if this is a reference to a def we need to make sure this
				newly allocated transform component points to the original
				def... */
				parent_entity_ptr = entity_GetEntityPointerHandle(entity);	/* if we got here, entity will point to the parent entity of this ref... */
				transform_component = entity_GetComponentPointer(parent_entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				component = entity_GetComponentPointer(transform_component->child_transforms[transform_component->children_count - 1]);
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
			}
			
		break;
		
		case COMPONENT_TYPE_SCRIPT:
			script_component = (struct script_component_t *)component;
			
			script_component->script = script_GetScript(component_record->component.script_component.script_name);
			
		break;
		
		case COMPONENT_TYPE_MODEL:
			model_component = (struct model_component_t *)component;
			model_component->model_index = model_GetModel(component_record->component.model_component.model_name);
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
		
		switch(collider_def->type)
		{
			case COLLIDER_TYPE_CHARACTER_COLLIDER:
				collider_record_start->collider.collider_data.radius = collider_def->radius;
				collider_record_start->collider.collider_data.height = collider_def->height;
				collider_record_start->collider.collider_data.max_slope_angle = collider_def->slope_angle;
				collider_record_start->collider.collider_data.max_step_height = collider_def->step_height;
				collider_record_start->collider.collider_data.jump_height = 0.0;
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

void entity_ReadCollider(void **buffer)
{
	struct collider_def_t *def;
	
	struct collider_record_start_t *collider_record_start;
	struct collider_record_end_t *collider_record_end;
	struct collision_shape_record_t *collision_shape_record;
	
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
																				  collider_record_start->collider.collider_data.max_slope_angle);
		break;
		
		case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
			def = physics_CreateRigidBodyColliderDef(collider_record_start->name);
			
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
	
	collider_record_end = (struct collider_record_end_t *)in;
	in += sizeof(struct collider_record_end_t);
	
	*buffer = in;
}



void entity_WriteEntity(void **buffer, struct entity_handle_t entity, struct component_t *referencing_transform)
{	
	struct entity_record_start_t *ent_record_start;
	struct entity_record_end_t *ent_record_end;
	
	struct entity_t *entity_def_ptr;
	struct entity_t *entity_ptr;
	
	struct component_t *component;
	
	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform_component;
	
	struct stack_list_t *list;
	
	int i;
	
	char *out;
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
				write_entity_data = 0;
			}
			else
			{
				/* this is an entity instance, and it was already
				serialized, so don't do it again... */
				depth_level--;
				return;
			}
			
		}
		
		entity_ptr->flags |= ENTITY_FLAG_SERIALIZED;
				
		/* write record start... */				
		ent_record_start = (struct entity_record_start_t *)out;
		out += sizeof(struct entity_record_start_t);
		
		memset(ent_record_start, 0, sizeof(struct entity_record_start_t));
		strcpy(ent_record_start->tag, entity_record_start_tag);
		

		strcpy(ent_record_start->name, entity_ptr->name);
		
		if(!entity.def)
		{
			entity_def_ptr = entity_GetEntityPointerHandle(entity_ptr->def);
			strcpy(ent_record_start->def_name, entity_def_ptr->name);
		}
		else
		{
			ent_record_start->flags |= ENTITY_RECORD_FLAG_DEF;
		}
		
		if(!write_entity_data)
		{
			ent_record_start->flags |= ENTITY_RECORD_FLAG_DEF_REF;
		}
		
		if(entity_ptr->flags & ENTITY_FLAG_MODIFIED)
		{
			ent_record_start->flags |= ENTITY_RECORD_FLAG_MODIFIED;
		}
				
		if(write_entity_data)
		{
			/* Always write props, regardless of the entity having been modified... */
			for(i = 0; i < entity_ptr->prop_count; i++)
			{
				entity_WriteProp((void **)&out, entity_ptr->props + i);
			}
			
			if(entity.def || (entity_ptr->flags & ENTITY_FLAG_MODIFIED))
			{
				/* If this is a def or it is a post-spawn modified entity,
				write it stuff down... */
				
				/* write components... */
				for(i = 0; i < COMPONENT_TYPE_LAST; i++)
				{
					if(entity_ptr->components[i].type != COMPONENT_TYPE_NONE)
					{
						component = entity_GetComponentPointer(entity_ptr->components[i]);
						entity_WriteComponent((void **)&out, component, 0);	
					}
				}
			}
			else if(depth_level == 0)
			{
				/* if this is the root of the hierachy of an unmodified entity, write it's 
				only it's transform... */
				component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				entity_WriteComponent((void **)&out, component, 0);
			}
			
			/* write nestled transforms... */					
			transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				
			for(i = 0; i < transform_component->children_count; i++)
			{
				component = entity_GetComponentPointer(transform_component->child_transforms[i]);
						
				if(component->entity.entity_index != INVALID_ENTITY_INDEX)
				{
					/* this child transform component points to an entity,
					so write it to the buffer before continuing with the 
					current entity... */
					entity_WriteEntity((void **)&out, component->entity, component);
				}
				else if(write_entity_data && (entity_ptr->flags & ENTITY_FLAG_MODIFIED))
				{
					/* This nestled transform belongs to some component
					that needs to keep spacial information (light, camera,
					particle system, etc).
					
					This just gets written if this isn't a reference to
					an entity def or it is a post-spawn modified entity... */
					entity_WriteComponent((void **)&out, component, 1);
				}	
			}
			
		}
		else
		{
			/* This def was already serialized, which means this is just a reference to it, and as so 
			we'll just need to serialize this reference's transform component. We flag it as nestled here
			so it can be added to the parent entity's transform component child list upon deserialization... */
			entity_WriteComponent((void **)out, (struct component_t *)referencing_transform, 1);
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

void entity_ReadEntity(void **buffer, struct entity_handle_t parent)
{
	struct entity_record_start_t *ent_record_start;
	struct entity_record_start_t *nestled_ent_record_start;
	struct entity_record_end_t *ent_record_end;
	
	struct entity_handle_t handle;
	struct entity_handle_t entity_def;
	struct entity_t *entity;
	struct entity_t *parent_entity;
	
	struct component_t *component;
	struct transform_component_t *parent_transform_component;
	
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
		assert(ent_record_start->flags & ENTITY_RECORD_FLAG_DEF);
		
		/* This record belongs to a reference to an entity def, which means that we'll be reading only a transform component
		that will point to the orignal entity def. This transform won't be added to the original def, but instead will be to the
		child list of the parent entity of this reference. In order to do that, we need the handle to the parent entity, so we 
		can access it's transform component... */
		handle = parent;
	}
	else
	{
		if((ent_record_start->flags & ENTITY_RECORD_FLAG_DEF) || (ent_record_start->flags & ENTITY_RECORD_FLAG_MODIFIED))
		{
			/* This record belongs to an entity def or to post-spawned modified entity, so we create a new entity here... */
			handle = entity_CreateEntity(ent_record_start->name, ent_record_start->flags & ENTITY_RECORD_FLAG_DEF);
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
				/* if we're deeper in the hierarchy, this entity was already spawned, so we just get a handle to it... */
				//handle = entity_GetEntityHandle(ent_record_start->name, ent_record_start->flags & ENTITY_RECORD_FLAG_DEF);
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
				goto _gtfo;
			}
			else if(!strcmp(in, entity_record_start_tag))
			{
				entity_ReadEntity((void **)&in, handle);
			}
			else if(!strcmp(in, component_record_tag))
			{
				entity_ReadComponent((void **)&in, handle, ent_record_start);
			}
			else if(!strcmp(in, collider_record_start_tag))
			{
				entity_ReadCollider((void **)&in);
			}
			else if(!strcmp(in, entity_prop_record_tag))
			{
				entity_ReadProp((void **)&in, handle, ent_record_start);
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
	
	_gtfo:
		
	ent_record_end = (struct entity_record_end_t *)in;
	in += sizeof(struct entity_record_end_t);
	
	if(parent.entity_index != INVALID_ENTITY_INDEX)
	{
		/* In case this is a def, only parent
		it if it isn't a reference. 
		
		If this is a reference, it already got
		parented when it's transform component
		got read and added to the child list
		of its parent transform component... */
		if(!ent_record_start->flags & ENTITY_RECORD_FLAG_DEF_REF)
		{
			entity_ParentEntity(parent, handle);
		}
		
	}

	*buffer = in;
	
	depth_level--;
}


/*
*******************************************
*******************************************
*******************************************
*/



void entity_WriteEntityDef(void **buffer, int *buffer_size, struct entity_handle_t entity_def)
{
	char *out = NULL;
	int out_size = 0;
	
	*buffer = out;
	*buffer_size = out_size;	
}




void entity_SerializeEntities(void **buffer, int *buffer_size, int serialize_defs)
{
	struct entity_section_header_t *header;
	struct entity_section_tail_t *tail;
	struct entity_t *entity;
	struct entity_handle_t handle;
	struct component_t *component;
	
	struct transform_component_t *transform_component;

	struct stack_list_t *list;
	
	int entity_def_count = 0;
	int entity_count = 0;
	
	int write_entity_def;
	
	int i;	
	int j;
	int k;
	int l;
	char *out;
	int out_size = 0;
	
	out_size = sizeof(struct entity_section_header_t) + sizeof(struct entity_section_tail_t);
	
	
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
				
				/* props... */	
				out_size += sizeof(struct entity_prop_record_t) * entity->prop_count;
				
				
				/* nestled transforms... */
				transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
				out_size += sizeof(struct component_record_t) * transform_component->children_count;				
			}
		}
	}
		
	out = memory_Calloc(1, out_size, "entity_SerializeEntities");
		
	*buffer = out;
	*buffer_size = out_size;
	header = (struct entity_section_header_t *)out;
	out += sizeof(struct entity_section_header_t);
			
	memset(header, 0, sizeof(struct entity_section_header_t));
	strcpy(header->tag, entity_section_header_tag);
		
	header->entity_count = entity_count;
	header->entity_def_count = entity_def_count;
			
	header->reserved0 = 0;
	header->reserved1 = 0;
	header->reserved2 = 0;
	header->reserved3 = 0;
	header->reserved4 = 0;
	header->reserved5 = 0;
	header->reserved6 = 0;
	header->reserved7 = 0;
	
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
					if(transform_component->parent.type == COMPONENT_TYPE_NONE)
					{
						entity_WriteEntity((void **)&out, handle, NULL);
					}
				}
			}
		}
	}
	 	
	tail = (struct entity_section_tail_t *)out;
	out += sizeof(struct entity_section_tail_t );
	
	memset(tail, 0, sizeof(struct entity_section_tail_t));
	strcpy(tail->tag, entity_section_tail_tag);
}

void entity_DeserializeEntities(void **buffer, int deserialize_defs)
{
	char *in;
	struct entity_section_header_t *header;
	struct entity_section_tail_t *tail;
	struct entity_handle_t handle;
	
	int i;
	 
	in = *buffer;
	
	header = (struct entity_section_header_t *)in;
	in += sizeof(struct entity_section_header_t);
	
	handle.entity_index = INVALID_ENTITY_INDEX;
	
	while(1)
	{
		if(!strcmp(in, entity_record_start_tag))
		{
			entity_ReadEntity((void **)&in, handle);
			continue;
		}
		if(!strcmp(in, entity_section_tail_tag))
		{
			goto _gtfo;
		}
		else
		{
			in++;
		}
	}
	
	_gtfo:
	
	tail = (struct entity_section_tail_t *)in;
	in += sizeof(struct entity_section_tail_t);
	
	*buffer = in;	
}

#ifdef __cplusplus
}
#endif


