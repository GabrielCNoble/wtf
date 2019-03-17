#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "GL\glew.h"

#include "gpu.h"
#include "physics.h"
#include "material.h"
#include "shader.h"
#include "entity.h"
#include "ent_serialization.h"

#include "camera.h"

#include "world.h"
#include "script.h"
#include "w_script.h"
#include "model.h"
#include "texture.h"

#include "l_main.h"
#include "l_cache.h"

#include "stack_list.h"

#include "bsp_common.h"
#include "bsp.h"
//#include "bsp_file.h"

#include "engine.h"
#include "c_memory.h"

#include "portal.h"

#include "r_debug.h"


//bsp_node_t *world_bsp = NULL;
//int world_leaves_count = 0;
//bsp_sleaf_t *world_leaves = NULL;
int w_world_vertices_count = 0;
vertex_t *w_world_vertices = NULL;

int w_world_triangle_count = 0;

int w_world_nodes_count = 0;
struct bsp_pnode_t *w_world_nodes = NULL;

int w_world_leaves_count = 0;
struct bsp_dleaf_t *w_world_leaves = NULL;
bsp_lights_t *w_leaf_lights = NULL;

//int w_max_leaf_lights = 0;
//int w_
//int *w_free_leaf_lights = NULL;
//bsp_indexes_t *w_leaf_lights = NULL;

/* Maybe a simple per-leaf index list would be a better idea.
1024 entities in the world seems too little... */
bsp_entities_t *w_leaf_entities = NULL;
//bsp_batch_t *leaf_batches = NULL;
//int *leaf_batches_indexes = NULL;

//int world_hull_node_count = 0;
//bsp_pnode_t *world_hull = NULL;


struct world_level_t *w_levels = NULL;
struct world_level_t *w_last_level = NULL;
struct world_level_t *w_current_level = NULL;

int w_visible_leaves_count = 0;
struct bsp_dleaf_t **w_visible_leaves = NULL;

//int w_visible_lights_count = 0;
//unsigned short w_visible_lights[MAX_WORLD_LIGHTS];


struct world_script_t *w_world_script = NULL;
int w_execute_on_map_enter = 0;
int w_execute_on_map_load = 0;

struct list_t w_visible_entities;


struct gpu_light_t *w_light_buffer = NULL;
struct gpu_bsp_node_t *w_bsp_buffer = NULL;

struct gpu_alloc_handle_t w_world_handle = INVALID_GPU_ALLOC_HANDLE;
int w_world_start = -1;
int w_world_count = -1;

struct gpu_alloc_handle_t w_world_index_handle = INVALID_GPU_ALLOC_HANDLE;
int w_world_index_start = -1;
int w_world_index_count = -1;
unsigned int *w_index_buffer = NULL;

struct gpu_alloc_handle_t w_world_sorted_index_handle = INVALID_GPU_ALLOC_HANDLE;
int w_world_sorted_index_start = -1;
int w_world_sorted_index_count = -1;
unsigned int *w_sorted_index_buffer = NULL;

unsigned int w_max_visible_indexes = 0;
unsigned int w_max_visible_batches = 0;
unsigned int w_max_portal_recursion_level = 3;

int w_need_to_clear_world = 0;

int w_world_batch_count = 0;
struct batch_t *w_world_batches = NULL;

int w_world_z_batch_count = 0;
struct batch_t *w_world_z_batches = NULL;

struct list_t w_world_vars;


//unsigned int world_element_buffer = 0;
extern int forward_pass_shader;


/* from entity.c */
extern int ent_entity_list_cursor;
extern struct stack_list_t ent_entities[2];
extern struct stack_list_t ent_components[2][COMPONENT_TYPE_LAST];
extern struct stack_list_t ent_entity_aabbs;
extern struct stack_list_t ent_world_transforms;


//#include "l_globals.h"

/* from l_main.c */
extern struct stack_list_t l_light_positions;
extern struct stack_list_t l_light_params;
extern struct stack_list_t l_light_clusters;

//extern int l_light_count;
//extern int l_light_list_cursor;
//extern light_params_t *l_light_params;
//extern light_position_t *l_light_positions;
//extern unsigned int l_light_cache_uniform_buffer;
//extern unsigned int l_cluster_texture;
//extern cluster_t *l_clusters;

/* from camera.c */
//extern camera_t *cameras;


/* from portal.c */
extern int ptl_portal_list_cursor;
extern portal_t *ptl_portals;


/* from r_main.c */
extern int r_frame;
extern float r_fade_value;
extern int r_clusters_per_row;
extern int r_cluster_rows;
extern int r_cluster_layers;
extern struct cluster_t *r_clusters;
extern unsigned int r_cluster_texture;
extern unsigned int r_light_uniform_buffer;
extern unsigned int r_bsp_uniform_buffer;
extern unsigned int r_world_vertices_uniform_buffer;
extern vec4_t *r_world_vertices_buffer;


#ifdef __cplusplus
extern "C"
{
#endif


void *world_SetupScriptDataCallback(struct script_t *script, void *data)
{
    struct world_script_t *world_script;
	int i;

    world_script = (struct world_script_t *)script;

    if(w_execute_on_map_load)
    {
        script_QueueEntryPoint(world_script->on_map_load);
        w_execute_on_map_load = 0;
    }

    if(w_execute_on_map_enter)
    {
        script_QueueEntryPoint(world_script->on_map_enter);
        w_execute_on_map_enter = 0;
    }

	for(i = 0; i < world_script->event_count; i++)
	{
        if(world_script->events[i].executing)
		{
			script_QueueEntryPoint(world_script->events[i].event_function);
		}
	}

    return NULL;
}



char on_map_enter_function_name[] = "OnMapEnter";
char on_map_exit_function_name[] = "OnMapExit";
char on_map_load_function_name[] = "OnMapLoad";

int world_GetScriptDataCallback(struct script_t *script)
{
    struct world_script_t *world_script;

    world_script = (struct world_script_t *)script;

    void *function;
    char *function_name;
    char *function_name_postfix;

    int function_count;
    int i;


    char event_postfix[] = "_event";

	function_count = script_GetFunctionCount(script);

	world_script->on_map_exit = script_GetFunctionAddress(on_map_exit_function_name, script);
    world_script->on_map_enter = script_GetFunctionAddress(on_map_enter_function_name, script);
    world_script->on_map_load = script_GetFunctionAddress(on_map_load_function_name, script);
    world_script->current_event = -1;

	if(function_count)
	{
		world_script->events = memory_Calloc(function_count, sizeof(struct world_event_t));
		world_script->event_count = 0;

        for(i = 0; i < function_count; i++)
		{
			function = script_GetFunctionAddressByIndex(i, script);

			if(function != world_script->on_map_enter && function != world_script->on_map_exit)
			{
				function_name = script_GetFunctionName(function);

				function_name_postfix = strstr(function_name, "_event");

				if(function_name_postfix)
				{
					/* if this function has a _event substring,
					check to see whether it's truly the end of the string... */

					if(strlen(function_name_postfix) + 1 == sizeof(event_postfix))
					{
						world_script->events[world_script->event_count].event_function = function;

						/* keep the name without the postfix... */
						world_script->events[world_script->event_count].event_name = memory_Calloc(function_name_postfix - function_name + 1, 1);
						memcpy(world_script->events[world_script->event_count].event_name, function_name, function_name_postfix - function_name);
						world_script->events[world_script->event_count].executing = 0;
						world_script->event_count++;
					}
				}
			}
		}
	}

	return 1;
}





int world_Init()
{

    FILE *maps;
    unsigned long maps_file_buffer_size;
    char *maps_file_buffer;

    struct world_level_t *map;

    int i;
    int j;
	//w_light_buffer = memory_Malloc(sizeof(struct gpu_light_t) * MAX_VISIBLE_LIGHTS);
	//w_bsp_buffer = memory_Malloc(sizeof(struct gpu_bsp_node_t) * W_MAX_BSP_NODES);
	//w_visible_entities = list_create(sizeof(struct entity_handle_t), 128, NULL);

	w_world_vars = list_create(sizeof(struct world_var_t), 128, NULL);


	script_RegisterScriptType("world_script", "was", sizeof(struct world_script_t), world_GetScriptDataCallback, NULL, world_SetupScriptDataCallback);


	script_RegisterGlobalFunction("void world_AddWorldVar(string &in name, ? &in type)", world_ScriptAddWorldVar);
	script_RegisterGlobalFunction("void world_AddWorldArrayVar(string &in name, int max_elements, ? &in type)", world_ScriptAddWorldArrayVar);
    script_RegisterGlobalFunction("void world_RemoveWorldVar(string &in name)", world_ScriptRemoveWorldVar);
    script_RegisterGlobalFunction("int world_GetWorldArrayVarLength(string &in name)", world_ScriptGetWorldArrayVarLength);

    script_RegisterGlobalFunction("void world_SetWorldVarValue(string &in name, ? &in value)", world_ScriptSetWorldVarValue);
    script_RegisterGlobalFunction("void world_GetWorldVarValue(string &in name, ? &out value)", world_ScriptGetWorldVarValue);
    script_RegisterGlobalFunction("void world_SetWorldArrayVarValue(string &in name, int index, ? &in value)", world_ScriptSetWorldArrayVarValue);
    script_RegisterGlobalFunction("void world_GetWorldArrayVarValue(string &in name, int index, ? &out value)", world_ScriptGetWorldArrayVarValue);
	script_RegisterGlobalFunction("void world_AppendWorldArrayVarValue(string &in name, ? &in value)", world_ScriptAppendWorldArrayVarValue);
	script_RegisterGlobalFunction("void world_ClearWorldArrayVar(string &in name)", world_ScriptClearWorldArrayVar);
	//scr_virtual_machine->RegisterGlobalFunction("array<entity_handle_t> @world_GetEntities()", asFUNCTION(world_ScriptGetEntities), asCALL_CDECL);
	script_RegisterGlobalFunction("void world_CallEvent(string &in event_name)", world_ScriptCallEvent);
	script_RegisterGlobalFunction("void world_StopCurrentEvent()", world_ScriptStopCurrentEvent);
	script_RegisterGlobalFunction("void world_StopAllEvents()", world_ScriptStopAllEvents);
	script_RegisterGlobalFunction("void world_ClearWorld()", world_ScriptClearWorld);


    maps = fopen("game.cfg", "r");

    if(maps)
    {
        maps_file_buffer_size = path_GetFileSize(maps);

        if(maps_file_buffer_size)
        {
            maps_file_buffer = memory_Calloc(1, maps_file_buffer_size + 1);
            fread(maps_file_buffer, maps_file_buffer_size, 1, maps);
            fclose(maps);


            maps_file_buffer[maps_file_buffer_size] = '\0';


            i = 0;

            while(maps_file_buffer[i])
            {
                if(maps_file_buffer[i] == '[')
                {
                    i++;

                    j = i;

                    while(maps_file_buffer[j] != ']' &&
                          maps_file_buffer[j] != '\n' &&
                          maps_file_buffer[j] != '\r' &&
                          maps_file_buffer[j] != '\0')
                    {
                        j++;
                    }

                    if(maps_file_buffer[j] == ']')
                    {
                        maps_file_buffer[j] = '\0';

                        map = memory_Calloc(1, sizeof(struct world_level_t));

                        map->level_name = memory_Strdup(maps_file_buffer + i);

                        if(!w_levels)
                        {
                            w_levels = map;
                        }
                        else
                        {
                            w_last_level->next = map;
                            map->prev = w_last_level;
                        }

                        w_last_level = map;
                    }

                    i = j;
                }
            }

            memory_Free(maps_file_buffer);
        }

    }


	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	return 1;
}

void world_Finish()
{
	world_Clear(WORLD_CLEAR_FLAG_BSP | WORLD_CLEAR_FLAG_ENTITIES | WORLD_CLEAR_FLAG_LIGHTS | WORLD_CLEAR_FLAG_PHYSICS_MESH);

	list_destroy(&w_world_vars);
	//memory_Free(w_light_buffer);
	//list_destroy(&w_visible_entities);
}


int world_LoadBsp(char *file_name)
{
	/*FILE *file;
	bsp_header_t header;
	light_record_t light_record;
	file = fopen(file_name, "rb");

	if(!file)
	{
		printf("couldn't open file [%s]\n", file_name);
		return 0;
	}

	int i;
	int light_count;

	fread(&header, sizeof(bsp_header_t), 1, file);

	for(i = 0; i < header.light_count; i++)
	{
		fread(&light_record, sizeof(light_record_t), 1, file);

		light_CreateLight("light", &light_record.orientation, light_record.position, light_record.color, light_record.radius, light_record.energy, light_record.bm_flags);
	}

	world_nodes = malloc(sizeof(bsp_pnode_t) * header.world_nodes_count);
	world_leaves = malloc(sizeof(bsp_dleaf_t) * header.world_leaves_count);
	collision_nodes = malloc(sizeof(bsp_pnode_t) * header.collision_nodes_count);

	fread(world_nodes, sizeof(bsp_pnode_t), world_nodes_count, file);
	fread(world_leaves, sizeof(bsp_dleaf_t), world_leaves_count, file);
	fread(collision_nodes, sizeof(bsp_pnode_t), collision_nodes_count, file);



	fclose(file);


	return 1;*/

	return 0;
}


void world_MarkEntitiesOnLeaves()
{
	#if 0
	int i;
	int j;
	//mat4_t *transform;
	entity_t *entity;
	entity_aabb_t *aabb;
	//model_t *model;
	//ent_entity_transform_cursor = 0;
	bsp_dleaf_t *old_leaf;
	bsp_dleaf_t *cur_leaf;
	//collider_t *collider;


	int int_index;
	int bit_index;
	int leaf_index;

	for(i = 0; i < ent_entity_list_cursor; i++)
	{
		if(ent_entities[i].flags & ENTITY_INVALID)
			continue;

		if(ent_entities[i].flags & ENTITY_INVISIBLE)
			continue;

		entity = &ent_entities[i];
		aabb = &ent_aabbs[i];

		if(ent_entities[i].flags & ENTITY_HAS_MOVED)
		{
			/*if(entity->collider_index > -1)
			{
				collider = physics_GetColliderPointerIndex(entity->collider_index);

				entity->position = collider->position;
				entity->orientation = collider->orientation;
			}



			entity_UpdateEntityAabbIndex(i);*/

			int_index = i >> 5;
			bit_index = i % 32;

			cur_leaf = bsp_GetCurrentLeaf(w_world_nodes, entity->position);

			if(cur_leaf)
			{

				if(entity->leaf)
				{
					old_leaf = entity->leaf;
					leaf_index = old_leaf - w_world_leaves;

					w_leaf_entities[leaf_index].entities[int_index] &= ~(1 << bit_index);

					if(old_leaf->pvs)
					{
						for(j = 0; j < w_world_leaves_count; j++)
						{
							//if(old_leaf->pvs[j >> 3] & (1 << (j % 8)))
							if(LEAF_ON_PVS(old_leaf, j))
							{
								w_leaf_entities[j].entities[int_index] &= ~(1 << bit_index);
							}
						}
					}
				}


				leaf_index = cur_leaf - w_world_leaves;
				w_leaf_entities[leaf_index].entities[int_index] |= (1 << bit_index);

				if(cur_leaf->pvs)
				{
					for(j = 0; j < w_world_leaves_count; j++)
					{
						//if(cur_leaf->pvs[j >> 3] & (1 << (j % 8)))
						if(LEAF_ON_PVS(cur_leaf, j))
						{
							w_leaf_entities[j].entities[int_index] |= (1 << bit_index);
						}
					}
				}

			}

			entity->leaf = cur_leaf;
		}
	}

	#endif
}

void world_MarkLightsOnLeaves()
{
	int i;
	int j;
	int r;
	int int_index;
	int bit_index;
	int leaf_index;
	int lleaf_index;
	float energy;
	float radius;
	float dist;
	static int sign[4] = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
	struct bsp_dleaf_t *light_leaf;
	struct bsp_dleaf_t *leaf;
	struct bsp_dleaf_t *lleaf;
	bsp_lights_t *lights;
//	light_position_t *pos;
//	light_params_t *parms;

    struct light_position_data_t *position;
    struct light_params_data_t *params;

	struct bsp_dleaf_t **leaves;
	int leaves_count;
	unsigned long long s;
	unsigned long long e;
	vec3_t v;
	vec4_t v4;
	vec4_t leaf_extents4;
	vec4_t leaf_pos4;
	vec4_t light_pos4;


	if(!w_world_leaves)
		return;

	for(i = 0; i < l_light_positions.element_count; i++)
	{

	    //params = (struct light_params_data_t *)l_light_params.elements + i;

		/*if(l_light_params[i].bm_flags & LIGHT_INVALID)
			continue;

		if(!(l_light_params[i].bm_flags & LIGHT_MOVED))
			continue;*/

        position = (struct light_position_data_t *)l_light_positions.elements + i;

        if(!(position->flags & LIGHT_INVALID))
        {
            continue;
        }

        if(!(position->flags & LIGHT_MOVED))
        {
            continue;
        }



        position->flags &= ~LIGHT_MOVED;

        #if 0

		//l_light_params[i].bm_flags &= ~LIGHT_MOVED;

		light_leaf = bsp_GetCurrentLeaf(w_world_nodes, position->position);

		//printf("a\n");

		if(light_leaf)
		{
			//parms = &l_light_params[i];
			//pos = &l_light_positions[i];

			int_index = i >> 5;
			bit_index = i % 32;


			if(position->leaf)
			{
				/* don't update anything if this light didn't
				leave the leaf... */
				if((struct bsp_dleaf_t *)position->leaf != light_leaf)
				{
					//leaf = (bsp_dleaf_t *)parms->leaf;
					//leaf_index = leaf - w_world_leaves;
					//w_leaf_lights[leaf_index].lights[int_index] &= ~(1 << bit_index);

					leaves = bsp_DecompressPvs(position->leaf, &leaves_count);

					for(j = 0; j < leaves_count; j++)
					{
						leaf_index = leaves[j] - w_world_leaves;
						w_leaf_lights[leaf_index].lights[int_index] &= ~(1 << bit_index);
					}
				}
			}

			/* check to see which leaves from its current
			leaf's pvs this light touch... */
			//leaf_index = light_leaf - w_world_leaves;
			//w_leaf_lights[leaf_index].lights[int_index] |= 1 << bit_index;

			radius = UNPACK_LIGHT_RADIUS(position->radius);
			//energy = UNPACK_LIGHT_ENERGY(params->energy);
			radius *= radius;

			leaves = bsp_DecompressPvs(light_leaf, &leaves_count);

			light_pos4.x = position->position.x;
			light_pos4.y = position->position.y;
			light_pos4.z = position->position.z;
			light_pos4.w = 0.0;

			if(!light_leaf->pvs)
            {
                continue;
            }

			//#define BLODDY_SSE


			#ifdef BLODDY_SSE

			asm volatile
			(
				"movups xmm0, [%[light_pos4]]\n"
				"movups xmm3, [%[sign]]\n"
				:: [light_pos4] "rm" (light_pos4),
				   [sign] "rm" (sign)
			);

			#endif


			for(j = 0; j < leaves_count; j++)
			{
				//r = 1 << (j % 8);


				leaf = leaves[j];
				leaf_index = leaf - w_world_leaves;

				//if(light_leaf->pvs[j >> 3] & r)
				//{
				//leaf = &w_world_leaves[j];

				//s = _rdtsc();

				#ifdef BLODDY_SSE
				asm volatile
				(
					"movups xmm1, [%[leaf_pos4]]\n"
					"movups xmm2, [%[leaf_extents4]]\n"

					"movups xmm4, xmm2\n"					/* half extents... */
					"orps xmm4, xmm3\n"						/* negate half extents... */

					"movups xmm5, xmm0\n"					/* v = pos->position - leaf->center */
					"subps xmm5, xmm1\n"					/* ================================ */

					"minps xmm5, xmm2\n"					/* clamp v to the leaf's positive half-extents... */
					"maxps xmm5, xmm4\n"					/* clamp v to the leaf's negative half-extents...*/

					"addps xmm5, xmm1\n"					/* v += leaf->center */

					"subps xmm5, xmm0\n"					/* v -= pos->position */
					"movups [%[v4]], xmm5\n"				/* ================== */


					:[v4] "=m" (v4) : [leaf_pos4] "rm" (leaf->center),
					   				  [leaf_extents4] "rm" (leaf->extents)


				);

				#else

				v4.x = position->position.x - leaf->center.x;
				v4.y = position->position.y - leaf->center.y;
				v4.z = position->position.z - leaf->center.z;

				if(v4.x > leaf->extents.x) v4.x = leaf->extents.x;
				else if(v4.x < -leaf->extents.x) v4.x = -leaf->extents.x;

				if(v4.y > leaf->extents.y) v4.y = leaf->extents.y;
				else if(v4.y < -leaf->extents.y) v4.y = -leaf->extents.y;

				if(v4.z > leaf->extents.x) v4.z = leaf->extents.z;
				else if(v4.z < -leaf->extents.z) v4.z = -leaf->extents.z;


				v4.x += leaf->center.x;
				v4.y += leaf->center.y;
				v4.z += leaf->center.z;

				v4.x = position->position.x - v4.x;
				v4.y = position->position.y - v4.y;
				v4.z = position->position.z - v4.z;

				#endif

				dist = dot3(v4.vec3, v4.vec3);


				if(dist < radius)
				{
					w_leaf_lights[leaf_index].lights[int_index] |= 1 << bit_index;
				}
				else
				{
					w_leaf_lights[leaf_index].lights[int_index] &= ~(1 << bit_index);
				}


				//}
			}

			position->leaf = (struct bsp_dleaf_t *)light_leaf;



			/*if(pos->position.x > parms->box_max.x || pos->position.x < parms->box_min.x)
				goto _do_box;
			if(pos->position.y > parms->box_max.y || pos->position.y < parms->box_min.y)
				goto _do_box;
			if(pos->position.z > parms->box_max.z || pos->position.z < parms->box_min.z)
				goto _do_box;

			continue;*/

			//_do_box:
			world_VisibleLightTriangles(i);

			#undef BLODDY_SSE


		}

		#endif
	}
}


/*
=================================================
=================================================
=================================================
*/

#if 0

void world_WorldOnView(view_data_t *view_data)
{
	int visible_leaves_count = 0;
	int i;
	int j;
	bsp_dleaf_t **visible_leaves = NULL;
	bsp_dleaf_t **new_leaves = NULL;
	bsp_dleaf_t *leaf;
	mat4_t world_transform;
	vec3_t world_position;
	camera_t *active_camera = camera_GetActiveCamera();

	world_transform = view_data->view_matrix;
	mat4_t_inverse_transform(&world_transform);


	world_position.x = world_transform.floats[3][0];
	world_position.y = world_transform.floats[3][1];
	world_position.z = world_transform.floats[3][2];


	view_data->view_triangles_cursor = 0;
	view_data->view_leaves_list_cursor = 0;

	int positive_z;

	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	float qt = -nznear / ntop;
	float qr = -nznear / nright;
	float x_max;
	float x_min;
	float y_max;
	float y_min;

	vec4_t corners[8];


	visible_leaves = bsp_PotentiallyVisibleLeaves(&visible_leaves_count, world_position);

	if(visible_leaves)
	{

		for(j = 0; j < visible_leaves_count; j++)
		{
			//break;
			leaf = visible_leaves[j];

			corners[0].x = leaf->center.x - leaf->extents.x;
			corners[0].y = leaf->center.y + leaf->extents.y;
			corners[0].z = leaf->center.z + leaf->extents.z;
			corners[0].w = 1.0;

			corners[1].x = leaf->center.x - leaf->extents.x;
			corners[1].y = leaf->center.y - leaf->extents.y;
			corners[1].z = leaf->center.z + leaf->extents.z;
			corners[1].w = 1.0;

			corners[2].x = leaf->center.x + leaf->extents.x;
			corners[2].y = leaf->center.y - leaf->extents.y;
			corners[2].z = leaf->center.z + leaf->extents.z;
			corners[2].w = 1.0;

			corners[3].x = leaf->center.x + leaf->extents.x;
			corners[3].y = leaf->center.y + leaf->extents.y;
			corners[3].z = leaf->center.z + leaf->extents.z;
			corners[3].w = 1.0;



			corners[4].x = leaf->center.x - leaf->extents.x;
			corners[4].y = leaf->center.y + leaf->extents.y;
			corners[4].z = leaf->center.z - leaf->extents.z;
			corners[4].w = 1.0;

			corners[5].x = leaf->center.x - leaf->extents.x;
			corners[5].y = leaf->center.y - leaf->extents.y;
			corners[5].z = leaf->center.z - leaf->extents.z;
			corners[5].w = 1.0;

			corners[6].x = leaf->center.x + leaf->extents.x;
			corners[6].y = leaf->center.y - leaf->extents.y;
			corners[6].z = leaf->center.z - leaf->extents.z;
			corners[6].w = 1.0;

			corners[7].x = leaf->center.x + leaf->extents.x;
			corners[7].y = leaf->center.y + leaf->extents.y;
			corners[7].z = leaf->center.z - leaf->extents.z;
			corners[7].w = 1.0;

			x_max = -10.9;
			x_min = 10.9;

			y_max = -10.9;
			y_min = 10.9;

			positive_z = 0;

			for(i = 0; i < 8; i++)
			{
				mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &corners[i]);
				if(corners[i].z > nznear)
				{
					corners[i].z = nznear;
					positive_z++;
				}

				corners[i].x = (corners[i].x * qr) / corners[i].z;
				corners[i].y = (corners[i].y * qt) / corners[i].z;

				if(corners[i].x > x_max) x_max = corners[i].x;
				if(corners[i].x < x_min) x_min = corners[i].x;

				if(corners[i].y > y_max) y_max = corners[i].y;
				if(corners[i].y < y_min) y_min = corners[i].y;

			}


			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;

			if(x_min > 1.0) x_min = 1.0;
			else if(x_min < -1.0) x_min = -1.0;

			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;

			if(y_min > 1.0) y_min = 1.0;
			else if(y_min < -1.0) y_min = -1.0;


			if((x_max - x_min) * (y_max - y_min) <= 0.0 || positive_z == 8)
			{
				if(j < visible_leaves_count - 1)
				{
					visible_leaves[j] = visible_leaves[visible_leaves_count - 1];
					j--;
				}
				visible_leaves_count--;
			}

		}


		if(visible_leaves_count > view_data->view_leaves_list_size)
		{
			view_data->view_leaves_list_size = ((visible_leaves_count + 32) - 255) & (~255);
			memory_Free(view_data->view_leaves);
			view_data->view_leaves = memory_Malloc(sizeof(bsp_dleaf_t *) * view_data->view_leaves_list_size, "world_WorldOnView");
		}

		for(i = 0; i < visible_leaves_count; i++)
		{
			view_data->view_leaves[i] = visible_leaves[i];
		}
	}
}

#endif

#if 0

void world_WorldOnPortalView(portal_view_data_t *view_data)
{
	#if 0
	int visible_leaves_count = 0;
	int i;
	int j;
	bsp_dleaf_t **visible_leaves = NULL;
	bsp_dleaf_t **new_leaves = NULL;
	bsp_dleaf_t *leaf;
	mat4_t world_transform;
	vec3_t world_position;
	camera_t *active_camera = camera_GetActiveCamera();

	world_transform = view_data->view_data.view_matrix;
	mat4_t_inverse_transform(&world_transform);


	world_position.x = world_transform.floats[3][0];
	world_position.y = world_transform.floats[3][1];
	world_position.z = world_transform.floats[3][2];


	view_data->view_data.view_triangles_cursor = 0;
	view_data->view_data.view_leaves_list_cursor = 0;

	int positive_z;

	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	float qt = -nznear / ntop;
	float qr = -nznear / nright;
	float x_max;
	float x_min;
	float y_max;
	float y_min;

	vec4_t corners[8];


	visible_leaves = bsp_PotentiallyVisibleLeaves(&visible_leaves_count, world_position);

	if(visible_leaves)
	{

		for(j = 0; j < visible_leaves_count; j++)
		{
			leaf = visible_leaves[j];

			corners[0].x = leaf->center.x - leaf->extents.x;
			corners[0].y = leaf->center.y + leaf->extents.y;
			corners[0].z = leaf->center.z + leaf->extents.z;
			corners[0].w = 1.0;

			corners[1].x = leaf->center.x - leaf->extents.x;
			corners[1].y = leaf->center.y - leaf->extents.y;
			corners[1].z = leaf->center.z + leaf->extents.z;
			corners[1].w = 1.0;

			corners[2].x = leaf->center.x + leaf->extents.x;
			corners[2].y = leaf->center.y - leaf->extents.y;
			corners[2].z = leaf->center.z + leaf->extents.z;
			corners[2].w = 1.0;

			corners[3].x = leaf->center.x + leaf->extents.x;
			corners[3].y = leaf->center.y + leaf->extents.y;
			corners[3].z = leaf->center.z + leaf->extents.z;
			corners[3].w = 1.0;



			corners[4].x = leaf->center.x - leaf->extents.x;
			corners[4].y = leaf->center.y + leaf->extents.y;
			corners[4].z = leaf->center.z - leaf->extents.z;
			corners[4].w = 1.0;

			corners[5].x = leaf->center.x - leaf->extents.x;
			corners[5].y = leaf->center.y - leaf->extents.y;
			corners[5].z = leaf->center.z - leaf->extents.z;
			corners[5].w = 1.0;

			corners[6].x = leaf->center.x + leaf->extents.x;
			corners[6].y = leaf->center.y - leaf->extents.y;
			corners[6].z = leaf->center.z - leaf->extents.z;
			corners[6].w = 1.0;

			corners[7].x = leaf->center.x + leaf->extents.x;
			corners[7].y = leaf->center.y + leaf->extents.y;
			corners[7].z = leaf->center.z - leaf->extents.z;
			corners[7].w = 1.0;

			x_max = -10.9;
			x_min = 10.9;

			y_max = -10.9;
			y_min = 10.9;

			positive_z = 0;

			for(i = 0; i < 8; i++)
			{
				mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &corners[i]);
				if(corners[i].z > nznear)
				{
					corners[i].z = nznear;
					positive_z++;
				}

				corners[i].x = (corners[i].x * qr) / corners[i].z;
				corners[i].y = (corners[i].y * qt) / corners[i].z;

				if(corners[i].x > x_max) x_max = corners[i].x;
				if(corners[i].x < x_min) x_min = corners[i].x;

				if(corners[i].y > y_max) y_max = corners[i].y;
				if(corners[i].y < y_min) y_min = corners[i].y;

			}


			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;

			if(x_min > 1.0) x_min = 1.0;
			else if(x_min < -1.0) x_min = -1.0;

			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;

			if(y_min > 1.0) y_min = 1.0;
			else if(y_min < -1.0) y_min = -1.0;


			if((x_max - x_min) * (y_max - y_min) <= 0.0 || positive_z == 8)
			{
				if(j < visible_leaves_count - 1)
				{
					visible_leaves[j] = visible_leaves[visible_leaves_count - 1];
					j--;
				}
				visible_leaves_count--;
			}

		}


		if(visible_leaves_count > view_data->view_data.view_leaves_list_size)
		{
			view_data->view_data.view_leaves_list_size = ((visible_leaves_count + 32) - 255) & (~255);
			memory_Free(view_data->view_data.view_leaves);
			view_data->view_data.view_leaves = memory_Malloc(sizeof(bsp_dleaf_t *) * view_data->view_data.view_leaves_list_size, "world_WorldOnView");
		}

		for(i = 0; i < visible_leaves_count; i++)
		{
			view_data->view_data.view_leaves[i] = visible_leaves[i];
		}
	}

	#endif
}

#endif

#if 0

void world_WorldOnViews()
{
	camera_t *current_view;
	current_view = cameras;


	while(current_view)
	{

		if(!(current_view->bm_flags & CAMERA_INACTIVE))
		{
			current_view = current_view->next;
			continue;
		}

		world_WorldOnView(&current_view->view_data);

		current_view = current_view->next;
	}

}

#endif



void world_LightBounds()
{
    #if 0
	int i;
	int c;

	int x;
	int y;
	int z;

//	camera_t *active_camera = camera_GetActiveCamera();
	//camera_t *active_camera = view;
	vec4_t light_origin;

//	view_clusters_t *view_clusters;
//	view_light_t *view_light;
	mat4_t mt;

	vec2_t ac;
	vec2_t lb;
	vec2_t rb;

	//float nzfar = -view->frustum.zfar;
	//float nznear = -view->frustum.znear;
	//float ntop = view->frustum.top;
	//float nright = view->frustum.right;

	float x_max;
	float x_min;
	float y_max;
	float y_min;
	float d;
	float t;
	float denom;
	float si;
	float co;
	float k;
	float l;
	float light_radius;
	float light_z;
	int positive_z;
	int light_index;
	int view_clusters_index;
	int offset;

	light_position_t *position;
	light_params_t *params;

	//float qt = -view_data->projection_matrix.floats[1][1];
	//float qr = -view_data->projection_matrix.floats[0][0];
	/* for z' == 1 -> z == znear */
	//float znear = view_data->projection_matrix.floats[3][2] / 2.0;

	float znear = -active_camera->frustum.znear;
	float qr = znear / active_camera->frustum.right;
	float qt = znear / active_camera->frustum.top;

	/* FUCK!!! */
	float zfar = -active_camera->frustum.zfar;
	/* for z' == -1 ->z == zfar */
	//float zfar = view_data->projection_matrix.floats[3][3] - view_data->projection_matrix.floats[3][2];

	short cluster_x_start;
	short cluster_y_start;
	short cluster_z_start;
	short cluster_x_end;
	short cluster_y_end;
	short cluster_z_end;

	#define CLUSTER_NEAR 1.0

	denom = log(-zfar / CLUSTER_NEAR);

	//c = view_data->view_lights_list_cursor;
	c = w_visible_lights_count;

	for(i = 0; i < c; i++)
	{

		//light_index = view_data->view_lights[i].light_index;
		light_index = w_visible_lights[i];


		position = &l_light_positions[light_index];
		params = &l_light_params[light_index];

		light_origin.x = position->position.x;
		light_origin.y = position->position.y;
		light_origin.z = position->position.z;
		light_origin.w = 1.0;

		//mat4_t_vec4_t_mult(&view_data->view_matrix, &light_origin);
		mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &light_origin);

		light_radius = UNPACK_LIGHT_RADIUS(params->radius);

		d = light_origin.x * light_origin.x + light_origin.y * light_origin.y + light_origin.z * light_origin.z;

		if(light_radius * light_radius > d)
		{
			x_max = 1.0;
			x_min = -1.0;
			y_max = 1.0;
			y_min = -1.0;
			positive_z = 0;
			goto _skip_stuff;
		}


		/*
			from   '2D Polyhedral Bounds of a Clipped,
			   		Perspective-Projected 3D Sphere
			   		by Michael Mara and Morgan McGuire'
		*/

		x_max = -10.0;
		x_min = 10.0;

		y_max = -10.0;
		y_min = 10.0;

		positive_z = 0;

		ac.x = light_origin.x;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
		l = light_radius * light_radius;
		k = znear - ac.y;
		k = sqrt(light_radius * light_radius - k * k);

		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);


			si = light_radius / d;
			co = t / d;

			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;

			if(rb.y > znear)
			{
				rb.x = ac.x + k;
				rb.y = znear;
				positive_z++;
			}

			if(lb.y > znear)
			{
				lb.x = ac.x - k;
				lb.y = znear;
				positive_z++;
			}

			x_min = (qr * lb.x) / lb.y;
			x_max = (qr * rb.x) / rb.y;

			if(x_min < -1.0) x_min = -1.0;
			else if(x_min > 1.0) x_min = 1.0;
			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;
		}
		else
		{
			x_min = -1.0;
			x_max = 1.0;
		}

		ac.x = light_origin.y;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
				//l = light_radius * light_radius;

		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);
					//k = nznear - ac.y;
					//k = sqrt(light_radius * light_radius - k * k);

			si = light_radius / d;
			co = t / d;

			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;


			if(rb.y > znear)
			{
				rb.x = ac.x + k;
				rb.y = znear;
				positive_z++;
			}

			if(lb.y > znear)
			{
				lb.x = ac.x - k;
				lb.y = znear;
				positive_z++;
			}

			y_min = (qt * lb.x) / lb.y;
			y_max = (qt * rb.x) / rb.y;

			if(y_min < -1.0) y_min = -1.0;
			else if(y_min > 1.0) y_min = 1.0;
			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;
		}
		else
		{
			y_min = -1.0;
			y_max = 1.0;
		}

		if((x_max - x_min) * (y_max - y_min) == 0.0 || positive_z == 4)
		{
			_remove_light:

			if(i < c)
			{
				w_visible_lights[i] = w_visible_lights[c - 1];
				//view_data->view_lights[i] = view_data->view_lights[c - 1];
			}
			c--;
			i--;
			continue;
		}
		else
		{

			_skip_stuff:

			cluster_x_start = (int)((x_min * 0.5 + 0.5) * r_clusters_per_row);
			cluster_y_start = (int)((y_min * 0.5 + 0.5) * r_cluster_rows);
			light_z = light_origin.z + light_radius;

			if(cluster_x_start >= r_clusters_per_row) cluster_x_start = r_clusters_per_row - 1;
			if(cluster_y_start >= r_cluster_rows) cluster_y_start = r_cluster_rows - 1;


			if(light_z > -CLUSTER_NEAR) cluster_z_start = 0;
			else cluster_z_start = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(r_cluster_layers));

			if(cluster_z_start > r_cluster_layers) cluster_z_start = r_cluster_layers - 1;


			cluster_x_end = (int)((x_max * 0.5 + 0.5) * r_clusters_per_row);
			cluster_y_end = (int)((y_max * 0.5 + 0.5) * r_cluster_rows);
			light_z = light_origin.z - light_radius;

			if(cluster_x_end >= r_clusters_per_row) cluster_x_end = r_clusters_per_row - 1;
			if(cluster_y_end >= r_cluster_rows) cluster_y_end = r_cluster_rows - 1;

			if(light_z > -CLUSTER_NEAR) cluster_z_end = 0;
			else cluster_z_end = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(r_cluster_layers));

			if(cluster_z_end > r_cluster_layers) cluster_z_end = r_cluster_layers - 1;


			params->first_cluster.x = cluster_x_start;
			params->first_cluster.y = cluster_y_start;
			params->first_cluster.z = cluster_z_start;

			params->last_cluster.x = cluster_x_end;
			params->last_cluster.y = cluster_y_end;
			params->last_cluster.z = cluster_z_end;


			//params->first_cluster = PACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end);
			//view_data->view_lights[i].light_index = light_index;
			//view_data->view_lights[i].view_clusters = PACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end);
			//view_clusters->clusters = params->first_cluster;
			//view_clusters->view = view;
			//view->view_lights[i].view_cluster_index = view_clusters_index;
		}



		//renderer_Draw2dLine(vec2(x_min, y_max), vec2(x_min, y_min), vec3_t_c(0.0, 1.0, 0.0), 1.0, 1);
		//renderer_Draw2dLine(vec2(x_min, y_min), vec2(x_max, y_min), vec3_t_c(0.0, 1.0, 0.0), 1.0, 1);
		//renderer_Draw2dLine(vec2(x_max, y_min), vec2(x_max, y_max), vec3_t_c(0.0, 1.0, 0.0), 1.0, 1);
		//renderer_Draw2dLine(vec2(x_max, y_max), vec2(x_min, y_max), vec3_t_c(0.0, 1.0, 0.0), 1.0, 1);



	}

	w_visible_lights_count = c;

	#endif

}


#if 0

void world_LightBoundsOnView(view_data_t *view_data)
{
	int i;
	int c;

	int x;
	int y;
	int z;

	camera_t *active_camera = camera_GetActiveCamera();
	//camera_t *active_camera = view;
	vec4_t light_origin;

	view_clusters_t *view_clusters;
	view_light_t *view_light;
	mat4_t mt;

	vec2_t ac;
	vec2_t lb;
	vec2_t rb;

	//float nzfar = -view->frustum.zfar;
	//float nznear = -view->frustum.znear;
	//float ntop = view->frustum.top;
	//float nright = view->frustum.right;

	float x_max;
	float x_min;
	float y_max;
	float y_min;
	float d;
	float t;
	float denom;
	float si;
	float co;
	float k;
	float l;
	float light_radius;
	float light_z;
	int positive_z;
	int light_index;
	int view_clusters_index;
	int offset;

	light_position_t *position;
	light_params_t *params;

	//float qt = -view_data->projection_matrix.floats[1][1];
	//float qr = -view_data->projection_matrix.floats[0][0];
	/* for z' == 1 -> z == znear */
	//float znear = view_data->projection_matrix.floats[3][2] / 2.0;

	float znear = -active_camera->frustum.znear;
	float qr = znear / active_camera->frustum.right;
	float qt = znear / active_camera->frustum.top;

	/* FUCK!!! */
	float zfar = -active_camera->frustum.zfar;
	/* for z' == -1 ->z == zfar */
	//float zfar = view_data->projection_matrix.floats[3][3] - view_data->projection_matrix.floats[3][2];

	short cluster_x_start;
	short cluster_y_start;
	short cluster_z_start;
	short cluster_x_end;
	short cluster_y_end;
	short cluster_z_end;

	#define CLUSTER_NEAR 1.0

	denom = log(-zfar / CLUSTER_NEAR);

	c = view_data->view_lights_list_cursor;

	for(i = 0; i < c; i++)
	{

		light_index = view_data->view_lights[i].light_index;


		position = &l_light_positions[light_index];
		params = &l_light_params[light_index];

		light_origin.x = position->position.x;
		light_origin.y = position->position.y;
		light_origin.z = position->position.z;
		light_origin.w = 1.0;

		mat4_t_vec4_t_mult(&view_data->view_matrix, &light_origin);

		light_radius = LIGHT_RADIUS(params->radius);

		d = light_origin.x * light_origin.x + light_origin.y * light_origin.y + light_origin.z * light_origin.z;

		if(light_radius * light_radius > d)
		{
			x_max = 1.0;
			x_min = -1.0;
			y_max = 1.0;
			y_min = -1.0;
			positive_z = 0;
			goto _skip_stuff;
		}


		/*
			from   '2D Polyhedral Bounds of a Clipped,
			   		Perspective-Projected 3D Sphere
			   		by Michael Mara and Morgan McGuire'
		*/

		x_max = -10.0;
		x_min = 10.0;

		y_max = -10.0;
		y_min = 10.0;

		positive_z = 0;

		ac.x = light_origin.x;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
		l = light_radius * light_radius;
		k = -znear - ac.y;
		k = sqrt(light_radius * light_radius - k * k);

		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);


			si = light_radius / d;
			co = t / d;

			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;

			if(rb.y > znear)
			{
				rb.x = ac.x + k;
				rb.y = znear;
				positive_z++;
			}

			if(lb.y > znear)
			{
				lb.x = ac.x - k;
				lb.y = znear;
				positive_z++;
			}

			x_min = (qr * lb.x) / lb.y;
			x_max = (qr * rb.x) / rb.y;

			if(x_min < -1.0) x_min = -1.0;
			else if(x_min > 1.0) x_min = 1.0;
			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;
		}
		else
		{
			x_min = -1.0;
			x_max = 1.0;
		}

		ac.x = light_origin.y;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
				//l = light_radius * light_radius;

		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);
					//k = nznear - ac.y;
					//k = sqrt(light_radius * light_radius - k * k);

			si = light_radius / d;
			co = t / d;

			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;


			if(rb.y > znear)
			{
				rb.x = ac.x + k;
				rb.y = znear;
				positive_z++;
			}

			if(lb.y > znear)
			{
				lb.x = ac.x - k;
				lb.y = znear;
				positive_z++;
			}

			y_min = (qt * lb.x) / lb.y;
			y_max = (qt * rb.x) / rb.y;

			if(y_min < -1.0) y_min = -1.0;
			else if(y_min > 1.0) y_min = 1.0;
			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;
		}
		else
		{
			y_min = -1.0;
			y_max = 1.0;
		}

		if((x_max - x_min) * (y_max - y_min) == 0.0 || positive_z == 4)
		{
			_remove_light:

			if(i < c)
			{
				view_data->view_lights[i] = view_data->view_lights[c - 1];
			}
			c--;
			i--;
			continue;
		}
		else
		{

			_skip_stuff:

			cluster_x_start = (int)((x_min * 0.5 + 0.5) * CLUSTERS_PER_ROW);
			cluster_y_start = (int)((y_min * 0.5 + 0.5) * CLUSTER_ROWS);
			light_z = light_origin.z + light_radius;

			if(light_z > -CLUSTER_NEAR) cluster_z_start = 0;
			else cluster_z_start = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(CLUSTER_LAYERS));
			if(cluster_z_start > CLUSTER_LAYERS)
				cluster_z_start = CLUSTER_LAYERS - 1;

			//params->first_cluster = PACK_CLUSTER_INDEXES(cluster_x_start, cluster_y_start, cluster_z_start);



			cluster_x_end = (int)((x_max * 0.5 + 0.5) * CLUSTERS_PER_ROW);
			cluster_y_end = (int)((y_max * 0.5 + 0.5) * CLUSTER_ROWS);
			light_z = light_origin.z - light_radius;

			if(light_z > -CLUSTER_NEAR) cluster_z_end = 0;
			else cluster_z_end = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(CLUSTER_LAYERS));
			if(cluster_z_end > CLUSTER_LAYERS)
				cluster_z_end = CLUSTER_LAYERS - 1;

			//params->first_cluster = PACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end);
			view_data->view_lights[i].light_index = light_index;
			view_data->view_lights[i].view_clusters = PACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end);
			//view_clusters->clusters = params->first_cluster;
			//view_clusters->view = view;
			//view->view_lights[i].view_cluster_index = view_clusters_index;
		}
	}

	view_data->view_lights_list_cursor = c;
}

#endif


#if 0
int visible_lights_index_list[MAX_WORLD_LIGHTS];
int visible_lights_index_list_cursor = 0;

void world_LightsOnView(view_data_t *view_data)
{
	int i;
	int c;

	int j;
	int k;
	int leaf_index;
	int int_index;
	int bit_index;

	camera_t *current_view;

	bsp_dleaf_t *leaf;
	bsp_dleaf_t *cur_leaf;
	//camera_t *active_camera = camera_GetActiveCamera();
	//vec4_t light_origin;
	//vec3_t v;
	//vec3_t farthest;
	float farthest;
	int farthest_index;
	light_position_t *pos;
	light_params_t *parms;
	bsp_lights_t lights;
	int current_visible_light_count = 0;
	view_light_t *view_lights;

	vec3_t v;

	float d;
	float radius;
	float s;
	float e;

	//visible_light_count = 0;

	//current_view = cameras;

	//while(current_view)
	{
	//	if(current_view->bm_flags & CAMERA_INACTIVE)
	//	{
	//		current_view = current_view->next;
	//		continue;
	//	}

		view_data->view_lights_list_cursor = 0;
		current_visible_light_count = 0;

		for(i = 0; i < MAX_WORLD_LIGHTS >> 5; i++)
		{
			lights.lights[i] = 0;
		}

		if(!w_world_leaves)
		{
			for(i = 0; i < l_light_list_cursor; i++)
			{
				if(!(l_light_params[i].bm_flags & LIGHT_INVALID))
				{
					lights.lights[i >> 5] |= 1 << (i % 32);
				}
			}

			current_visible_light_count = l_light_count;
		}
		else
		{
			/*for(i = 0, k = 0; i < visible_leaves_count; i++)
			{
				leaf = visible_leaves[i];

				leaf_index = leaf - world_leaves;

				for(j = 0; j < MAX_WORLD_LIGHTS >> 5; j++)
				{
					lights.lights[j] |= leaf_lights[leaf_index].lights[j];
					current_visible_light_count++;
				}
			}*/
		}

		#define VIEW_LIGHT_LIST_SIZE_INCREMENT 128

		if(current_visible_light_count >= view_data->view_lights_list_size)
		{
			c = (current_visible_light_count - VIEW_LIGHT_LIST_SIZE_INCREMENT - 1) & (~(VIEW_LIGHT_LIST_SIZE_INCREMENT - 1));

			view_lights = memory_Malloc(sizeof(view_light_t) * (view_data->view_lights_list_size + c), "world_LightsOnViews");
			memcpy(view_lights, view_data->view_lights, sizeof(view_light_t) * view_data->view_lights_list_size);
			memory_Free(view_data->view_lights);
			view_data->view_lights = view_lights;
			view_data->view_lights_list_size += c;
		}

		for(i = 0; i < MAX_WORLD_LIGHTS; i++)
		{
			/*if(!(lights.lights[i >> 5] & 0xffffffff))
			{
				i += 31;
				continue;
			}*/

			if(lights.lights[i >> 5] & (1 << (i % 32)))
			{
				view_data->view_lights[view_data->view_lights_list_cursor].light_index = i;
				view_data->view_lights_list_cursor++;
			}
		}





		//_light_bounds:
		world_LightBoundsOnView(view_data);

		//light_LightBounds();

		/* drop far away lights... */
		/* NOTE: quicksorting the lights might scale better... */
	/*	if(view_data->view_lights_list_cursor > MAX_VISIBLE_LIGHTS)
		{

			c = view_data->view_lights_list_cursor - MAX_VISIBLE_LIGHTS;

			for(i = 0; i < c; i++)
			{
				farthest = FLT_MIN;

				for(j = 0; j < current_view->view_data.view_lights_list_cursor; j++)
				{
					k = current_view->view_data.view_lights[j].light_index;

					v.x = l_light_positions[k].position.x - current_view->world_position.x;
					v.y = l_light_positions[k].position.y - current_view->world_position.y;
					v.z = l_light_positions[k].position.z - current_view->world_position.z;

					d = dot3(v, v);

					if(d > farthest)
					{
						farthest = d;
						farthest_index = j;
					}
				}

				if(farthest_index < current_view->view_data.view_lights_list_cursor - 1)
				{
					current_view->view_data.view_lights[farthest_index] = current_view->view_data.view_lights[current_view->view_data.view_lights_list_cursor - 1];
				}

				current_view->view_data.view_lights_list_cursor--;
			}
		}*/

		//current_view = current_view->next;
	}
}

#endif


#if 0

void world_LightsOnViews()
{
	int i;
	int c;

	int j;
	int k;
	int leaf_index;
	int int_index;
	int bit_index;

	camera_t *current_view;

	bsp_dleaf_t *leaf;
	bsp_dleaf_t *cur_leaf;
	//camera_t *active_camera = camera_GetActiveCamera();
	//vec4_t light_origin;
	//vec3_t v;
	//vec3_t farthest;
	float farthest;
	int farthest_index;
	light_position_t *pos;
	light_params_t *parms;
	bsp_lights_t lights;
	int current_visible_light_count = 0;
	view_light_t *view_lights;
	view_data_t *view_data;

	vec3_t v;

	float d;
	float radius;
	float s;
	float e;

	//visible_light_count = 0;

	current_view = cameras;

	while(current_view)
	{
		if(current_view->bm_flags & CAMERA_INACTIVE)
		{
			current_view = current_view->next;
			continue;
		}

		/*current_view->view_data.view_lights_list_cursor = 0;
		current_visible_light_count = 0;

		for(i = 0; i < MAX_WORLD_LIGHTS >> 5; i++)
		{
			lights.lights[i] = 0;
		}

		if(!world_leaves)
		{
			for(i = 0; i < l_light_list_cursor; i++)
			{
				if(!(l_light_params[i].bm_flags & LIGHT_INVALID))
				{
					lights.lights[i >> 5] |= 1 << (i % 32);
				}
			}

			current_visible_light_count = l_light_count;
		}
		else
		{

		}*/

	/*	#define VIEW_LIGHT_LIST_SIZE_INCREMENT 128

		if(current_visible_light_count >= current_view->view_data.view_lights_list_size)
		{
			c = (current_visible_light_count - VIEW_LIGHT_LIST_SIZE_INCREMENT - 1) & (~(VIEW_LIGHT_LIST_SIZE_INCREMENT - 1));

			view_lights = memory_Malloc(sizeof(view_light_t) * (current_view->view_data.view_lights_list_size + c), "world_LightsOnViews");
			memcpy(view_lights, current_view->view_data.view_lights, sizeof(view_light_t) * current_view->view_data.view_lights_list_size);
			memory_Free(current_view->view_data.view_lights);
			current_view->view_data.view_lights = view_lights;
			current_view->view_data.view_lights_list_size += c;
		}

		for(i = 0; i < MAX_WORLD_LIGHTS; i++)
		{


			if(lights.lights[i >> 5] & (1 << (i % 32)))
			{
				current_view->view_data.view_lights[current_view->view_data.view_lights_list_cursor].light_index = i;
				current_view->view_data.view_lights_list_cursor++;
			}
		}
		*/


		world_LightsOnView(&current_view->view_data);
		//world_LightBoundsOnView(&current_view->view_data);

		//light_LightBounds();

		/* drop far away lights... */
		/* NOTE: quicksorting the lights might scale better... */
		//if(visible_light_count > MAX_VISIBLE_LIGHTS)
		if(current_view->view_data.view_lights_list_cursor > MAX_VISIBLE_LIGHTS)
		{

			c = current_view->view_data.view_lights_list_cursor - MAX_VISIBLE_LIGHTS;

			for(i = 0; i < c; i++)
			{
				farthest = FLT_MIN;

				for(j = 0; j < current_view->view_data.view_lights_list_cursor; j++)
				{
					//k = visible_lights[j];
					k = current_view->view_data.view_lights[j].light_index;

					v.x = l_light_positions[k].position.x - current_view->world_position.x;
					v.y = l_light_positions[k].position.y - current_view->world_position.y;
					v.z = l_light_positions[k].position.z - current_view->world_position.z;

					d = dot3(v, v);

					if(d > farthest)
					{
						farthest = d;
						farthest_index = j;
					}
				}

				if(farthest_index < current_view->view_data.view_lights_list_cursor - 1)
				{
					current_view->view_data.view_lights[farthest_index] = current_view->view_data.view_lights[current_view->view_data.view_lights_list_cursor - 1];
				}

				current_view->view_data.view_lights_list_cursor--;
			}
		}

		view_data = &current_view->view_data;

		/*for(i = 0; i < current_view->view_data.view_portals_list_cursor; i++)
		{
			world_LightsOnPortals(&ptl_portals[view_data->view_portals[i]]);
		}*/


		current_view = current_view->next;
	}

	/*for(i = 0; i < ptl_portal_list_cursor; i++)
	{
		world_LightsOnPortal(&ptl_portals[i]);
	}*/

}


#endif

//void world_LightBoundsOnView(camera_t *view)


#if 0
void world_EntitiesOnViews()
{
	camera_t *current_view;

	current_view = cameras;

	while(current_view)
	{
		if(!(current_view->bm_flags & CAMERA_INACTIVE))
		{
			//current_view->view_draw_command_list_cursor = 0;
		}

		current_view = current_view->next;
	}

}

#endif


#if 0
void world_PortalOnView(view_data_t *view_data, portal_t *portal, vec3_t position)
{
	//bsp_dleaf_t *portal_leaf;
	//portal_recursive_view_data_t *recursive_view_data;
	//portal_view_data_t *new_view_data;
	unsigned short *portal_indexes;

	vec4_t portal_verts[4];
	//vec3_t current_portal_forward_vector;
	//vec3_t current_view_forward_vector;
	camera_t *active_view = camera_GetActiveCamera();
	vec3_t view_portal_vec;

	vec3_t portal_right_vector;
	vec3_t portal_up_vector;
	vec4_t portal_forward_vector;
	vec4_t portal_position;

	float d;

	//mat4_t current_view_projection_matrix;
	//mat4_t portal_transform;
	//mat4_t model_view_projection_matrix;
	mat4_t view_matrix;
	int i;
	int j;
	int k;


	float portal_near_d;
	float portal_vert_d;

	float qr;
	//float ql;
	float qt;
	float znear;
	//float qb;

	int portal_index;

	float x_max;
	float x_min;
	float y_max;
	float y_min;
	int positive_z;


	/*qr = -view_data->projection_matrix.floats[0][0];
	qt = -view_data->projection_matrix.floats[1][1];
	znear = view_data->projection_matrix.floats[3][2];*/

	znear = -active_view->frustum.znear;
	qr = znear / active_view->frustum.right;
	qt = znear / active_view->frustum.top;

	//current_portal = &ptl_portals[i];

	portal_forward_vector.x = portal->orientation.floats[0][2];
	portal_forward_vector.y = portal->orientation.floats[1][2];
	portal_forward_vector.z = portal->orientation.floats[2][2];
	portal_forward_vector.w = 0.0;

	portal_right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
	portal_right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
	portal_right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;

	portal_up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
	portal_up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
	portal_up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;

	portal_position.x = portal->position.x;
	portal_position.y = portal->position.y;
	portal_position.z = portal->position.z;
	portal_position.w = 1.0;

	portal_verts[0].x = portal_position.x - portal_right_vector.x + portal_up_vector.x;
	portal_verts[0].y = portal_position.y - portal_right_vector.y + portal_up_vector.y;
	portal_verts[0].z = portal_position.z - portal_right_vector.z + portal_up_vector.z;
	portal_verts[0].w = 1.0;

	portal_verts[1].x = portal_position.x - portal_right_vector.x - portal_up_vector.x;
	portal_verts[1].y = portal_position.y - portal_right_vector.y - portal_up_vector.y;
	portal_verts[1].z = portal_position.z - portal_right_vector.z - portal_up_vector.z;
	portal_verts[1].w = 1.0;

	portal_verts[2].x = portal_position.x + portal_right_vector.x - portal_up_vector.x;
	portal_verts[2].y = portal_position.y + portal_right_vector.y - portal_up_vector.y;
	portal_verts[2].z = portal_position.z + portal_right_vector.z - portal_up_vector.z;
	portal_verts[2].w = 1.0;

	portal_verts[3].x = portal_position.x + portal_right_vector.x + portal_up_vector.x;
	portal_verts[3].y = portal_position.y + portal_right_vector.y + portal_up_vector.y;
	portal_verts[3].z = portal_position.z + portal_right_vector.z + portal_up_vector.z;
	portal_verts[3].w = 1.0;

	view_portal_vec.x = position.x - portal_position.x;
	view_portal_vec.y = position.y - portal_position.y;
	view_portal_vec.z = position.z - portal_position.z;

	x_max = -10.0;
	x_min = 10.0;
	y_max = -10.0;
	y_min = 10.0;
	positive_z = 0;

	/* could cache this... */
	//mat4_t_vec4_t_mult(&view_data->view_matrix, &portal_forward_vector);
	//mat4_t_vec4_t_mult(&view_data->view_matrix, &portal_position);

	/* could cache this... */
	//portal_near_d = dot3(portal_position.vec3, portal_forward_vector.vec3);

	//printf("%f\n", portal_near_d);

	for(k = 0; k < 4; k++)
	{
		mat4_t_vec4_t_mult(&view_data->view_matrix, &portal_verts[k]);
		//portal_vert_d = dot3(portal_verts[k].vec3, portal_forward_vector.vec3) + portal_near_d;

		/*if(portal_vert_d > 0.0)
		{
			positive_z++;
		}*/

		//printf("Z: %f %f\n", portal_verts[k].z, znear);

		/*if(portal_verts[k].z > portal_verts[k].w)
		{
			positive_z++;
		}*/

		if(portal_verts[k].z > znear)
		{
			portal_verts[k].z = znear;
			positive_z++;
		}

		portal_verts[k].x = (portal_verts[k].x * qr) / portal_verts[k].z;
		portal_verts[k].y = (portal_verts[k].y * qt) / portal_verts[k].z;

		if(portal_verts[k].x > x_max) x_max = portal_verts[k].x;
		if(portal_verts[k].x < x_min) x_min = portal_verts[k].x;

		if(portal_verts[k].y > y_max) y_max = portal_verts[k].y;
		if(portal_verts[k].y < y_min) y_min = portal_verts[k].y;
	}

	//printf("[%f %f]   [%f %f]\n", x_min, x_max, y_min, y_max);


	if(x_max > 1.0) x_max = 1.0;
	else if(x_max < -1.0) x_max = -1.0;

	if(x_min > 1.0) x_min = 1.0;
	else if(x_min < -1.0) x_min = -1.0;

	if(y_max > 1.0) y_max = 1.0;
	else if(y_max < -1.0) y_max = -1.0;

	if(y_min > 1.0) y_min = 1.0;
	else if(y_min < -1.0) y_min = -1.0;

	//printf("[%f %f %f]\n", portal_forward_vector.x, portal_forward_vector.y, portal_forward_vector.z);

	if((x_max - x_min) * (y_max - y_min) > 0.0 && positive_z < 4)
	{
		//if(dot3(portal_forward_vector.vec3, portal_position.vec3) > 0.0)
		if(dot3(portal_forward_vector.vec3, view_portal_vec) < 0.0)
		{
			if(view_data->view_portals_frame != r_frame)
			{
				view_data->view_portals_frame = r_frame;
				view_data->view_portals_list_cursor = 0;
			}

			if(view_data->view_portals_list_cursor >= view_data->view_portals_list_size)
			{
				portal_indexes = memory_Malloc(sizeof(unsigned short) * (view_data->view_portals_list_size + 16), "world_PortalOnView");
				memcpy(portal_indexes, view_data->view_portals, sizeof(unsigned short) * view_data->view_portals_list_size);
				memory_Free(view_data->view_portals);
				view_data->view_portals = portal_indexes;
				view_data->view_portals_list_size += 16;
			}

			portal_index = portal - ptl_portals;

			view_data->view_portals[view_data->view_portals_list_cursor] = portal_index;
			view_data->view_portals_list_cursor++;

	/*		if(view_data == &active_view->view_data)
			{
				renderer_Draw2dLine(vec2(x_min, y_max), vec2(x_min, y_min), vec3(0.0, 0.0, 1.0), 2.0, 0);
				renderer_Draw2dLine(vec2(x_min, y_min), vec2(x_max, y_min), vec3(0.0, 0.0, 1.0), 2.0, 0);
				renderer_Draw2dLine(vec2(x_max, y_min), vec2(x_max, y_max), vec3(0.0, 0.0, 1.0), 2.0, 0);
				renderer_Draw2dLine(vec2(x_max, y_max), vec2(x_min, y_max), vec3(0.0, 0.0, 1.0), 2.0, 0);
			}*/

		}

	}
}

#endif

#if 0

void world_PortalOnPortalView(portal_view_data_t *view_data, portal_t *portal)
{
	#if 1
	//bsp_dleaf_t *portal_leaf;
	//portal_recursive_view_data_t *recursive_view_data;
	//portal_view_data_t *new_view_data;
	unsigned short *portal_indexes;
	camera_t *active_view = camera_GetActiveCamera();
	vec4_t portal_verts[4];
	//vec3_t current_portal_forward_vector;
	//vec3_t current_view_forward_vector;
	vec3_t view_portal_vec;

	vec3_t portal_right_vector;
	vec3_t portal_up_vector;
	vec4_t portal_forward_vector;
	vec4_t portal_position;

	vec4_t viewing_portal_forward_vector;
	vec4_t viewing_portal_position;

	//view_data_t *view_data;

	//mat4_t current_view_projection_matrix;
	//mat4_t portal_transform;
	//mat4_t model_view_projection_matrix;
	mat4_t view_matrix;
	int i;
	int j;
	int k;


	float portal_near_d;
	float portal_vert_d;

	float qr;
	//float ql;
	float qt;
	float znear;
	//float qb;

	int portal_index;

	float x_max;
	float x_min;
	float y_max;
	float y_min;
	int positive_z;


	//qr = -view_data->view_data.projection_matrix.floats[0][0];
	//qt = -view_data->view_data.projection_matrix.floats[1][1];
	//znear = view_data->view_data.projection_matrix.floats[3][2];


	znear = -active_view->frustum.znear;
	qr = znear / active_view->frustum.right;
	qt = znear / active_view->frustum.top;



	/*
	=====================================================================
	=====================================================================
	=====================================================================
	*/
/*
	viewing_portal_forward_vector.x = viewing_portal->orientation.floats[0][2];
	viewing_portal_forward_vector.y = viewing_portal->orientation.floats[1][2];
	viewing_portal_forward_vector.z = viewing_portal->orientation.floats[2][2];
	viewing_portal_forward_vector.w = 0.0;

	viewing_portal_position.x = viewing_portal->position.x;
	viewing_portal_position.y = viewing_portal->position.y;
	viewing_portal_position.z = viewing_portal->position.z;
	viewing_portal_position.w = 1.0;

	mat4_t_vec4_t_mult(&view_data->view_matrix, &viewing_portal_forward_vector);
	mat4_t_vec4_t_mult(&view_data->view_matrix, &viewing_portal_position);

	portal_near_d = -dot3(viewing_portal_position.vec3, portal_forward_vector.vec3);*/

	/*
	=====================================================================
	=====================================================================
	=====================================================================
	*/


	portal_forward_vector.x = portal->orientation.floats[0][2];
	portal_forward_vector.y = portal->orientation.floats[1][2];
	portal_forward_vector.z = portal->orientation.floats[2][2];
	portal_forward_vector.w = 0.0;

	portal_right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
	portal_right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
	portal_right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;

	portal_up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
	portal_up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
	portal_up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;

	portal_position.x = portal->position.x;
	portal_position.y = portal->position.y;
	portal_position.z = portal->position.z;
	portal_position.w = 1.0;

	portal_verts[0].x = portal_position.x - portal_right_vector.x + portal_up_vector.x;
	portal_verts[0].y = portal_position.y - portal_right_vector.y + portal_up_vector.y;
	portal_verts[0].z = portal_position.z - portal_right_vector.z + portal_up_vector.z;
	portal_verts[0].w = 1.0;

	portal_verts[1].x = portal_position.x - portal_right_vector.x - portal_up_vector.x;
	portal_verts[1].y = portal_position.y - portal_right_vector.y - portal_up_vector.y;
	portal_verts[1].z = portal_position.z - portal_right_vector.z - portal_up_vector.z;
	portal_verts[1].w = 1.0;

	portal_verts[2].x = portal_position.x + portal_right_vector.x - portal_up_vector.x;
	portal_verts[2].y = portal_position.y + portal_right_vector.y - portal_up_vector.y;
	portal_verts[2].z = portal_position.z + portal_right_vector.z - portal_up_vector.z;
	portal_verts[2].w = 1.0;

	portal_verts[3].x = portal_position.x + portal_right_vector.x + portal_up_vector.x;
	portal_verts[3].y = portal_position.y + portal_right_vector.y + portal_up_vector.y;
	portal_verts[3].z = portal_position.z + portal_right_vector.z + portal_up_vector.z;
	portal_verts[3].w = 1.0;



	x_max = -10.0;
	x_min = 10.0;
	y_max = -10.0;
	y_min = 10.0;
	positive_z = 0;

	/* could cache this... */
	mat4_t_vec4_t_mult(&view_data->view_data.view_matrix, &portal_forward_vector);
	mat4_t_vec4_t_mult(&view_data->view_data.view_matrix, &portal_position);

	/* could cache this... */
//	portal_near_d = dot3(portal_position.vec3, portal_forward_vector.vec3);

	//printf("%f\n", portal_near_d);

	for(k = 0; k < 4; k++)
	{
		mat4_t_vec4_t_mult(&view_data->view_data.view_matrix, &portal_verts[k]);
		//portal_vert_d = dot3(portal_verts[k].vec3, viewing_portal_forward_vector.vec3) - portal_near_d;
		//portal_vert_d = -dot3(portal_verts[k].vec3, view_data->near_plane.vec3) - view_data->near_plane.w;

	/*	if(portal_vert_d < 0.0)
		{
			positive_z++;
		}*/

	//	portal_verts[k].x *= qr;
	//	portal_verts[k].y *= qt;

		if(portal_verts[k].z > znear)
		{
			portal_verts[k].z = znear;
			positive_z++;
		}

		portal_verts[k].x = (portal_verts[k].x * qr) / portal_verts[k].z;
		portal_verts[k].y = (portal_verts[k].y * qt) / portal_verts[k].z;


		if(portal_verts[k].x > x_max) x_max = portal_verts[k].x;
		if(portal_verts[k].x < x_min) x_min = portal_verts[k].x;

		if(portal_verts[k].y > y_max) y_max = portal_verts[k].y;
		if(portal_verts[k].y < y_min) y_min = portal_verts[k].y;
	}

	if(x_max > 1.0) x_max = 1.0;
	else if(x_max < -1.0) x_max = -1.0;

	if(x_min > 1.0) x_min = 1.0;
	else if(x_min < -1.0) x_min = -1.0;

	if(y_max > 1.0) y_max = 1.0;
	else if(y_max < -1.0) y_max = -1.0;

	if(y_min > 1.0) y_min = 1.0;
	else if(y_min < -1.0) y_min = -1.0;



	//printf("[%f %f %f]\n", portal_forward_vector.x, portal_forward_vector.y, portal_forward_vector.z);

	if((x_max - x_min) * (y_max - y_min) > 0.0 && positive_z < 4)
	{
		if(dot3(portal_forward_vector.vec3, portal_position.vec3) > 0.0)
		{
			if(view_data->view_data.view_portals_frame != r_frame)
			{
				view_data->view_data.view_portals_frame = r_frame;
				view_data->view_data.view_portals_list_cursor = 0;
			}

			if(view_data->view_data.view_portals_list_cursor >= view_data->view_data.view_portals_list_size)
			{
				portal_indexes = memory_Malloc(sizeof(unsigned short) * (view_data->view_data.view_portals_list_size + 16), "world_PortalOnView");
				memcpy(portal_indexes, view_data->view_data.view_portals, sizeof(unsigned short) * view_data->view_data.view_portals_list_size);
				memory_Free(view_data->view_data.view_portals);
				view_data->view_data.view_portals = portal_indexes;
				view_data->view_data.view_portals_list_size += 16;
			}

			portal_index = portal - ptl_portals;

			view_data->view_data.view_portals[view_data->view_data.view_portals_list_cursor] = portal_index;
			view_data->view_data.view_portals_list_cursor++;

		}

	}

	#endif
}

#endif

#if 0

void world_PortalsOnViews()
{
	camera_t *current_view;
	portal_t *current_portal;
	int i;

	current_view = cameras;
	//active_camera = camera_GetActiveCamera();

	while(current_view)
	{
		if(current_view->bm_flags & CAMERA_INACTIVE)
		{
			current_view = current_view->next;
			continue;
		}


		/* first find which portals can be seen directly
		from the main view... */
		for(i = 0; i < ptl_portal_list_cursor; i++)
		{
			current_portal = &ptl_portals[i];
			world_PortalOnView(&current_view->view_data, current_portal, current_view->world_position);
		}

		//printf("%s: %d\n", current_view->name, current_view->view_data.view_portals_list_cursor);


		/* now, for every directly visible portal... */
		for(i = 0; i < current_view->view_data.view_portals_list_cursor; i++)
		{
			/* find which portals those directly seen portals see... */
			world_PortalsOnPortals(&ptl_portals[current_view->view_data.view_portals[i]], &current_view->world_orientation, current_view->world_position, -1);
		}



		current_view = current_view->next;
	}
}

#endif

/*
==========================================================================
==========================================================================
==========================================================================
*/

#if 0

void world_PortalsOnPortals(portal_t *portal, mat3_t *view_orientation, vec3_t view_position, int viewing_portal_index)
{
	#if 0
	portal_t *current_portal;
	bsp_dleaf_t *portal_leaf;
	portal_recursive_view_data_t *recursive_view_data;
	portal_view_data_t *view_data;
	int i;
	int viewing_portal;
	static int recursion_level = -1;

	/*if(recursion_level >= W_MAX_PORTAL_RECURSION_LEVEL - 1)
	{
		return;
	}

	if()*/

	if(recursion_level >= portal->max_recursion - 1)
	{
		return;
	}

	recursion_level++;

	/* calculate the portal view based on the current view position we're seeing from... */
	if(portal_CalculatePortalView(portal, view_orientation, view_position, recursion_level, viewing_portal_index))
	{
		recursive_view_data = &portal->portal_recursive_views[recursion_level];
		view_data = &recursive_view_data->views[recursive_view_data->views_count - 1];

		/* use this newly calculated view to find out
		which portals this portal see (can include itself)... */
		for(i = 0; i < ptl_portal_list_cursor; i++)
		{
			current_portal = &ptl_portals[i];
			world_PortalOnPortalView(view_data, current_portal);
			//world_PortalOnView(&view_data->view_data, current_portal);
		}

		viewing_portal = portal - ptl_portals;

		/* for every portal seen by this portal... */
		for(i = 0; i < view_data->view_data.view_portals_list_cursor; i++)
		{
			/* find out which portals they see... */
			world_PortalsOnPortals(&ptl_portals[view_data->view_data.view_portals[i]], &view_data->orientation, view_data->position, viewing_portal);
		}
	}

	recursion_level--;

	#endif

}

#endif

#if 0

void world_WorldOnPortals()
{

}

#endif

#if 0

void world_LightsOnPortal(portal_t *portal)
{
	#if 0
	int i;
	int j;
	int k;
	static int recursion_level = -1;

	portal_recursive_view_data_t *recursive_view_data;

	for(i = 0; i < ptl_portal_list_cursor; i++)
	{
		for(j = 0; j < W_MAX_PORTAL_RECURSION_LEVEL; j++)
		{
			recursive_view_data = &portal->portal_recursive_views[j];

			if(recursive_view_data->frame == r_frame)
			{
				for(k = 0; k < recursive_view_data->views_count; k++)
				{
					world_LightsOnView(&recursive_view_data->views[k].view_data);
				}
			}
		}
	}

	#endif

}

#endif

#if 0

void world_EntitiesOnPortals()
{

}

#endif


/*
=================================================
=================================================
=================================================
*/

void world_VisibleEntities()
{

    #if 0
	int i;
	int c;
	int j;

	struct entity_t *entity;

	struct entity_transform_t *world_transforms;
	struct entity_transform_t *world_transform;

	struct entity_aabb_t *aabbs;
	struct entity_aabb_t *aabb;

	struct model_component_t *model_component;
	struct model_t *model;
	struct batch_t *batch;
	struct lod_t *lod;

	camera_t *active_camera = camera_GetActiveCamera();

	vec3_t box[8];

	struct entity_handle_t *visible_entities;
	int visible_entity_count;

	world_transforms = (struct entity_transform_t *)ent_world_transforms.elements;
	aabbs = (struct entity_aabb_t *)ent_entity_aabbs.elements;


	//w_visible_entities.element_count = 0;
	visible_entities = (struct entity_handle_t *)w_visible_entities.elements;
	visible_entity_count = 0;
	c = ent_entities[0].element_count;

	//if(!w_world_)
	for(i = 0; i < c; i++)
	{
		entity = entity_GetEntityPointerIndex(i);

		if(!entity)
		{
			continue;
		}

		if(entity->components[COMPONENT_TYPE_MODEL].type == COMPONENT_TYPE_NONE)
		{
			/* entities without models won't get rendered... */
			continue;
		}

		world_transform = world_transforms + entity->components[COMPONENT_TYPE_TRANSFORM].index;
		aabb = aabbs + entity->components[COMPONENT_TYPE_TRANSFORM].index;

		if(camera_BoxScreenArea(active_camera, vec3_t_c(world_transform->transform.floats[3][0], world_transform->transform.floats[3][1], world_transform->transform.floats[3][2]), aabb->current_extents))
		{
			if(visible_entity_count >= w_visible_entities.max_elements)
			{
				list_resize(&w_visible_entities, w_visible_entities.max_elements + 128);
				visible_entities = (struct entity_handle_t *)w_visible_entities.elements;
			}

			visible_entities[visible_entity_count].def = 0;
			visible_entities[visible_entity_count].entity_index = i;

			visible_entity_count++;
		}
	}

	//printf("%d\n", visible_entity_count);

	for(i = 0; i < visible_entity_count; i++)
	{
		entity = entity_GetEntityPointerHandle(visible_entities[i]);

		model_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_MODEL]);
		world_transform = world_transforms + entity->components[COMPONENT_TYPE_TRANSFORM].index;
		model = model_GetModelPointerIndex(model_component->model_index);

		lod = &model->lods[0];

		for(j = 0; j < model->batch_count; j++)
		{
			batch = lod->batches + j;
			//renderer_SubmitDrawCommand(&world_transform->transform, lod->draw_mode, model->vert_start + batch->start, batch->next, batch->material_index, 0);
			renderer_SubmitDrawCommand(&world_transform->transform, GL_TRIANGLES, lod->lod_indice_buffer_start + batch->start, batch->next, batch->material_index, 1);
		}
	}



	/*int leaf_index;
	int entity_index;

	bsp_dleaf_t *leaf;
	entity_t *entity;
	model_t *model;
	mat4_t *transform;

	ent_visible_entities_count = 0;
	if(!w_world_leaves)
	{
		for(i = 0; i < ent_entity_list_cursor; i++)
		{
			if(ent_entities[i].flags & ENTITY_INVALID)
				continue;

			if(ent_entities[i].flags & ENTITY_INVISIBLE)
				continue;

			ent_visible_entities_indexes[ent_visible_entities_count] = i;
			ent_visible_entities_count++;
		}
		return;
	}

	for(i = 0; i < MAX_ENTITIES >> 5; i++)
	{
		visible_entities.entities[i] = 0;
	}

	for(i = 0; i < w_visible_leaves_count; i++)
	{
		leaf = w_visible_leaves[i];
		leaf_index = leaf - w_world_leaves;

		for(j = 0; j < MAX_ENTITIES >> 5; j++)
		{
			visible_entities.entities[j] |= w_leaf_entities[leaf_index].entities[j];
		}
	}

	for(i = 0; i < MAX_ENTITIES; i++)
	{
		if(visible_entities.entities[i >> 5] & (1 << (i % 32)))
		{
			ent_visible_entities_indexes[ent_visible_entities_count] = i;
			ent_visible_entities_count++;
		}
	}*/

	#endif
}

void world_UploadVisibleLights()
{

    #if 0
	int i;
	int c;
	int light_index;
	int gpu_index = 0;

	float s;
	float e;

	int x;
	int y;
	int z;

	int cluster_x_start;
	int cluster_y_start;
	int cluster_z_start;

	int cluster_x_end;
	int cluster_y_end;
	int cluster_z_end;

	int cluster_index;
	int offset;


	vec4_t light_position;
	light_position_t *pos;
	light_params_t *parms;
	camera_t *active_camera;
	//view_light_t *view_lights;

	struct gpu_light_t *lights;

	if(!l_light_uniform_buffer)
		return;


	R_DBG_PUSH_FUNCTION_NAME();


	float proj_param_a;
	float proj_param_b;



	proj_param_a = l_shadow_map_projection_matrix.floats[2][2];
	proj_param_b = l_shadow_map_projection_matrix.floats[3][2];

	active_camera = camera_GetActiveCamera();
	glBindBuffer(GL_UNIFORM_BUFFER, r_light_uniform_buffer);
	//lights = w_light_buffer;

	//view_lights = view_data->view_lights;

	for(i = 0; i < w_visible_lights_count && i < MAX_VISIBLE_LIGHTS; i++)
	{
		//light_index = view_lights[i].light_index;
		light_index = w_visible_lights[i];

		light_AllocShadowMap(light_index);

		pos = &l_light_positions[light_index];
		parms = &l_light_params[light_index];

		light_position.x = pos->position.x;
		light_position.y = pos->position.y;
		light_position.z = pos->position.z;
		light_position.w = 1.0;

		//mat4_t_vec4_t_mult(&view_data->view_matrix, &light_position);
		mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &light_position);

		w_light_buffer[i].position_radius.x = light_position.x;
		w_light_buffer[i].position_radius.y = light_position.y;
		w_light_buffer[i].position_radius.z = light_position.z;

		w_light_buffer[i].position_radius.w = UNPACK_LIGHT_RADIUS(parms->radius);

		w_light_buffer[i].color_energy.r = (float)parms->r / 255.0;
		w_light_buffer[i].color_energy.g = (float)parms->g / 255.0;
		w_light_buffer[i].color_energy.b = (float)parms->b / 255.0;
		w_light_buffer[i].color_energy.a = UNPACK_LIGHT_ENERGY(parms->energy);

//		w_light_buffer[i].x_y = (parms->y << 16) | parms->x;
		w_light_buffer[i].bm_flags = parms->bm_flags & (~LIGHT_GENERATE_SHADOWS);

		w_light_buffer[i].proj_param_a = proj_param_a;
		w_light_buffer[i].proj_param_b = proj_param_b;
		w_light_buffer[i].shadow_map = parms->shadow_map;

		if(parms->bm_flags & LIGHT_UPLOAD_INDICES)
		{
			if(parms->indices_handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
			{
                parms->indices_handle = gpu_AllocIndexesAlign(sizeof(int) * 8192 * 3, sizeof(int));
				parms->indices_start = gpu_GetAllocStart(parms->indices_handle) / sizeof(int);
			}

            gpu_WriteNonMapped(parms->indices_handle, 0, parms->visible_triangles.elements, sizeof(int) * parms->visible_triangles.element_count);

            parms->bm_flags &= ~LIGHT_UPLOAD_INDICES;
		}
	}

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct gpu_light_t) * MAX_VISIBLE_LIGHTS, w_light_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	/*************************************************************/
	/*************************************************************/
	/*************************************************************/

	#undef CLUSTER_OFFSET

	#define CLUSTER_OFFSET(x, y, z) (x+y*r_clusters_per_row+z*r_clusters_per_row*r_cluster_rows)


	for(z = 0; z < r_cluster_layers; z++)
	{
		for(y = 0; y < r_cluster_rows; y++)
		{
			for(x = 0; x < r_clusters_per_row; x++)
			{
				cluster_index = CLUSTER_OFFSET(x, y, z);
				r_clusters[cluster_index].light_indexes_bm = 0;
			}
		}
	}


	//view_lights = view_data->view_lights;
	//c = view_data->view_lights_list_cursor;

	c = w_visible_lights_count;

	for(i = 0; i < c && i < MAX_VISIBLE_LIGHTS; i++)
	{
		//light_index = view_lights[i].light_index;
		light_index = w_visible_lights[i];
		parms = &l_light_params[light_index];

		//UNPACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end, view_lights[i].view_clusters);
		//UNPACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end, parms->first_cluster);

		cluster_x_start = parms->first_cluster.x;
		cluster_y_start = parms->first_cluster.y;
		cluster_z_start = parms->first_cluster.z;

		cluster_x_end = parms->last_cluster.x;
		cluster_y_end = parms->last_cluster.y;
		cluster_z_end = parms->last_cluster.z;

		for(z = cluster_z_start; z <= cluster_z_end; z++)
		{
			for(y = cluster_y_start; y <= cluster_y_end; y++)
			{
				for(x = cluster_x_start; x <= cluster_x_end; x++)
				{
					cluster_index = CLUSTER_OFFSET(x, y, z);
					r_clusters[cluster_index].light_indexes_bm |= 1 << i;
				}
			}
		}

	}


	/* this is a bottleneck... */
	glBindTexture(GL_TEXTURE_3D, r_cluster_texture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, r_clusters_per_row, r_cluster_rows, r_cluster_layers, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, r_clusters);
	glBindTexture(GL_TEXTURE_3D, 0);

	R_DBG_POP_FUNCTION_NAME();

	#endif
}

void world_VisibleLights()
{
    #if 0
	//printf("light_VisibleLights\n");
	int i;
	int c = w_visible_leaves_count;

	int j;
	int k;
	int leaf_index;
	int int_index;
	int bit_index;

	struct bsp_dleaf_t *leaf;
	struct bsp_dleaf_t *cur_leaf;
	camera_t *active_camera = camera_GetActiveCamera();
	vec4_t light_origin;
	vec3_t v;
	//vec3_t farthest;
	float farthest;
	int farthest_index;
	light_position_t *pos;
	light_params_t *parms;
	bsp_lights_t lights;

	w_visible_lights_count = 0;

	float d;
	float radius;
	float s;
	float e;

	if(!w_world_leaves)
	{
		for(i = 0; i < l_light_list_cursor; i++)
		{
			if(!(l_light_params[i].bm_flags & LIGHT_INVALID))
			{
				w_visible_lights[w_visible_lights_count++] = i;
			}
		}
	}
	else
	{
		s = engine_GetDeltaTime();

		for(i = 0; i < MAX_WORLD_LIGHTS >> 5; i++)
		{
			lights.lights[i] = 0;
		}

		for(i = 0; i < c; i++)
		{
			leaf = w_visible_leaves[i];

			leaf_index = leaf - w_world_leaves;

			for(j = 0; j < MAX_WORLD_LIGHTS >> 5; j++)
			{
				lights.lights[j] |= w_leaf_lights[leaf_index].lights[j];
			}
		}

		for(i = 0; i < MAX_WORLD_LIGHTS; i++)
		{
			if(lights.lights[i >> 5] & (1 << (i % 32)))
			{
				w_visible_lights[w_visible_lights_count++] = i;
			}
		}
	}

	world_LightBounds();


	/* drop far away lights... */
	/* NOTE: quicksorting the lights might scale better... */
	if(w_visible_lights_count > MAX_VISIBLE_LIGHTS)
	{
		//printf("maximum visible lights exceeded! Eliminating...\n");
		c = w_visible_lights_count - MAX_VISIBLE_LIGHTS;

		for(i = 0; i < c; i++)
		{
			farthest = FLT_MIN;

			for(j = 0; j < w_visible_lights_count; j++)
			{
				k = w_visible_lights[j];

				v.x = l_light_positions[k].position.x - active_camera->world_position.x;
				v.y = l_light_positions[k].position.y - active_camera->world_position.y;
				v.z = l_light_positions[k].position.z - active_camera->world_position.z;

				d = dot3(v, v);

				if(d > farthest)
				{
					farthest = d;
					farthest_index = j;
				}
			}

			if(farthest_index < w_visible_lights_count - 1)
			{
				w_visible_lights[farthest_index] = w_visible_lights[w_visible_lights_count - 1];
			}

			w_visible_lights_count--;
			c--;
		}
	}

	world_UploadVisibleLights();

/*	for(i = 0; i < visible_light_count; i++)
	{
		light_CacheLight(visible_lights[i]);
	}

	light_UpdateClusters();
	light_UploadCache();*/

	#endif

}

void world_VisibleLightTriangles(int light_index)
{
    #if 0
	int slot_index;
	int i;
	int c;
	int j;
	int r;
	int first_vertex_index;
	float radius;
	float sqrd_radius;
	float dist;
	light_params_t *parms;
	light_position_t *pos;
//	bsp_striangle_t *visible_triangles;
//	bsp_striangle_t *leaf_triangles;
//	int *visible_tris;
	vertex_t *first_vertex;
	vec3_t v;
	vec3_t p;

	struct list_t *visible_triangles;
	int *visible_triangle_indices;

	//int light_index;

	//vec3_t box_max = {-9999999999.9, -9999999999.9, -9999999999.9};
	//vec3_t box_min = {9999999999.9, 9999999999.9, 9999999999.9};

	//vec3_t close_max = {-9999999999.9, -9999999999.9, -9999999999.9};
	//vec3_t close_min = {9999999999.9, 9999999999.9, 9999999999.9};


	struct bsp_dleaf_t *leaf;

	if(!w_world_leaves)
		return;


	int frustums;


	if(light_index >= 0 && light_index <= l_light_list_cursor)
	{
		if(!(l_light_params[light_index].bm_flags & LIGHT_INVALID))
		{
			parms = &l_light_params[light_index];
			pos = &l_light_positions[light_index];
			leaf = (struct bsp_dleaf_t *)parms->leaf;

			//parms->visible_triangles.element_count = 0;

			visible_triangles = &parms->visible_triangles;
			visible_triangle_indices = (int *)visible_triangles->elements;

			visible_triangles->element_count = 0;

			if(leaf)
			{
				radius = UNPACK_LIGHT_RADIUS(parms->radius);
				sqrd_radius = radius * radius;

				//printf("update visible triangles\n");

				//parms->visible_triangle_count = 0;
				parms->bm_flags |= LIGHT_UPLOAD_INDICES | LIGHT_UPDATE_SHADOW_MAP;

				for(r = 0; r < w_world_leaves_count; r++)
				{
					if(w_leaf_lights[r].lights[light_index >> 5] & (1 << (light_index % 32)))
					{
						leaf = &w_world_leaves[r];

						c = leaf->tris_count;

						for(i = 0; i < c; i++)
						{
							first_vertex = &w_world_vertices[leaf->tris[i].first_vertex];

							for(j = 0; j < 3; j++)
							{
								v.x = first_vertex[j].position.x - pos->position.x;
								v.y = first_vertex[j].position.y - pos->position.y;
								v.z = first_vertex[j].position.z - pos->position.z;

								dist = dot3(v, v);

								if(dist < sqrd_radius)
								{

                                    if(visible_triangles->element_count >= visible_triangles->max_elements)
									{
                                        list_resize(visible_triangles, visible_triangles->max_elements + 128);
                                        visible_triangle_indices = (int *)visible_triangles->elements;
									}

									visible_triangle_indices[visible_triangles->element_count] = w_world_start + leaf->tris[i].first_vertex;
									visible_triangles->element_count++;

									visible_triangle_indices[visible_triangles->element_count] = w_world_start + leaf->tris[i].first_vertex + 1;
									visible_triangles->element_count++;

									visible_triangle_indices[visible_triangles->element_count] = w_world_start + leaf->tris[i].first_vertex + 2;
									visible_triangles->element_count++;

									break;
								}

							}
						}
					}
				}



			}

			//printf("%d triangles\n", visible_triangles->element_count);
		}
	}

	#endif
}


void world_VisibleLeaves()
{
    #if 0
	camera_t *active_camera = camera_GetActiveCamera();
	w_visible_leaves = bsp_PotentiallyVisibleLeaves(&w_visible_leaves_count, active_camera->world_position);
	#endif
}



void world_UploadWorldBspNodes()
{
    /*glBindBuffer(GL_UNIFORM_BUFFER, r_bsp_uniform_buffer);

    int i;

    for(i = 0; i < w_world_nodes_count; i++)
	{
		w_bsp_buffer[i].normal_dist.x = w_world_nodes[i].normal.x;
		w_bsp_buffer[i].normal_dist.y = w_world_nodes[i].normal.y;
		w_bsp_buffer[i].normal_dist.z = w_world_nodes[i].normal.z;

		w_bsp_buffer[i].normal_dist.w = w_world_nodes[i].dist;

		w_bsp_buffer[i].children[0] = w_world_nodes[i].child[0];
		w_bsp_buffer[i].children[0] = w_world_nodes[i].child[0];
	}


	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct gpu_bsp_node_t) * W_MAX_BSP_NODES, w_bsp_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);*/
}


void world_VisibleWorld()
{
    #if 0

	int i;
	int c;
	int j;

	int start;
	int next;
	int leaf_index;
	int positive_z;

	camera_t *active_camera = camera_GetActiveCamera();

	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	float qt = nznear / ntop;
	float qr = nznear / nright;
	float x_max;
	float x_min;
	float y_max;
	float y_min;

	float s;
	float e;

	struct bsp_dleaf_t *leaf;
	//triangle_group_t *group;
	struct batch_t *batch;
	struct bsp_striangle_t *triangle;
	//bsp_dleaf_t **visible;
	//int visible_count;
	unsigned int *indexes;

	vec4_t corners[8];

	if(!w_world_nodes)
		return;

	//s = engine_GetDeltaTime();

	//visible_leaves_count = 0;
	//leaf = bsp_GetCurrentLeaf(world_nodes, active_camera->world_position);
	w_visible_leaves = bsp_PotentiallyVisibleLeaves(&w_visible_leaves_count, active_camera->world_position);

	/*e = engine_GetDeltaTime();

	printf("%f\n", e - s);*/

	if(w_visible_leaves)
	{
		/* zero out the next index of every world batch... */
		for(i = 0; i < w_world_batch_count; i++)
		{
			w_world_batches[i].next = 0;
		}

		for(j = 0; j < w_visible_leaves_count; j++)
		{
			leaf = w_visible_leaves[j];

			corners[0].x = leaf->center.x - leaf->extents.x;
			corners[0].y = leaf->center.y + leaf->extents.y;
			corners[0].z = leaf->center.z + leaf->extents.z;
			corners[0].w = 1.0;

			corners[1].x = leaf->center.x - leaf->extents.x;
			corners[1].y = leaf->center.y - leaf->extents.y;
			corners[1].z = leaf->center.z + leaf->extents.z;
			corners[1].w = 1.0;

			corners[2].x = leaf->center.x + leaf->extents.x;
			corners[2].y = leaf->center.y - leaf->extents.y;
			corners[2].z = leaf->center.z + leaf->extents.z;
			corners[2].w = 1.0;

			corners[3].x = leaf->center.x + leaf->extents.x;
			corners[3].y = leaf->center.y + leaf->extents.y;
			corners[3].z = leaf->center.z + leaf->extents.z;
			corners[3].w = 1.0;



			corners[4].x = leaf->center.x - leaf->extents.x;
			corners[4].y = leaf->center.y + leaf->extents.y;
			corners[4].z = leaf->center.z - leaf->extents.z;
			corners[4].w = 1.0;

			corners[5].x = leaf->center.x - leaf->extents.x;
			corners[5].y = leaf->center.y - leaf->extents.y;
			corners[5].z = leaf->center.z - leaf->extents.z;
			corners[5].w = 1.0;

			corners[6].x = leaf->center.x + leaf->extents.x;
			corners[6].y = leaf->center.y - leaf->extents.y;
			corners[6].z = leaf->center.z - leaf->extents.z;
			corners[6].w = 1.0;

			corners[7].x = leaf->center.x + leaf->extents.x;
			corners[7].y = leaf->center.y + leaf->extents.y;
			corners[7].z = leaf->center.z - leaf->extents.z;
			corners[7].w = 1.0;

			x_max = -10.9;
			x_min = 10.9;

			y_max = -10.9;
			y_min = 10.9;

			positive_z = 0;
			for(i = 0; i < 8; i++)
			{
				mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &corners[i]);
				if(corners[i].z > nznear)
				{
					corners[i].z = nznear;
					positive_z++;
				}

				corners[i].x = (corners[i].x * qr) / corners[i].z;
				corners[i].y = (corners[i].y * qt) / corners[i].z;

				if(corners[i].x > x_max) x_max = corners[i].x;
				if(corners[i].x < x_min) x_min = corners[i].x;

				if(corners[i].y > y_max) y_max = corners[i].y;
				if(corners[i].y < y_min) y_min = corners[i].y;

			}


			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;

			if(x_min > 1.0) x_min = 1.0;
			else if(x_min < -1.0) x_min = -1.0;

			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;

			if(y_min > 1.0) y_min = 1.0;
			else if(y_min < -1.0) y_min = -1.0;


			if((x_max - x_min) * (y_max - y_min) <= 0.0 || positive_z == 8)
			{
				if(j < w_visible_leaves_count - 1)
				{
					w_visible_leaves[j] = w_visible_leaves[w_visible_leaves_count - 1];
					j--;
				}
				w_visible_leaves_count--;
			}

		}

		/* for each leaf on the list... */
		for(j = 0; j < w_visible_leaves_count; j++)
		{
			leaf = w_visible_leaves[j];

			leaf->visible_frame = r_frame;

			c = leaf->tris_count;

			/* add it's triangles for rendering... */
			for(i = 0; i < c; i++)
			{
				triangle = &leaf->tris[i];
				batch = &w_world_batches[triangle->batch];

				start = batch->start;
				next = batch->next;

				w_index_buffer[start + next	] = w_world_start + triangle->first_vertex;
				w_index_buffer[start + next + 1] = w_world_start + triangle->first_vertex + 1;
				w_index_buffer[start + next + 2] = w_world_start + triangle->first_vertex + 2;

				batch->next += 3;
			}
		}

		gpu_WriteNonMapped(w_world_index_handle, 0, w_index_buffer, sizeof(int) * w_world_vertices_count);
	}

	#endif

}








struct world_var_t *world_AddWorldVar(char *name, int size)
{
	struct world_var_t *world_var;
	struct world_var_t *world_vars;
	void *old_value;
	int world_var_index;
	int i;
	int c;

	if(size <= 0)
	{
		return NULL;
	}

	world_var = world_GetWorldVarPointer(name);

    if(!world_var)
	{
	    /* create if non-existant... */
		world_var_index = list_add(&w_world_vars, NULL);
		world_var = list_get(&w_world_vars, world_var_index);

		memset(world_var, 0, sizeof(struct world_var_t));

        world_var->name = memory_Strdup(name);
        world_var->element_size = size;
        world_var->value = memory_Malloc(size);
	}
	else
    {
        if(world_var->element_size < size)
        {
            /* resize if requested size is larger... */
            old_value = world_var->value;
            world_var->value = memory_Malloc(size);
            memcpy(world_var->value, old_value, world_var->element_size);
            memory_Free(old_value);
            world_var->element_size = size;
        }
    }

	return world_var;
}

struct world_var_t *world_AddWorldArrayVar(char *name, int elem_size, int max_elements)
{
    struct world_var_t *world_var;

    int elem_count = 0;

    world_var = world_GetWorldVarPointer(name);

    if(world_var)
    {
        /* if this var already exists, save it's element count in
        case it's being resized... */
        elem_count = world_var->element_count;
    }

    world_var = world_AddWorldVar(name, elem_size * max_elements);

    /* adjust element size for array vars... */
    world_var->element_size = elem_size;
    world_var->max_elements = max_elements;
    world_var->element_count = elem_count;

    return world_var;
}

void world_RemoveWorldVar(char *name)
{
	struct world_var_t *world_var;
	struct world_var_t *first_world_var;
	int world_var_index;

	world_var = world_GetWorldVarPointer(name);

	if(world_var)
	{
        first_world_var = list_get(&w_world_vars, 0);
		world_var_index = world_var - first_world_var;

		memory_Free(world_var->name);
		memory_Free(world_var->value);

		list_remove(&w_world_vars, world_var_index);
	}
}




struct world_var_t *world_GetWorldVarPointer(char *name)
{
	struct world_var_t *world_var;
	struct world_var_t *world_vars;

	int i;
	int c;


	world_vars = (struct world_var_t *)w_world_vars.elements;
	c = w_world_vars.element_count;

    for(i = 0; i < c; i++)
	{
		if(!strcmp(name, world_vars[i].name))
		{
			return world_vars + i;
		}
	}

	return NULL;
}




void world_WorldVarValue(char *name, void *value, int set)
{
	struct world_var_t *world_var;

	world_var = world_GetWorldVarPointer(name);

	if(world_var)
	{
        if(set)
		{
			memcpy(world_var->value, value, world_var->element_size);
		}
		else
		{
            memcpy(value, world_var->value, world_var->element_size);
		}
	}
    else
    {
        printf("world_WorldVarValue: no var named [%s]\n", name);
    }
}

void world_WorldArrayVarValue(char *name, void *value, int index, int set)
{
    struct world_var_t *world_var;
    int offset;

    world_var = world_GetWorldVarPointer(name);

    if(world_var)
    {
        if(index < 0)
        {
            index = world_var->element_count;
            world_var->element_count++;
        }



        if(index >= world_var->max_elements)
        {
        	if(set)
			{
				/* if index is out of bounds, resize the var to avoid the user having to explicitly do it... */
				world_var = world_AddWorldArrayVar(name, world_var->element_size, (index + 32 + 3) & (~3));
			}
			else
			{
				return;
			}

        }

        offset = world_var->element_size * index;

        if(set)
        {
            memcpy((char *)world_var->value + offset, value, world_var->element_size);
        }
        else
        {
            memcpy(value, (char *)world_var->value + offset, world_var->element_size);
        }
    }

}



void world_SetWorldVarValue(char *name, void *value)
{
	world_WorldVarValue(name, value, 1);
}

void world_GetWorldVarValue(char *name, void *value)
{
	world_WorldVarValue(name, value, 0);
}

void world_SetWorldArrayVarValue(char *name, void *value, int index)
{
    world_WorldArrayVarValue(name, value, index, 1);
}

void world_GetWorldArrayVarValue(char *name, void *value, int index)
{
    world_WorldArrayVarValue(name, value, index, 0);
}

void world_AppendWorldArrayVarValue(char *name, void *value)
{
    world_WorldArrayVarValue(name, value, -1, 1);
}

void world_ClearWorldArrayVar(char *name)
{
    struct world_var_t *world_var;

    world_var = world_GetWorldVarPointer(name);

    if(world_var)
	{
		world_var->element_count = 0;
	}
}







struct world_script_t *world_LoadScript(char *script_name)
{
    return (struct world_script_t *)script_LoadScript(script_name);
}


void world_SetWorldScript(struct world_script_t *world_script)
{
    w_world_script = world_script;

    if(world_script)
    {
        w_execute_on_map_enter = 1;
    }
}

struct world_script_t *world_GetWorldScript()
{
	return w_world_script;
}

void world_ExecuteWorldScript()
{
    int i;

    if(w_world_script)
    {
        script_ExecuteScriptImediate((struct script_t *)w_world_script, NULL);
    }
}

struct world_event_t *world_GetEventPointer(char *event_name)
{
	int i;

	if(w_world_script)
	{
        for(i = 0; i < w_world_script->event_count; i++)
		{
			if(!strcmp(event_name, w_world_script->events[i].event_name))
			{
				return w_world_script->events + i;
			}
		}
	}

	return NULL;
}

void world_CallEvent(char *event_name)
{
	int i;

	struct world_event_t *event;

	if(w_world_script)
	{
		event = world_GetEventPointer(event_name);

		if(event)
		{
			event->executing = 1;

			//printf("world_CallEvent: event [%s] called\n", event->event_name);
		}
	}
}

void world_CallEventIndex(int event_index)
{
	if(w_world_script)
	{
		if(event_index >= 0 && event_index < w_world_script->event_count)
		{
            w_world_script->events[event_index].executing = 1;

            //printf("world_CallEventIndex: event [%s] called\n", w_world_script->events[event_index].event_name);
		}
	}
}

void world_StopEvent(char *event_name)
{
    int i;

    struct world_event_t *event;

	if(w_world_script)
	{
		event = world_GetEventPointer(event_name);

		if(event)
		{
			event->executing = 0;

			//printf("world_StopEvent: event [%s] stopped\n", event->event_name);
		}
	}
}

void world_StopEventIndex(int event_index)
{
	if(w_world_script)
	{
		if(event_index >= 0 && event_index < w_world_script->event_count)
		{
            w_world_script->events[event_index].executing = 0;

            //printf("world_StopEventIndex: event [%s] stopped\n", w_world_script->events[event_index].event_name);
		}
	}
}

void world_StopAllEvents()
{
	int i;

    if(w_world_script)
	{
		for(i = 0; i < w_world_script->event_count; i++)
		{
            w_world_script->events[i].executing = 0;
		}
	}
}

void world_FadeIn()
{

}

int world_HasFadedIn()
{

}

void world_FadeOut()
{

}

int world_HasFadedOut()
{

}

void world_ClearBsp()
{
	if(w_world_leaves)
	{
		renderer_Free(w_world_handle);
		renderer_Free(w_world_index_handle);
		memory_Free(w_world_vertices);
		memory_Free(w_world_nodes);
		memory_Free(w_world_leaves);
		memory_Free(w_world_batches);
		memory_Free(w_world_z_batches);
		memory_Free(w_index_buffer);

		w_world_handle = INVALID_GPU_ALLOC_HANDLE;
		w_world_index_handle = INVALID_GPU_ALLOC_HANDLE;
		w_index_buffer = NULL;

		w_world_batches = NULL;
		w_world_batch_count = 0;

		w_world_z_batches = NULL;
		w_world_batch_count = 0;

		w_world_vertices_count = 0;
		w_world_vertices = NULL;

		w_world_nodes_count = 0;
		w_world_nodes = NULL;

		w_world_leaves_count = 0;
		w_world_leaves = NULL;

		w_max_visible_batches = 0;
		w_max_visible_indexes = 0;

		w_need_to_clear_world = 0;
	}
}

void world_Update()
{
	int i;
	int j;
	int c;
	int k;

	struct bsp_dleaf_t *leaf;

	int total_batches = 0;
	int cur_group_index;
	int cur_batch_index;

	struct compact_vertex_t *compact_world_vertices;


	if(!w_world_leaves)
		return;

	if(w_need_to_clear_world)
	{
		world_Clear(WORLD_CLEAR_FLAG_PHYSICS_MESH | WORLD_CLEAR_FLAG_LIGHT_LEAVES | WORLD_CLEAR_FLAG_BSP);
	}

	w_leaf_lights = memory_Malloc(sizeof(bsp_lights_t ) * w_world_leaves_count);
	w_index_buffer = memory_Malloc(sizeof(unsigned int ) * w_world_vertices_count);
	w_sorted_index_buffer = memory_Malloc(sizeof(unsigned int ) * w_world_vertices_count);

	w_world_z_batches = memory_Calloc(w_world_leaves_count, sizeof(struct batch_t));

	for(i = 0; i < w_world_leaves_count; i++)
	{
		for(j = 0; j < MAX_WORLD_LIGHTS >> 5; j++)
		{
			w_leaf_lights[i].lights[j] = 0;
		}
	}

//	if(r_world_vertices_buffer)
//    {
//        memory_Free(r_world_vertices_buffer);
//    }

	//r_world_vertices_buffer = memory_Malloc(sizeof(vec4_t) * w_world_vertices_count);

	w_world_handle = renderer_AllocVerticesAlign(sizeof(struct compact_vertex_t) * w_world_vertices_count, sizeof(struct compact_vertex_t));
	w_world_start = renderer_GetAllocStart(w_world_handle) / sizeof(struct compact_vertex_t);

	w_world_index_handle = renderer_AllocIndexesAlign(sizeof(int) * w_world_vertices_count, sizeof(int));
	w_world_index_start = renderer_GetAllocStart(w_world_index_handle) / sizeof(int);

    w_world_sorted_index_handle = renderer_AllocIndexesAlign(sizeof(int) * w_world_vertices_count, sizeof(int));
    w_world_sorted_index_start = renderer_GetAllocStart(w_world_sorted_index_handle) / sizeof(int);


//    glBindBuffer(GL_UNIFORM_BUFFER, r_world_vertices_uniform_buffer);
//	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4_t) * w_world_vertices_count, NULL, GL_DYNAMIC_DRAW);
//	glBindBuffer(GL_UNIFORM_BUFFER, 0);

    renderer_ResizeWorldTrianglesUniformBuffer(w_world_vertices_count / 3);

	compact_world_vertices = model_ConvertVertices(w_world_vertices, w_world_vertices_count);
	renderer_Write(w_world_handle, 0, compact_world_vertices, sizeof(struct compact_vertex_t) * w_world_vertices_count);

	physics_BuildWorldCollisionMesh();
	memory_Free(compact_world_vertices);

	world_UploadWorldBspNodes();

	w_need_to_clear_world = 1;

}

void world_Clear(int clear_flags)
{

	if(clear_flags & WORLD_CLEAR_FLAG_LIGHT_LEAVES)
	{
		light_ClearLightLeaves();
	}

    if(clear_flags & WORLD_CLEAR_FLAG_LIGHTS)
	{
		light_DestroyAllLights();
	}

	if(clear_flags & WORLD_CLEAR_FLAG_ENTITIES)
	{
        entity_RemoveAllEntities(0);
	}

	if(clear_flags & WORLD_CLEAR_FLAG_ENTITY_DEFS)
    {
        entity_RemoveAllEntities(1);
    }

	if(clear_flags & WORLD_CLEAR_FLAG_PHYSICS_MESH)
	{
		physics_ClearWorldCollisionMesh();
	}

	if(clear_flags & WORLD_CLEAR_FLAG_WAYPOINTS)
    {
        navigation_DestroyAllWaypoints();
    }

    if(clear_flags & WORLD_CLEAR_FLAG_BSP)
	{
		world_ClearBsp();
	}
}

struct world_level_t *world_CreateLevel(char *level_name, struct serializer_t serializer, struct world_script_t *world_script)
{
    struct world_level_t *level;

    level = memory_Calloc(1, sizeof(struct world_level_t ));

    level->level_name = memory_Strdup(level_name);
    level->script = world_script;
    level->serializer = serializer;

    if(!w_levels)
    {
        w_levels = level;
    }
    else
    {
        w_last_level->next = level;
        level->prev = w_last_level;
    }

    w_last_level = level;

    return level;
}

void world_DestroyLevel(char *level_name)
{
    struct world_level_t *level;

    level = world_GetLevel(level_name);

    if(level)
    {
        serializer_FreeSerializer(&level->serializer, 0);

        memory_Free(level->level_name);

        if(level->script)
        {
            script_DestroyScript((struct script_t *)&level->script);
        }

        if(level == w_levels)
        {
            w_levels = level->next;

            if(w_levels)
            {
                w_levels->prev = NULL;
            }
        }
        else
        {
            level->prev->next = level->next;

            if(level->next)
            {
                level->next->prev = level->prev;
            }
            else
            {
                w_last_level = level->prev;
            }
        }

        memory_Free(level);
    }
}

struct world_level_t *world_GetLevel(char *level_name)
{
    struct world_level_t *level;

    level = w_levels;

    while(level)
    {
        if(!strcmp(level_name, level->level_name))
        {
            break;
        }

        level = level->next;
    }

    return level;
}

struct world_level_t *world_GetCurrentLevel()
{
    return w_current_level;
}

void world_ChangeLevel(char *level_name)
{
    struct world_level_t *level;
    struct serializer_entry_t *entry;
    void *read_buffer;

    level = world_GetLevel(level_name);

    if(!level)
    {
        level = world_LoadLevel(level_name);
    }

    //if(level != w_current_level)
    {
        world_UnloadCurrentLevel();

        w_current_level = level;

        entry = serializer_GetEntry(&level->serializer, "materials");
        if(entry)
        {
            read_buffer = entry->entry_buffer;
            material_DeserializeMaterials(&read_buffer);
        }


        entry = serializer_GetEntry(&level->serializer, "waypoints");
        if(entry)
        {
            read_buffer = entry->entry_buffer;
            navigation_DeserializeWaypoints(&read_buffer);
        }


        entry = serializer_GetEntry(&level->serializer, "lights");
        if(entry)
        {
            read_buffer = entry->entry_buffer;
            light_DeserializeLights(&read_buffer);
        }


        entry = serializer_GetEntry(&level->serializer, "bsp");
        if(entry)
        {
            read_buffer = entry->entry_buffer;
            bsp_DeserializeBsp(&read_buffer);
        }


        entry = serializer_GetEntry(&level->serializer, "entity defs");
        if(entry)
        {
            read_buffer = entry->entry_buffer;
            entity_DeserializeEntities(&read_buffer, 1);
        }


        entry = serializer_GetEntry(&level->serializer, "entities");
        if(entry)
        {
            read_buffer = entry->entry_buffer;
            entity_DeserializeEntities(&read_buffer, 0);
        }

        world_SetWorldScript(level->script);


//        navigation_DeserializeWaypoints(serializer_GetEntry(&level->serializer, "waypoints")->entry_buffer);
//        light_DeserializeLights(serializer_GetEntry(&level->serializer, "lights")->entry_buffer);
//        bsp_DeserializeBsp(serializer_GetEntry(&level->serializer, "bsp")->entry_buffer);
//        entity_DeserializeEntities(serializer_GetEntry(&level->serializer, "entity defs")->entry_buffer, 1);
//        entity_DeserializeEntities(serializer_GetEntry(&level->serializer, "entities")->entry_buffer, 0);
    }
}

struct world_level_t *world_LoadLevel(char *level_name)
{
    struct world_script_t *world_script;
    char world_script_full_name[512];
    char *world_script_name;

    struct world_level_t *level = NULL;
    struct serializer_t serializer;

    void *world_buffer = NULL;
    void *read_buffer;
    int world_buffer_size = 0;

    FILE *file;

    //void *world_bsp;

    level = world_GetLevel(level_name);

    if(level)
    {
        return level;
    }

    if(path_FileExists(level_name))
    {
        file = path_TryOpenFile(level_name);

        if(file)
        {
            world_buffer_size = path_GetFileSize(file);

            world_buffer = memory_Calloc(1, world_buffer_size);
            fread(world_buffer, 1, world_buffer_size, file);
            fclose(file);

            read_buffer = world_buffer;

            level = world_LoadLevelFromMemory(level_name, &read_buffer);

            memory_Free(world_buffer);
        }
    }

    return level;
}

struct world_level_t *world_LoadLevelFromMemory(char *level_name, void **buffer)
{
    struct serializer_t serializer;
    char *world_script_name;
    char world_script_full_name[512];
    struct world_script_t *world_script;
    struct world_level_t *level;

    serializer = world_DeserializeWorld(buffer);

    world_script_name = path_GetNameNoExt(level_name);
    strcpy(world_script_full_name, world_script_name);
    strcat(world_script_full_name, ".was");

    world_script = (struct world_script_t *)script_GetScript(world_script_full_name);

    if(!world_script)
    {
        world_script = world_LoadScript(world_script_full_name);
    }

    level = world_CreateLevel(level_name, serializer, world_script);
}


void world_UnloadCurrentLevel()
{
    world_Clear(WORLD_CLEAR_FLAG_ALL);
}

void world_SerializeWorld(void **buffer, int *buffer_size)
{
    struct world_record_start_t *record_start;
    struct world_record_end_t *record_end;

    struct serializer_t serializer;

    void *entity_buffer = NULL;
    int entity_buffer_size = 0;

    void *entity_def_buffer = NULL;
    int entity_def_buffer_size = 0;

    void *waypoint_buffer = NULL;
    int waypoint_buffer_size = 0;

    void *light_buffer = NULL;
    int light_buffer_size = 0;

    void *material_buffer = NULL;
    int material_buffer_size = 0;

    void *bsp_buffer = NULL;
    int bsp_buffer_size = 0;

    char *out = NULL;
    int out_size = 0;

    void *level_buffer = NULL;
    int level_buffer_size = 0;


    entity_SerializeEntities(&entity_buffer, &entity_buffer_size, 0);
    entity_SerializeEntities(&entity_def_buffer, &entity_def_buffer_size, 1);
    light_SerializeLights(&light_buffer, &light_buffer_size);
    navigation_SerializeWaypoints(&waypoint_buffer, &waypoint_buffer_size);
    material_SerializeMaterials(&material_buffer, &material_buffer_size);
    bsp_SerializeBsp(&bsp_buffer, &bsp_buffer_size);

    memset(&serializer, 0, sizeof(struct serializer_t));

    serializer_AddEntry(&serializer, "entity defs", entity_def_buffer_size, entity_def_buffer);
    serializer_AddEntry(&serializer, "entities", entity_buffer_size, entity_buffer);
    serializer_AddEntry(&serializer, "lights", light_buffer_size, light_buffer);
    serializer_AddEntry(&serializer, "waypoints", waypoint_buffer_size, waypoint_buffer);
    serializer_AddEntry(&serializer, "materials", material_buffer_size, material_buffer);
    serializer_AddEntry(&serializer, "bsp", bsp_buffer_size, bsp_buffer);

    serializer_Serialize(&serializer, &level_buffer, &level_buffer_size);
    serializer_FreeSerializer(&serializer, 1);

    out_size = level_buffer_size + sizeof(struct world_record_start_t ) + sizeof(struct world_record_end_t);

    out = memory_Calloc(1, out_size);

    *buffer = out;
    *buffer_size = out_size;

    record_start = (struct world_record_start_t *)out;
    out += sizeof(struct world_record_start_t );

    strcpy(record_start->tag, world_record_start_tag);

    memcpy(out, level_buffer, level_buffer_size);
    out += level_buffer_size;

    record_end = (struct world_record_end_t *)out;
    out += sizeof(struct world_record_end_t);

    strcpy(record_end->tag, world_record_end_tag);
}

struct serializer_t world_DeserializeWorld(void **buffer)
{
    struct world_record_start_t *record_start;
    struct world_record_end_t *record_end;

    struct serializer_t serializer;

    char *in;


    in = *(char **)buffer;

    if(in)
    {
        while(1)
        {
            if(!strcmp(in, world_record_start_tag))
            {
                in += sizeof(struct world_record_start_t);
                serializer_Deserialize(&serializer, (void **)&in);
                continue;
            }
            else if(!strcmp(in, world_record_end_tag))
            {
                in += sizeof(struct world_record_end_t);
                break;
            }

            in++;
        }

        *buffer = in;
    }

    return serializer;
}


#ifdef __cplusplus
}
#endif













