#include "ent_serialization.h"
//#include "camera.h"
#include "entity.h"
#include "script.h"
#include "r_view.h"

#include "path.h"

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

void entity_WriteComponent(void **buffer, struct component_handle_t component_handle, int nestled, int ref, int dry_fire)
{
	struct component_record_t *component_record;
	struct transform_component_t *transform_component;
	struct transform_component_t *parent_transform_component;

	struct component_record_t dry_fire_component_record;

	struct camera_component_t *camera_component;
	struct model_component_t *model_component;
	struct script_component_t *script_component;
	struct physics_component_t *physics_component;

	struct collider_t *collider;
	struct collider_def_t *collider_def;
	struct model_t *model;

	struct entity_t *entity;

    struct component_t *component;

	int i;

    component = entity_GetComponentPointer(component_handle);

	//camera_t *camera;

	char *in;

	in = *buffer;

	if(component->type != COMPONENT_TYPE_NONE)
	{

//		if(component->flags & COMPONENT_FLAG_SERIALIZED)
//		{
//			return;
//		}
//
//		component->flags |= COMPONENT_FLAG_SERIALIZED;

		//component_record = (struct component_record_t *)in;
		//in += sizeof(struct component_record_t);
		//*buffer = in;

		//memset(component_record, 0, sizeof(struct component_record_t));

		//component_record = entity_SerializeComponent(buffer);

		component_record = (struct component_record_t *)in;
        in += sizeof(struct component_record_t );

        *buffer = in;

        if(dry_fire)
        {
            component_record = &dry_fire_component_record;
        }

        memset(component_record, 0, sizeof(struct component_record_t));

        strcpy(component_record->tag, component_record_tag);

        nestled = nestled && 1;

        component_record->type = component->type;

//        if(nestled)
//        {
//            component_record->flags |= COMPONENT_RECORD_FLAG_NESTLED;
//        }
//
//        if(ref)
//        {
//            component_record->flags |= COMPONENT_RECORD_FLAG_REF;
//        }

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

				collider_def = physics_GetColliderDefPointerHandle(physics_component->collider);

				strcpy(component_record->component.physics_component.collider_def_name, collider_def->name);

				if(component->entity.def)
				{
                    entity_WriteCollider(buffer, collider_def, dry_fire);
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

	struct collider_handle_t collider_def;

	struct entity_t *entity_ptr;
	struct entity_t *parent_entity_ptr;

	in = *buffer;

	component_record = entity_DeserializeComponent(buffer);


	entity_ptr = entity_GetEntityPointerHandle(entity);
	parent_entity_ptr = entity_GetEntityPointerHandle(parent_entity);

	if(!entity_ptr)
    {
        return;
    }

	//if(component_record->flags & COMPONENT_RECORD_FLAG_NESTLED)
	if(entity_record->flags & ENTITY_RECORD_FLAG_DEF_REF)
	{
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
			//if(component_record->flags & COMPONENT_RECORD_FLAG_REF)
			if(entity_record->flags & ENTITY_RECORD_FLAG_DEF_REF)
			{
				transform_component->orientation = component_record->component.transform_component.orientation;
				transform_component->position = component_record->component.transform_component.position;
				transform_component->scale = component_record->component.transform_component.scale;

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
				//camera_component->camera = camera_CreateCamera("camera", vec3_t_c(0.0, 0.0, 0.0), NULL, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
				camera_component->view = renderer_CreateViewDef("view", vec3_t_c(0.0, 0.0, 0.0), NULL, 0.68, 1366.0, 768.0, 0.1, 500.0, 0);
			}
		break;

		case COMPONENT_TYPE_PHYSICS:
			physics_component = (struct physics_component_t *)component;

			if(!entity.def)
			{
				collider_def = physics_GetColliderDefByName(component_record->component.physics_component.collider_def_name);
				transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
				physics_component->collider = physics_CreateCollider(&transform_component->orientation, transform_component->position, transform_component->scale, collider_def, 0);
				physics_component->flags = component_record->component.physics_component.flags;
			}

		break;

		case COMPONENT_TYPE_SCRIPT:
			script_component = (struct script_component_t *)component;

			script = script_GetScript(component_record->component.script_component.script_name);

			if(!script)
			{
				script = (struct script_t *)entity_LoadScript(component_record->component.script_component.script_file_name);

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



void entity_WriteProp(void **buffer, struct entity_prop_t *prop, int dry_fire)
{
	char *in;
	struct entity_prop_record_t *prop_record;
	void *value;
	in = *buffer;

	if(prop)
	{
		prop_record = (struct entity_prop_record_t *)in;
		in += sizeof(struct entity_prop_record_t);

		if(!dry_fire)
        {
            memset(prop_record, 0, sizeof(struct entity_prop_record_t));
            strcpy(prop_record->tag, entity_prop_record_tag);
            strcpy(prop_record->name, prop->name);

            prop_record->size = prop->size;

            /* the prop value gets written right after the record... */
            value = (char *)prop_record + sizeof(struct entity_prop_record_t);

            memcpy(value, prop->memory, prop->size);
        }

		in += prop->size;
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


void entity_WriteCollider(void **buffer, struct collider_def_t *collider_def, int dry_fire)
{
	struct physics_component_t *physics_component;
	struct collision_shape_t *collision_shapes;
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

		if(!dry_fire)
        {
            memset(collider_record_start, 0, sizeof(struct collider_record_start_t ));

            strcpy(collider_record_start->tag, collider_record_start_tag);
            collider_record_start->type = collider_def->collider_type;

            strcpy(collider_record_start->name, collider_def->name);
            collider_record_start->collider.collider_data.mass = collider_def->mass;
        }


		switch(collider_def->collider_type)
		{
			case COLLIDER_TYPE_CHARACTER_COLLIDER:
			    if(!dry_fire)
                {
                    collider_record_start->collider.collider_data.radius = collider_def->radius;
                    collider_record_start->collider.collider_data.height = collider_def->height;
                    collider_record_start->collider.collider_data.max_slope_angle = collider_def->max_slope_angle;
                    collider_record_start->collider.collider_data.max_step_height = collider_def->max_step_height;
                    collider_record_start->collider.collider_data.jump_height = 0.0;
                    collider_record_start->collider.collider_data.max_walk_speed = collider_def->max_walk_speed;
                    collider_record_start->collider.collider_data.mass = collider_def->mass;
                }
			break;

			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:

			    collision_shapes = (struct collision_shape_t *)collider_def->collision_shape.elements;

			    collision_shape_record = (struct collision_shape_record_t *)out;
                out += sizeof(struct collision_shape_record_t ) * collider_def->collision_shape.element_count;

                if(!dry_fire)
                {
                    for(i = 0; i < collider_def->collision_shape.element_count; i++)
                    {
                        memset(collision_shape_record, 0, sizeof(struct collision_shape_record_t ));
                        strcpy(collision_shape_record->tag, collision_shape_record_tag);

                        collision_shape_record->type = collision_shapes[i].type;
                        collision_shape_record->orientation = collision_shapes[i].orientation;
                        collision_shape_record->position = collision_shapes[i].position;
                        collision_shape_record->scale = collision_shapes[i].scale;

                        collision_shape_record++;
                    }
                }

			break;
		}

		collider_record_end = (struct collider_record_end_t *)out;
		out += sizeof(struct collider_record_end_t );

        if(!dry_fire)
        {
            memset(collider_record_end, 0, sizeof(struct collider_record_end_t ));
            strcpy(collider_record_end->tag, collider_record_end_tag);
        }

		*buffer = out;
	}
}

void entity_ReadCollider(void **buffer, struct entity_handle_t entity)
{
	struct collider_def_t *def;
	struct collider_handle_t def_handle;

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
			def_handle = physics_CreateCharacterColliderDef(collider_record_start->name, collider_record_start->collider.collider_data.height,
																			      collider_record_start->collider.collider_data.crouch_height,
																				  collider_record_start->collider.collider_data.radius,
																				  collider_record_start->collider.collider_data.max_step_height,
																				  collider_record_start->collider.collider_data.max_slope_angle,
																				  collider_record_start->collider.collider_data.max_walk_speed,
																				  collider_record_start->collider.collider_data.mass);

            def = physics_GetColliderDefPointerHandle(def_handle);
		break;

		case COLLIDER_TYPE_RIGID_BODY_COLLIDER:
			def_handle = physics_CreateRigidBodyColliderDef(collider_record_start->name);
			def = physics_GetColliderDefPointerHandle(def_handle);

			def->mass = collider_record_start->collider.collider_data.mass;
			while(1)
			{
				if(!strcmp(in, collision_shape_record_tag))
				{
					collision_shape_record = (struct collision_shape_record_t *)in;
					in += sizeof(struct collision_shape_record_t);

					physics_AddCollisionShape(def_handle, collision_shape_record->scale, collision_shape_record->position, &collision_shape_record->orientation, collision_shape_record->type);

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

	physics_component->collider = def_handle;


	collider_record_end = (struct collider_record_end_t *)in;
	in += sizeof(struct collider_record_end_t);

	*buffer = in;
}


void entity_ClearSerializedFlag(struct component_handle_t transform)
{
    struct entity_t *entity_ptr;
    struct transform_component_t *transform_component;
    struct component_t *component;
    struct transform_component_t *nestled_transform_component;

    int i;

    transform_component = entity_GetComponentPointer(transform);

    if(transform_component)
    {
        transform_component->flags &= ~COMPONENT_FLAG_SERIALIZED;

        entity_ptr = entity_GetEntityPointerHandle(transform_component->base.entity);

        if(entity_ptr)
        {
            entity_ptr->flags &= ~ENTITY_FLAG_SERIALIZED;

            for(i = 0; i < COMPONENT_TYPE_LAST; i++)
            {
                component = entity_GetComponentPointer(entity_ptr->components[i]);

                if(component)
                {
                    component->flags &= ~COMPONENT_FLAG_SERIALIZED;
                }
            }


            for(i = 0; i < transform_component->children_count; i++)
            {
                entity_ClearSerializedFlag(transform_component->child_transforms[i]);
            }
        }
    }
}


void entity_WriteEntity(void **buffer, struct entity_handle_t entity, struct component_handle_t referencing_transform, int def_name_only, int dry_fire)
{
	struct entity_record_start_t *ent_record_start;
	struct entity_record_end_t *ent_record_end;
	struct entity_name_record_t *ent_name_record;
	struct entity_source_file_t *ent_source_file;

	struct entity_record_start_t ent_dry_fire_record_start;

	struct entity_t *entity_def_ptr;
	struct entity_t *entity_ptr;

	struct component_t *component;

	struct transform_component_t *transform_component;
	struct transform_component_t *child_transform_component;

	struct stack_list_t *list;

	struct entity_prop_t *props;

	struct component_handle_t components[COMPONENT_TYPE_LAST] = {INVALID_COMPONENT_HANDLE};

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

	    if(!depth_level)
        {
            entity_ClearSerializedFlag(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
        }


		if(entity_ptr->flags & ENTITY_FLAG_SERIALIZED)
		{
		    /* this def was already serialized, which means that
		    if we're here again, this is just a reference to this
            entity def... */
			if(entity.def)
			{
				/* this is a reference to an entity def... */
				if(depth_level < 1)
				{
					/* if this ref isn't inside a hierarchy, don't serialize
					anything (an entity def has to be referenced FROM another entity)... */
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

		transform_component = entity_GetComponentPointer(referencing_transform);

		/* write record start... */
		ent_record_start = (struct entity_record_start_t *)out;
		out += sizeof(struct entity_record_start_t);


        if(!dry_fire)
        {
            record_start = out;
        }
        else
        {
            /* we want to precisely calculate how much space an entity
            will take. To avoid having similar code implemented in two
            different places, this function calculates
            the size of the entity in case dry_fire is true... */
            ent_record_start = &ent_dry_fire_record_start;
        }

        memset(ent_record_start, 0, sizeof(struct entity_record_start_t));
        strcpy(ent_record_start->tag, entity_record_start_tag);
        strcpy(ent_record_start->name, entity_ptr->name);

        ent_record_start->entity_flags = entity_ptr->flags;

        if(transform_component)
        {
            if(transform_component->flags & COMPONENT_FLAG_ENTITY_DEF_REF)
            {
                /* if the transform that brought us here is used to only
                reference an entity def, mark the record as belonging to a
                reference to an entity def... */
                ent_record_start->flags |= ENTITY_RECORD_FLAG_DEF_REF;
            }
        }

        entity_ptr->flags |= ENTITY_FLAG_SERIALIZED;



        if(!entity.def)
        {
            entity_def_ptr = entity_GetEntityPointerHandle(entity_ptr->def);
            strcpy(ent_record_start->def_name, entity_def_ptr->name);

            if(entity_ptr->flags & ENTITY_FLAG_MODIFIED)
            {
                ent_record_start->flags |= ENTITY_RECORD_FLAG_MODIFIED;
            }
        }
        else
        {
            ent_record_start->flags |= ENTITY_RECORD_FLAG_DEF;
        }



        if(entity.def && def_name_only)
        {
            ent_record_start->flags |= ENTITY_RECORD_FLAG_FILE_REF;
        }
        else
        {
            for(i = 0; i < COMPONENT_TYPE_LAST; i++)
            {
                components[i] = INVALID_COMPONENT_HANDLE;
            }

            props = (struct entity_prop_t *)entity_ptr->props.elements;

            /* Always write props, regardless of the entity having been modified or not... */
            for(i = 0; i < entity_ptr->props.element_count; i++)
            {
                entity_WriteProp((void **)&out, props + i, dry_fire);
            }

            if(entity.def || (entity_ptr->flags & ENTITY_FLAG_MODIFIED))
            {

                if(ent_record_start->flags & ENTITY_RECORD_FLAG_DEF_REF)
                {
                    /*

                        entity def refs are nothing more than a transform component
                        that references an entity def but it isn't stored in said entity's
                        component list.

                        An entity def ref can have other entities nestled to it without
                        affecting the original entity def. This is possible because the
                        transform component that defines the reference to the entity def
                        has its own child transforms list.

                        If we're serializing a entity def ref here, first we'll serialize
                        the entity the transform references, which has its own transforms,
                        with potentially several nestled transforms. This consists in
                        creating a new entity record inside this entity record.

                        After that, we serialize the transform component that represents
                        this def ref.

                    */

                    transform_component = entity_GetComponentPointer(referencing_transform);

                    /* write the entity this transform component (def ref) references. This
                    will create a new entity record inside the current one... */
                    entity_WriteEntity((void **)&out, transform_component->base.entity, INVALID_COMPONENT_HANDLE, def_name_only, dry_fire);

                    components[COMPONENT_TYPE_TRANSFORM] = referencing_transform;
                }
                else
                {
                    /* If this is a def or an entity that got modified after being spawn,
                    write its stuff down... */

                    /* write components... */
                    for(i = 0; i < COMPONENT_TYPE_LAST; i++)
                    {
                        components[i] = entity_ptr->components[i];
                    }

                    transform_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
                }


                for(i = 0; i < COMPONENT_TYPE_LAST; i++)
                {
                    if(components[i].type != COMPONENT_TYPE_NONE)
                    {
                        //component = entity_GetComponentPointer(components[i]);
                        entity_WriteComponent((void **)&out, components[i], 0, 0, dry_fire);
                    }
                }


                for(i = 0; i < transform_component->children_count; i++)
                {
                    component = entity_GetComponentPointer(transform_component->child_transforms[i]);

                    if(component->entity.entity_index != INVALID_ENTITY_INDEX)
                    {
                       /* this child transform component points to an entity,
                       so write it to the buffer before continuing with the
                       current entity... */
                       entity_WriteEntity((void **)&out, component->entity, transform_component->child_transforms[i], def_name_only, dry_fire);
                    }
                }
            }
            else if(depth_level == 0)
            {
                /* if this is the root of the hierarchy of an unmodified entity, write
                only it's transform... */
                //component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_TRANSFORM]);
                entity_WriteComponent((void **)&out, entity_ptr->components[COMPONENT_TYPE_TRANSFORM], 0, 0, dry_fire);
            }

            ent_record_start->data_skip_offset = out - record_start;
        }






		/* write record end... */
		ent_record_end = (struct entity_record_end_t *)out;
		out += sizeof(struct entity_record_end_t );

		if(!dry_fire)
        {
            memset(ent_record_end, 0, sizeof(struct entity_record_end_t));
            strcpy(ent_record_end->tag, entity_record_end_tag);
        }

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
	char *entity_name;
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

	    /* if we got here, it means this record belongs to a reference to an
	    entity def...  */

	    if(entity)
        {

            /* if the entity def being referenced already exists we
            skip all the def data...*/
            in += ent_record_start->data_skip_offset;


            /* and read only a transform component, which contains data
            that is unique to this reference... */
            handle = INVALID_ENTITY_HANDLE;
        }
        else
        {
            /* .ent files are supposed to be self-contained, which means that even though this record
            is a reference to an entity def, it contains all the data necessary to create the def it
            references. If we got here, it means we're referencing an entity def that still doesn't
            exist. So, we create it here and read all its data from this record... */
			//handle = entity_CreateEntity(ent_record_start->name, ent_record_start->flags & ENTITY_RECORD_FLAG_DEF);

			handle = entity_ReadEntity((void **)&in, INVALID_ENTITY_HANDLE);
        }
	}
	else
	{
		if((ent_record_start->flags & ENTITY_RECORD_FLAG_DEF) ||
           (ent_record_start->flags & ENTITY_RECORD_FLAG_MODIFIED))
		{

		    handle = entity_GetEntityHandle(ent_record_start->name, (ent_record_start->flags & ENTITY_RECORD_FLAG_FILE_REF));

			if(!(ent_record_start->flags & ENTITY_RECORD_FLAG_FILE_REF))
			{
				/* This record belongs to to a post-spawn modified entity,
                so we create a new entity here... */
				if(handle.entity_index == INVALID_ENTITY_INDEX)
				{
					handle = entity_CreateEntity(ent_record_start->name, ent_record_start->flags & ENTITY_RECORD_FLAG_DEF);
				}
				else
				{
					in += ent_record_start->data_skip_offset;
				}

			}
			else
			{
			    /* This record contains only the name of an entity def. We
			    first check whether it exists. If it does, than we're done.
			    If it doesn't, then try to load it from the disk... */
				if(handle.entity_index == INVALID_ENTITY_INDEX)
                {
                    entity_name = path_AddExtToName(ent_record_start->name, ".ent");
                    handle = entity_LoadEntityDef(entity_name);

                    if(handle.entity_index == INVALID_ENTITY_INDEX)
                    {
                        in += ent_record_start->data_skip_offset;
                    }
                }
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

				if(entity_def.entity_index == INVALID_ENTITY_INDEX)
                {
                    in += ent_record_start->data_skip_offset;
                }
                else
                {
                    handle = entity_SpawnEntity(NULL, vec3_t_c(0.0, 0.0, 0.0), vec3_t_c(1.0, 1.0, 1.0), entity_def, ent_record_start->name);
                }
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
//			else if(!strcmp(in, entity_name_record_tag))
//			{
//				ent_file_record = (struct entity_name_record_tag *)in;
//				in += sizeof(struct entity_name_record_tag);
//				handle = entity_LoadEntityDef(ent_file_record->file_name);
//			}
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

//	entity = entity_GetEntityPointerHandle(handle);
//    parent_entity = entity_GetEntityPointerHandle(parent);
//
//	if(parent_entity)
//	{
//		if((ent_record_start->flags & ENTITY_RECORD_FLAG_DEF) && (!(ent_record_start->flags & ENTITY_RECORD_FLAG_DEF_REF)))
//		{
//		    /* if this is an entity def ref, there's no need
//		    to parent it to it's parent entity def since it
//		    was already parented when it's parent read it's
//		    nestled transforms... */
//			entity_ParentEntity(parent, handle);
//
//			/* this def was loaded from a buffer that's likely come
//			from a file, so mark the entity as existing in a file... */
//			//entity->flags |= ENTITY_FLAG_ON_DISK;
//
//		}
//	}



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
	/*for(i = 0; i < 2; i++)
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
	}*/



	serialize_defs = serialize_defs && 1;

	//for(write_entity_def = 0; write_entity_def <= serialize_defs; write_entity_def++)
	//{

    list = &ent_entities[serialize_defs];
    j = list->element_count;

    handle.def = serialize_defs;

    for(i = 0; i < j; i++)
    {
        handle.entity_index = i;

        entity = entity_GetEntityPointerHandle(handle);

        if(!entity)
        {
            continue;
        }

        transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

        if(transform_component->parent.type == COMPONENT_TYPE_NONE)
        {
            entity_WriteEntity((void **)&out_size, handle, INVALID_COMPONENT_HANDLE, 1, 1);
        }
    }
	//}

	if(!serialize_defs)
    {
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
    }

	out = memory_Calloc(2, out_size);

	*buffer = out;
	*buffer_size = out_size;
	section_start = (struct entity_section_start_t *)out;
	out += sizeof(struct entity_section_start_t);

//	memset(header, 0, sizeof(struct entity_section_header_t));
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




	//for(k = serialize_defs; k >= 0; k--)
	//{

    list = &ent_entities[serialize_defs];
    j = list->element_count;
    handle.def = serialize_defs;

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
                    entity_WriteEntity((void **)&out, handle, INVALID_COMPONENT_HANDLE, 1, 0);
                }
            }
        }
    }
	//}

	section_end = (struct entity_section_end_t *)out;
	out += sizeof(struct entity_section_end_t );

	//memset(tail, 0, sizeof(struct entity_section_tail_t));
	strcpy(section_end->tag, entity_section_end_tag);



	printf("entity_SerializeEntities - allocd space: %d bytes --- used space: %d bytes\n", out_size, out - (char *)*buffer);
}



void entity_CalculateBufferSize(int *buffer_size, struct entity_handle_t entity, struct component_handle_t from_transform)
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
			}
		}

		size += sizeof(struct entity_prop_record_t) * entity_ptr->props.element_count;

		physics_component = entity_GetComponentPointer(entity_ptr->components[COMPONENT_TYPE_PHYSICS]);

		if(physics_component)
		{
		    collider_def = physics_GetColliderDefPointerHandle(physics_component->collider);

			size += sizeof(struct collider_record_start_t);
			size += sizeof(struct collision_shape_record_t) * collider_def->collision_shape.element_count;
			size += sizeof(struct collider_record_end_t);
		}


		/* from_transform contains the transform component that brought us
		here.

		If this is a ref, then from_transform is the transform component that
		points to the entity def it references.

		If this isn't a ref, then from_transform is equal to the from_transform component
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
				    /* this transform is used by some other thing
				    that needs to keep transformation data (lights,
                    particle systems, sound sources, etc)... */
					size += sizeof(struct component_record_t);
				}
			}

			transform_component = entity_GetComponentPointer(from_transform);

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

void entity_SerializeEntity(void **buffer, int *buffer_size, struct entity_handle_t entity)
{

}

void entity_SerializeEntityDef(void **buffer, int *buffer_size, struct entity_handle_t entity_def)
{
	int out_size = 0;
	void *out_buffer;
	void *write_buffer;

	struct entity_t *def;

	def = entity_GetEntityPointerHandle(entity_def);

	if(!def)
    {
        return;
    }

	//entity_CalculateBufferSize(&out_size, entity_def, INVALID_COMPONENT_HANDLE);
	entity_WriteEntity((void **)&out_size, entity_def, INVALID_COMPONENT_HANDLE, 0, 1);
	out_buffer = memory_Calloc(out_size, 1);

	write_buffer = out_buffer;

	entity_WriteEntity(&write_buffer, entity_def, INVALID_COMPONENT_HANDLE, 0, 0);

	*buffer = out_buffer;
	*buffer_size = out_size;

	printf("entity_SerializeEntityDef - alloc space: %d bytes --- used space: %d bytes\n", out_size, write_buffer - out_buffer);
}

void entity_SerializeEntityDefName(void **buffer, int *buffer_size, struct entity_handle_t entity_def)
{
    int out_size = 0;
    void *out_buffer;
    struct entity_t *entity_def_ptr;
    struct entity_name_record_t *record;


    if(entity_def.def)
    {
        entity_def_ptr = entity_GetEntityPointerHandle(entity_def);
        out_size = sizeof(struct entity_name_record_t);
        out_buffer = memory_Calloc(out_size, 1);
        record = (struct entity_name_record_t *)out_buffer;

        strcpy(record->tag, entity_name_record_tag);
        strcpy(record->entity_name, entity_def_ptr->name);

        *buffer = out_buffer;
        *buffer_size = out_size;
    }
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

	if(in)
    {
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
}



#ifdef __cplusplus
}
#endif


