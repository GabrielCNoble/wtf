#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "ent_serialization.h"

#include "GL\glew.h"

#include "bsp.h"
//#include "brush.h"
#include "w_common.h"
#include "world.h"
#include "camera.h"
#include "material.h"
#include "shader.h"
#include "l_main.h"
#include "input.h"
#include "c_memory.h"
#include "path.h"
#include "log.h"
#include "navigation.h"

/* from world.c */
extern int w_world_vertices_count;
extern vertex_t *w_world_vertices;

extern int w_world_triangle_count;

extern int w_world_nodes_count;
extern struct bsp_pnode_t *w_world_nodes;

extern int w_world_leaves_count;
extern struct bsp_dleaf_t *w_world_leaves;

extern int w_world_batch_count;
extern struct batch_t *w_world_batches;

extern int w_world_index_count;

extern struct world_script_t *w_world_script;

//extern int w_world_triangle_group_count;
//extern triangle_group_t *w_world_triangle_groups;
//extern unsigned int *w_index_buffer;
//extern unsigned int w_world_element_buffer;
extern int w_world_handle;

//extern bsp_leaf_t *world_leaves;

//int draw_bsp_shader;

//bsp_pnode_t *entity_hull = NULL;


/* from light.c */
//extern int visible_light_count;



float color_table[][3] = {0.0, 0.8, 0.0,
						  0.0, 0.8, 0.14,
						  0.0, 0.8, 0.59,
						  0.0, 0.52, 0.8,
						  0.0, 0.1, 0.8,
						  0.0, 0.0, 0.8,
						  0.14, 0.0, 0.8,
						  0.46, 0.0, 0.8,
						  0.8, 0.0, 0.69,
						  0.8, 0.0, 0.21,
						  0.8, 0.0, 0.0,
						  0.8, 0.1, 0.0,
						  0.8, 0.4, 0.0,
						  0.72, 0.8, 0.0,
						  0.35, 0.8, 0.0,
						  0.0, 0.8, 0.0};


#ifdef __cplusplus
extern "C"
{
#endif

int bsp_Init()
{
	return 1;
}

void bsp_Finish()
{
	//bsp_DeleteSolidLeaf(world_bsp);
	//bsp_DeleteBsp();
}


void bsp_DeleteBsp()
{

}

/*
==============
bsp_ClipVelocityToPlane
==============
*/
void bsp_ClipVelocityToPlane(vec3_t normal, vec3_t velocity, vec3_t *new_velocity, float overbounce)
{
	float l;

	l = dot3(normal, velocity) * overbounce;

	normal.x *= l;
	normal.y *= l;
	normal.z *= l;

	new_velocity->x = velocity.x - normal.x;
	new_velocity->y = velocity.y - normal.y;
	new_velocity->z = velocity.z - normal.z;

	//printf("[%f %f %f]     [%f %f %f]\n", velocity.x, velocity.y, velocity.z, normal.x, normal.y, normal.z);

}



/*
==============
bsp_HullForEntity
==============
*/
struct bsp_pnode_t *bsp_HullForEntity(vec3_t mins, vec3_t max)
{

}


/*
==============
bsp_SolidPoint
==============
*/
int bsp_SolidPoint(struct bsp_pnode_t *node, vec3_t point)
{
	vec3_t v;
	float d;
	int child_index;

	while(node->child[0] > BSP_EMPTY_LEAF && node->child[0] < BSP_SOLID_LEAF)
	{
		d = dot3(node->normal, point) - node->dist;

		if(d >= 0.0)
		{
			child_index = 0;
		}
		else
		{
			child_index = 1;
		}
		node += node->child[child_index];
	}

	return node->child[0];

}




#define DIST_EPSILON 0.03125
/*
==============
bsp_RecursiveFirstHit
==============
*/
int bsp_RecursiveFirstHit(struct bsp_pnode_t *node, vec3_t *start, vec3_t *end, float t0, float t1, struct trace_t *trace)
{
	float d0;
	float d1;

	float frac;
	float frac2;
	float midf;
	float midf2;

	int near_index;

	vec3_t v;
	vec3_t mid;
	vec3_t mid2;

	struct bsp_dleaf_t *leaf;

	if(node->child[0] == BSP_EMPTY_LEAF || node->child[0] == BSP_SOLID_LEAF)
	{
		if(node->child[0] == BSP_EMPTY_LEAF)
		{
			trace->bm_flags &= ~TRACE_ALL_SOLID;
			trace->leaf = w_world_leaves + *(int *)&node->dist;
		}
		else
		{
			/* we reached a solid leaf, which means this
			is the start point, and we're inside solid world... */
			trace->bm_flags |= TRACE_START_SOLID;
		}

		return 1;
	}

	else
	{
		d0 = dot3(*start, node->normal) - node->dist;
		d1 = dot3(*end, node->normal) - node->dist;


		if(d0 >= 0.0 && d1 >= 0.0)
		{
			return bsp_RecursiveFirstHit(node + node->child[0], start, end, t0, t1, trace);
		}
		else if(d0 < 0.0 && d1 < 0.0)
		{
			return bsp_RecursiveFirstHit(node + node->child[1], start, end, t0, t1, trace);
		}


		if(d0 < 0.0)
		{
			/* nudge the intersection away from the plane... */
			frac = (d0 + DIST_EPSILON) / (d0 - d1);
			near_index = 1;

		}
		else
		{
			frac = (d0 - DIST_EPSILON) / (d0 - d1);
			near_index = 0;
		}


		if(frac > 1.0) frac = 1.0;
		else if(frac < 0.0) frac = 0.0;


		midf = t0 + (t1 - t0) * frac;
		mid.x = start->x + (end->x - start->x) * frac;
		mid.y = start->y + (end->y - start->y) * frac;
		mid.z = start->z + (end->z - start->z) * frac;

		if(!bsp_RecursiveFirstHit(node + node->child[near_index], start, &mid, t0, midf, trace))
		{
			return 0;
		}

		/* test to see whether the mid point would fall in solid space in case we were
		to thread down. If not, then the second half of the segment lies in empty space,
		and can further straddle other planes. If it falls in solid space, it means
		that frac is the nearest intersection, and we're done. */

		/* thank you John :) */
		if(bsp_SolidPoint(node + node->child[near_index ^ 1], mid) != BSP_SOLID_LEAF)
		{
			return bsp_RecursiveFirstHit(node + node->child[near_index ^ 1], &mid, end, midf, t1, trace);
		}

		/*if(!near_index)
		{
			trace->dist = d0;
			trace->normal = node->normal;
		}
		else
		{
			trace->dist = -d0;
			trace->normal.x = -node->normal.x;
			trace->normal.y = -node->normal.y;
			trace->normal.z = -node->normal.z;
		}*/

		trace->dist = d0;
		trace->normal = node->normal;
		trace->position = mid;
		trace->frac = midf;
		//trace->leaf = w_world_leaves + *(int *)&node->dist;

		//if(bsp_SolidPoint(w_world_nodes, mid) == BSP_SOLID_LEAF)
		//{
		//	trace->bm_flags |= TRACE_MID_SOLID;
		//	editor_StopPIE();
		//	//assert(0);

			//printf("oh shit... %f... ", d0);
		//}

		//printf("[%f %f %f]\n", trace->normal.x, trace->normal.y, trace->normal.z);


		//frac = trace->frac;

		/*while(bsp_SolidPoint(collision_nodes, mid) == BSP_SOLID_LEAF)
		{

			frac -= 0.1;
			if(frac < 0.0)
			{
				printf("backup past zero!\n");
				trace->bm_flags |= TRACE_MID_SOLID;
				break;
			}


			midf = t0 + (t1 - t0) * frac;
			mid.x = start->x + (end->x - start->x) * frac;
			mid.y = start->y + (end->y - start->y) * frac;
			mid.z = start->z + (end->z - start->z) * frac;

		}*/

		//trace->position = mid;
		//trace->frac = midf;

		//assert(bsp_SolidPoint(collision_nodes, mid) != BSP_SOLID_LEAF);

		/* return zero to avoid any further computation to be done
		by previous recursion calls... */
		return 0;

	}
}


/*
==============
bsp_FirstHit
==============
*/
int bsp_FirstHit(struct bsp_pnode_t *bsp, vec3_t start, vec3_t end, struct trace_t *trace)
{
	trace->frac = 1.0;
	trace->bm_flags = TRACE_ALL_SOLID;
	trace->leaf = NULL;

	return bsp_RecursiveFirstHit(bsp, &start, &end, 0, 1, trace);
}




#define STEP_HEIGHT 1.1
/*
==============
bsp_TryStepUp
==============
*/
int bsp_TryStepUp(vec3_t *position, vec3_t *velocity, struct trace_t *trace)
{
	#if 0
	vec3_t start;
	vec3_t end;
	vec3_t v;
	trace_t tr;

	int s;
	int e;

	float frac;
	float step_height;

	start = trace->position;


	/* move forward an itsy-bitsy... */
	start.x -= trace->normal.x * 0.05;
	start.z -= trace->normal.z * 0.05;


	/* kick the end-point STEP_HEIGHT up... */
	end.x = start.x;
	end.y = start.y + STEP_HEIGHT;
	end.z = start.z;

	s = bsp_SolidPoint(collision_nodes, start);
	e = bsp_SolidPoint(collision_nodes, end);

	//printf("%d %d\n", s, e);


	if(s == BSP_SOLID_LEAF)
	{
		/* both endpoints lie in
		solid space, so the step
		is too high to climb
		(or it could be a wall)... */
		if(e == BSP_SOLID_LEAF)
			return 0;

		/* trace downwards to find out the height of the
		step... */
		bsp_FirstHit(collision_nodes, end, start, &tr);

		#if 1

		frac = tr.frac;
		step_height = (end.y - start.y) * (1.0 - frac);
		//v = tr.position;

		v = *position;
		v.y += step_height;

		/* see if we can step up from our current position... */
		bsp_FirstHit(collision_nodes, *position, v, &tr);

		/* we'll bump our heads, so don't
		step up... */
		if(tr.frac < 1.0)
			return 0;

		/* step up... */
		position->y = start.y + step_height;

		/* ...and forward... */
		position->x = start.x;
		position->z = start.z;

		/* dampen the speed when stepping up... */
		velocity->x *= 0.5;
		velocity->z *= 0.5;

		return 1;

		#endif
	}


	#endif

	return 0;
}


int bsp_TryStepDown(vec3_t *position, vec3_t *velocity, struct trace_t *trace)
{
	#if 0
	vec3_t end;

	end = *position;
	end.y -= STEP_HEIGHT * 2.0;
	trace_t tr;


	bsp_FirstHit(collision_nodes, *position, end, &tr);

	//printf("%f\n", tr.frac);

	if(tr.frac > 0.0)
	{
		if(position->x == tr.position.x && position->y == tr.position.y && position->z == tr.position.z)
			return 0;

		//printf("[%f %f %f]\n")

		*position = tr.position;

		return 1;
	}


	return 0;

	#endif

}



#define BUMP_COUNT 5
#define SPEED_THRESHOLD 0.00001
/*
==============
bsp_Move
==============
*/
void bsp_Move(vec3_t *position, vec3_t *velocity)
{
	#if 0
	int i;
	int c;

	trace_t trace;

	vec3_t end;
	vec3_t new_velocity = *velocity;

	//end.x = position->x + velocity.x;
	//end.y = position->y + velocity.y;
	//end.z = position->z + velocity.z;

	//static int b_break = 0;

	if(collision_nodes)
	{
		end.x = position->x + velocity->x;
		end.y = position->y + velocity->y;
		end.z = position->z + velocity->z;

		for(i = 0; i < BUMP_COUNT; i++)
		{

			/* still enough to ignore any movement... */
			if(fabs(new_velocity.x) < SPEED_THRESHOLD &&
			   fabs(new_velocity.y) < SPEED_THRESHOLD &&
			   fabs(new_velocity.z) < SPEED_THRESHOLD)
			{
				break;
			}

			end.x = position->x + new_velocity.x;
			end.y = position->y + new_velocity.y;
			end.z = position->z + new_velocity.z;


			bsp_FirstHit(collision_nodes, *position, end, &trace);

			/* covered whole distance, bail out... */
			if(trace.frac == 1.0)
			{
				break;
			}
			else
			{

				position->x += new_velocity.x * trace.frac;
				position->y += new_velocity.y * trace.frac;
				position->z += new_velocity.z * trace.frac;

				#if 1
				/* hit a vertical-ish surface, test to see whether it's a step or a wall... */
				if(trace.normal.y < 0.2 && trace.normal.y > -0.2)
				{

					if(bsp_TryStepUp(position, &new_velocity, &trace))
					{

						/* if step-up was successful, do not clip the speed... */

						/* TODO: maybe it's a good idea to dampen the speed on
						staircases a little to avoid the player skyrocketing when walking one
						up... */
						continue;
					}
				}

				#endif


				/* horizontal-ish surface (floor or slope)... */
				bsp_ClipVelocityToPlane(trace.normal, new_velocity, &new_velocity, 1.0);

			}

		}

	//	printf(">>>>>>>>>end bsp_Move()\n");
	}

	/*if(b_break)
	{
		printf("breakpoint!\n");

		printf("breakpoint!\n");
	}*/



	*velocity = new_velocity;


	position->x += velocity->x;
	position->y += velocity->y;
	position->z += velocity->z;

	#endif

}


struct bsp_dleaf_t *bsp_GetCurrentLeaf(struct bsp_pnode_t *node, vec3_t position)
{

	float d;
	int node_index;
	int leaf_index;

	if(!node)
		return NULL;

	while(node->child[0] > BSP_EMPTY_LEAF && node->child[0] < BSP_SOLID_LEAF)
	{
		d = dot3(node->normal, position) - node->dist;

		if(d >= 0.0)
		{
			node_index = 0;
		}
		else
		{
			node_index = 1;
		}

		node += node->child[node_index];
	}

	if(node->child[0] == BSP_EMPTY_LEAF)
	{
		leaf_index = *(int *)&node->dist;
		return &w_world_leaves[leaf_index];
	}

	return NULL;
}

//k fl#define USE_COMPRESSED_PVS

unsigned char *bsp_CompressPvs(unsigned char *uncompressed_pvs, int uncompressed_pvs_size, int *compressed_pvs_size)
{
    unsigned char *compressed_pvs;
    unsigned char *out_compressed;
    int compressed_size;
    int i;
    int j;

    int compressed_index;
    int zero_count;

	/* twice the size, just to be safe... */
    compressed_pvs = memory_Calloc(2, uncompressed_pvs_size);

	for(i = 0, compressed_index = 0; i < uncompressed_pvs_size; i++)
	{
		#ifdef USE_COMPRESSED_PVS
		if(!uncompressed_pvs[i])
		{
			/* found a zero byte, so now count how many zero bytes there are (including this),
			and store the value in the byte after it... */

			compressed_pvs[compressed_index] = 0;
			compressed_index++;
			compressed_pvs[compressed_index] = 1;

			//do
			//{

			//	if(compressed_pvs[compressed_index] == 255)
			//	{
					/* hit the maximum storable value
					for a byte... */
            //        compressed_index += 2;
			//	}

			//	compressed_pvs[compressed_index]++;

			//	i++;
			//}
			//while(!uncompressed_pvs[i]);

			compressed_index++;

			//i--;
		}
		else
		#endif

		{
			/* non-zero bytes gets copied as is... */
            compressed_pvs[compressed_index] = uncompressed_pvs[i];
            compressed_index++;
		}
	}

	out_compressed = memory_Calloc(compressed_index, 1);
	memcpy(out_compressed, compressed_pvs, compressed_index);
	memory_Free(compressed_pvs);

	if(compressed_pvs_size)
	{
		*compressed_pvs_size = compressed_index;
	}

	return out_compressed;
}


struct bsp_dleaf_t **bsp_DecompressPvs(struct bsp_dleaf_t *leaf, int *leaves_count)
{
	int leaf_index;
	int byte_index;
	int i;

	int pvs_offset = 0;


	#define MAX_VISIBLE_LEAVES 512

	static int potentially_visible_leaves_count;
	static struct bsp_dleaf_t *potentially_visible_leaves[MAX_VISIBLE_LEAVES];


	leaf_index = leaf - w_world_leaves;

	potentially_visible_leaves_count = 0;
	potentially_visible_leaves[potentially_visible_leaves_count] = leaf;
	potentially_visible_leaves_count++;

	//if(!leaf->pvs)
	//{
		//return NULL;
	//}

	if(leaf->pvs)
	{
		for(i = 0; i < w_world_leaves_count /*&& potentially_visible_leaves_count < MAX_VISIBLE_LEAVES*/; i++)
		{

			/* this avoids the occasion where the current leaf is marked on its own
			pvs (which happens when the pvs thread is calculating visibility to a
			leaf that 'sees' the current leaf, and ends marking it on its own pvs
			to avoid infinte recursion), which ends up adding it twice for rendering, thus
			causing severe memory corruption problems. */
			if(i == leaf_index)
				continue;

			byte_index = (i >> 3) + pvs_offset;

			#ifdef USE_COMPRESSED_PVS

			if(!leaf->pvs[byte_index])
			{
				/* each byte fits eight leaves, so for each byte skipped
				the counter gets incremented by eight... */
				i += ((int)leaf->pvs[byte_index + 1]) << 3;



				/* The byte count in the pvs is a multiple of the leaves
				count, with each byte holding eight leaves.

				So, byte 0 holds leaves 0 to 7, byte 1 holds 8 to 15, and
				so on.

				Every zero byte incurs an overhead of an additional
				byte to keep the byte skip count. This extra byte offsets
				all other bytes in the pvs after it.

				This offsetting generates problems because the counter of
				this loop will end up indexing the wrong byte, which won't
				contain the right leaves.

                To solve this we also offset the index of the byte we're
                accessing by how many extra bytes we encountered so far.

                This problem cannot be solved while compressing the pvs
                by accounting for the extra byte because if the extra byte
                were to be added to the counter of this loop, this would
                make it stop before reaching the real end of the pvs. In
                other words the extra byte would be considered to contain
                actual leaf indexes... */
				pvs_offset++;

				continue;
			}

			#endif

			if(leaf->pvs[byte_index] & (1 << (i % 8)))
			{
				potentially_visible_leaves[potentially_visible_leaves_count] = &w_world_leaves[i];
				potentially_visible_leaves_count++;
			}
		}
	}

	*leaves_count = potentially_visible_leaves_count;

	return &potentially_visible_leaves[0];
}

struct bsp_dleaf_t **bsp_PotentiallyVisibleLeaves(int *leaf_count, vec3_t position)
{
	int i;
	int leaf_index;
	//int l = 0;

	struct bsp_pnode_t *node = &w_world_nodes[0];
	struct bsp_dleaf_t *cur_leaf = NULL;
	struct bsp_dleaf_t *leaf;

	struct bsp_dleaf_t **pvs_leaves = NULL;
	//int potentially_visible_vert_count = 0;
	//int leaf_vert_count = 0;

	float d;
	int node_index;

	if(!node)
		return NULL;

	while(node->child[0] > BSP_EMPTY_LEAF && node->child[0] < BSP_SOLID_LEAF)
	{
		d = dot3(node->normal, position) - node->dist;

		if(d >= 0.0)
		{
			node_index = 0;
		}
		else
		{
			node_index = 1;
		}

		node += node->child[node_index];
	}

	*leaf_count = 0;

	if(node->child[0] == BSP_EMPTY_LEAF)
	{
		leaf_index = *(int *)&node->dist;
		cur_leaf =  &w_world_leaves[leaf_index];
		pvs_leaves = bsp_DecompressPvs(cur_leaf, leaf_count);
	}

	return pvs_leaves;
}



struct bsp_dleaf_t *front_to_back_buffer[512];


void bsp_RecursiveFrontToBackWalk(struct bsp_pnode_t *node, int *cursor, vec3_t *position)
{
    struct bsp_dleaf_t *leaf;

    int near = 0;
    int leaf_index;

    float d;

    if(node->child[0] > BSP_EMPTY_LEAF && node->child[0] < BSP_SOLID_LEAF)
    {
        near = (dot3(*position, node->normal) - node->dist) < 0.0;

        bsp_RecursiveFrontToBackWalk(node + node->child[near], cursor, position);
        bsp_RecursiveFrontToBackWalk(node + node->child[near ^ 1], cursor, position);
    }
    else
    {
        if(node->child[0] == BSP_EMPTY_LEAF)
        {
            leaf_index = *(int *)&node->dist;
            leaf = w_world_leaves + leaf_index;

            front_to_back_buffer[*cursor] = leaf;
            (*cursor)++;
        }
    }
}


struct bsp_dleaf_t **bsp_FrontToBackWalk(int *leaf_count, vec3_t position)
{
    int cursor = 0;

    if(w_world_nodes)
    {
        bsp_RecursiveFrontToBackWalk(w_world_nodes, &cursor, &position);
    }

    front_to_back_buffer[cursor] = NULL;

    *leaf_count = cursor;

    return front_to_back_buffer;
}


void bsp_AddIndexes(int leaf_index)
{
	struct bsp_dleaf_t *leaf;

	if(w_world_leaves)
	{
		leaf = w_world_leaves + leaf_index;




	}

}

void bsp_RemoveIndexes(int leaf_index)
{

}


/*
===============================================
===============================================
===============================================
*/


char bsp_section_start_tag[] = "[bsp section start]";

struct bsp_section_start_t
{
    char tag[(sizeof(bsp_section_start_tag) + 3) &(~3)];
};





char bsp_section_end_tag[] = "[bsp section end]";

struct bsp_section_end_t
{
	char tag[(sizeof(bsp_section_end_tag) + 3) &(~3)];
};





char bsp_record_start_tag[] = "[bsp record start]";

struct bsp_record_start_t
{
    char tag[(sizeof(bsp_record_start_tag) + 3) & (~3)];

    char script_name[PATH_MAX];
    int leaf_count;
    int node_count;
    int vertice_count;
    int triangle_count;
    int batch_count;
};





char bsp_batch_record_tag[] = "[bsp batch record]";

struct bsp_batch_record_t
{
    char tag[(sizeof(bsp_batch_record_tag) + 3) & (~3)];
    char material_name[PATH_MAX];
    int count;
    int start;
};



char bsp_leaf_record_tag[] = "[bsp leaf record]";

struct bsp_leaf_record_t
{
	char tag[(sizeof(bsp_leaf_record_tag) + 3) & (~3)];
	int leaf_count;
	struct bsp_dleaf_t leaves[1];
};



char bsp_node_record_tag[] = "[bsp node record]";

struct bsp_node_record_t
{
	char tag[(sizeof(bsp_node_record_tag) + 3) & (~3)];
	int node_count;
	struct bsp_pnode_t nodes[1];
};



char bsp_triangle_record_tag[] = "[bsp triangle record]";

struct bsp_triangle_record_t
{
	char tag[(sizeof(bsp_triangle_record_tag) + 3) & (~3)];
	int triangle_count;
	struct bsp_striangle_t triangles[1];
};



char bsp_pvs_record_tag[] = "[bsp pvs record]";

struct bsp_pvs_record_t
{
	char tag[(sizeof(bsp_pvs_record_tag) + 3) & (~3)];
	int byte_count;
	unsigned char pvs[1];
};



char bsp_vertice_record_tag[] = "[bsp vertice record]";

struct bsp_vertice_record_t
{
	char tag[ (sizeof(bsp_vertice_record_tag) + 3) & (~3)];
	vertex_t vertices[1];
};



char bsp_record_end_tag[] = "[bsp record end]";

struct bsp_record_end_t
{
    char tag[(sizeof(bsp_record_end_tag) + 3) & (~3)];
};


void bsp_SerializeBsp(void **buffer, int *buffer_size)
{
	struct bsp_section_start_t *section_start;
	struct bsp_section_end_t *section_end;

	struct bsp_leaf_record_t *leaf_record;
	struct bsp_node_record_t *node_record;

	struct bsp_batch_record_t *batch_record;
	struct bsp_triangle_record_t *triangle_record;
	struct bsp_pvs_record_t *pvs_record;
	struct bsp_vertice_record_t *vertice_record;

	//FILE *debug_file;
    //debug_file = fopen("serialize_out.txt", "w");

	material_t *material;

	struct bsp_record_start_t *record_start;
	struct bsp_record_end_t *record_end;

	int i;
	int j;
	int triangle_count;
	int pvs_byte_count;


	char *out_buffer = NULL;
	char *out;
	int out_buffer_size = 0;


    void *entity_buffer = NULL;
    int entity_buffer_size = 0;

    void *waypoint_buffer = NULL;
    int waypoint_buffer_size = 0;

    void *light_buffer = NULL;
    int light_buffer_size = 0;

    void *material_buffer = NULL;
    int material_buffer_size = 0;

    out_buffer_size = sizeof(struct bsp_section_start_t) + sizeof(struct bsp_section_end_t);


    entity_SerializeEntities(&entity_buffer, &entity_buffer_size, 1);
    light_SerializeLights(&light_buffer, &light_buffer_size);
    navigation_SerializeWaypoints(&waypoint_buffer, &waypoint_buffer_size);
    material_SerializeMaterials(&material_buffer, &material_buffer_size);

	out_buffer_size += entity_buffer_size + light_buffer_size + waypoint_buffer_size + material_buffer_size;


	if(w_world_leaves)
	{

		/*pvs_byte_count = 0;

		for(i = 0; i < w_world_leaves_count; i++)
		{
			pvs_byte_count += w_world_leaves[i].pvs_lenght;
		}*/


		out_buffer_size += sizeof(struct bsp_record_start_t) + sizeof(struct bsp_record_end_t) +


						   sizeof(struct bsp_batch_record_t) * w_world_batch_count +

							/* bsp_leaf_record_t already has space for a single leaf, so that's why we take one leaf away here... */
					       sizeof(struct bsp_leaf_record_t) + sizeof(struct bsp_dleaf_t) * (w_world_leaves_count - 1) +

					       /* bsp_node_record_t already has space for a single node, so that's why we take one node away here... */
					       sizeof(struct bsp_node_record_t) + sizeof(struct bsp_pnode_t) * (w_world_nodes_count - 1) +

					       /* bsp_triangle_record_t already has space for a single triangle, so that's why we take one triangle away here... */
						   sizeof(struct bsp_triangle_record_t) + sizeof(struct bsp_striangle_t) * (w_world_triangle_count - 1) +

						   /* bsp_vertice_record_t already has space for a single vertice, so that's why we take one vertice away here... */
					       sizeof(struct bsp_vertice_record_t) + sizeof(struct vertex_t) * (w_world_vertices_count - 1);

							/* bsp_pvs_record_t already has space for a single byte, so that's why we take one byte away here... */
					       //sizeof(struct bsp_pvs_record_t) + pvs_byte_count - 1;
	}




	out_buffer = memory_Calloc(1, out_buffer_size);

    *buffer = out_buffer;
	*buffer_size = out_buffer_size;

	out = out_buffer;

	section_start = (struct bsp_section_start_t *)out;
	out += sizeof(struct bsp_section_start_t);

	strcpy(section_start->tag, bsp_section_start_tag);


	if(material_buffer)
	{
        memcpy(out, material_buffer, material_buffer_size);
		out += material_buffer_size;
	}

    if(light_buffer)
	{
		memcpy(out, light_buffer, light_buffer_size);
		out += light_buffer_size;
	}

	if(waypoint_buffer)
	{
		memcpy(out, waypoint_buffer, waypoint_buffer_size);
		out += waypoint_buffer_size;
	}

    if(entity_buffer)
	{
		memcpy(out, entity_buffer, entity_buffer_size);
		out += entity_buffer_size;
	}



    if(w_world_leaves)
	{
		record_start = (struct bsp_record_start_t *)out;
		out += sizeof(struct bsp_record_start_t);

		strcpy(record_start->tag, bsp_record_start_tag);

        if(w_world_script)
        {
            strcpy(record_start->script_name, w_world_script->script.file_name);
        }

        record_start->leaf_count = w_world_leaves_count;
        record_start->vertice_count = w_world_vertices_count;
        record_start->node_count = w_world_nodes_count;
        record_start->batch_count = w_world_batch_count;
        record_start->triangle_count = w_world_triangle_count;


        /* write vertices... */
		vertice_record = (struct bsp_vertice_record_t *)out;
		out += sizeof(struct bsp_vertice_record_t) + sizeof(vertex_t) * (w_world_vertices_count - 1);
		strcpy(vertice_record->tag, bsp_vertice_record_tag);

		for(i = 0; i < w_world_vertices_count; i++)
		{
			vertice_record->vertices[i] = w_world_vertices[i];
		}


		/* write batches... */
        for(i = 0; i < w_world_batch_count; i++)
		{
			batch_record = (struct bsp_batch_record_t *)out;
			out += sizeof(struct bsp_batch_record_t );

			strcpy(batch_record->tag, bsp_batch_record_tag);
			batch_record->count = w_world_batches[i].next;
			batch_record->start = w_world_batches[i].start;

			material = material_GetMaterialPointerIndex(w_world_batches[i].material_index);
			strcpy(batch_record->material_name, material->name);
		}


		/* write nodes... */
		node_record = (struct bsp_node_record_t *)out;
		out += sizeof(struct bsp_node_record_t) + sizeof(struct bsp_pnode_t) * (w_world_nodes_count - 1);
		strcpy(node_record->tag, bsp_node_record_tag);

		for(i = 0; i < w_world_nodes_count; i++)
		{
            node_record->nodes[i] = w_world_nodes[i];
		}



		/* write leaves... */
		leaf_record = (struct bsp_leaf_record_t *)out;
		out += sizeof(struct bsp_leaf_record_t) + sizeof(struct bsp_dleaf_t ) * (w_world_leaves_count - 1);
		strcpy(leaf_record->tag, bsp_leaf_record_tag);

		for(i = 0; i < w_world_leaves_count; i++)
		{
            leaf_record->leaves[i] = w_world_leaves[i];
		}


		/* write triangles... */
		triangle_record = (struct bsp_triangle_record_t *)out;
		out += sizeof(struct bsp_triangle_record_t ) + sizeof(struct bsp_striangle_t ) * (w_world_triangle_count - 1);
		strcpy(triangle_record->tag, bsp_triangle_record_tag);

		triangle_count = 0;

		for(i = 0; i < w_world_leaves_count; i++)
		{
			/* store in each leaf the offset into the triangle list... */
			leaf_record->leaves[i].tris = (struct bsp_striangle_t *)triangle_count;
			leaf_record->leaves[i].tris_count = w_world_leaves[i].tris_count;

			for(j = 0; j < w_world_leaves[i].tris_count; j++)
			{
                //triangle_record->triangles[triangle_count] = w_world_leaves[i].tris[j];
                triangle_record->triangles[triangle_count].first_vertex = w_world_leaves[i].tris[j].first_vertex;
                triangle_record->triangles[triangle_count].batch = w_world_leaves[i].tris[j].batch;
				triangle_count++;
			}
		}

		#if 0

		/* write pvs... */
        pvs_record = (struct bsp_pvs_record_t *)out;
        out += sizeof(struct bsp_pvs_record_t) + pvs_byte_count - 1;
        strcpy(pvs_record->tag, bsp_pvs_record_tag);

        pvs_record->byte_count = pvs_byte_count;

        pvs_byte_count = 0;

        for(i = 0; i < w_world_leaves_count; i++)
		{
			/* store in each leaf the offset into the pvs byte list... */
            leaf_record->leaves[i].pvs = (unsigned char *)pvs_byte_count;
            leaf_record->leaves[i].pvs_lenght = w_world_leaves[i].pvs_lenght;

			//printf("leaf %d:\n", i);

			//log_LogMessage(LOG_MESSAGE_NOTIFY, "serialize leaf %d pvs:\n", i);

			for(j = 0; j < w_world_leaves[i].pvs_lenght; j++)
			{
			//	printf("%d ", w_world_leaves[i].pvs[j]);
				//log_LogMessage(LOG_MESSAGE_NOTIFY, "%d ", w_world_leaves[i].pvs[j]);
                pvs_record->pvs[pvs_byte_count] = w_world_leaves[i].pvs[j];
				pvs_byte_count++;
			}
			//log_LogMessage(LOG_MESSAGE_NOTIFY, "\n");
		}

		#endif

		/* write end of the record... */
        record_end = (struct bsp_record_end_t *)out;
        out += sizeof(struct bsp_record_end_t);

        strcpy(record_end->tag, bsp_record_end_tag);
	}




	section_end = (struct bsp_section_end_t *)out;
	out += sizeof(struct bsp_section_end_t);

	strcpy(section_end->tag, bsp_section_end_tag);
}



//extern char material_section_start_tag[];
extern char light_section_start_tag[];
extern char waypoint_section_start_tag[];
//extern char entity_section_start_tag[];


void bsp_DeserializeBsp(void **buffer)
{
	struct bsp_section_start_t *section_start;
	struct bsp_section_end_t *section_end;

	struct bsp_record_start_t *record_start;
	struct bsp_record_end_t *record_end;

	struct bsp_batch_record_t *batch_records;
	struct bsp_node_record_t *node_record;
	struct bsp_leaf_record_t *leaf_record;
	struct bsp_triangle_record_t *triangle_record;
	struct bsp_pvs_record_t *pvs_record;
	struct bsp_vertice_record_t *vertice_record;

	char *in;

	int i;
	int j;

	in = *buffer;


    while(1)
	{
		if(!strcmp(in, bsp_section_start_tag))
		{
            section_start = (struct bsp_section_start_t *)in;
			in += sizeof(struct bsp_section_start_t );
		}
		else if(!strcmp(in, bsp_section_end_tag))
		{
            section_end = (struct bsp_section_end_t *)in;
			in += sizeof(struct bsp_section_end_t );
			break;
		}
		else if(!strcmp(in, light_section_start_tag))
		{
            light_DeserializeLights((void **)&in);
		}
		else if(!strcmp(in, entity_section_start_tag))
		{
            entity_DeserializeEntities((void **)&in, 1);
		}
		else if(!strcmp(in, waypoint_section_start_tag))
		{
            navigation_DeserializeWaypoints((void **)&in);
		}
		else if(!strcmp(in, material_section_start_tag))
		{
            material_DeserializeMaterials((void **)&in);
		}
		else if(!strcmp(in, bsp_record_start_tag))
		{
			record_start = (struct bsp_record_start_t *)in;
			in += sizeof(struct bsp_record_start_t );

            while(1)
            {
                if(!strcmp(in, bsp_vertice_record_tag))
                {
                    /* read vertices... */
                    vertice_record = (struct bsp_vertice_record_t *)in;
                    in += sizeof(struct bsp_vertice_record_t ) + sizeof(vertex_t) * (record_start->vertice_count - 1);

                    w_world_vertices_count = record_start->vertice_count;
                    w_world_vertices = memory_Calloc(w_world_vertices_count, sizeof(vertex_t));

                    for(i = 0; i < w_world_vertices_count; i++)
                    {
                        w_world_vertices[i] = vertice_record->vertices[i];
                    }
                }
                else if(!strcmp(in, bsp_batch_record_tag))
                {
                    /* read batches... */
                    //while(strcmp(in, bsp_batch_record_tag))i++;

                    batch_records = (struct bsp_batch_record_t *)in;
                    in += sizeof(struct bsp_batch_record_t ) * record_start->batch_count;

                    w_world_batches = memory_Calloc(record_start->batch_count, sizeof(struct batch_t));
                    w_world_batch_count = record_start->batch_count;


                    for(i = 0; i < record_start->batch_count; i++)
                    {
                        w_world_batches[i].next = batch_records[i].count;
                        w_world_batches[i].start = batch_records[i].start;
                        w_world_batches[i].material_index = material_MaterialIndex(batch_records[i].material_name);

                        //if(i)
                        //{
                        //	w_world_batches[i].start = w_world_batches[i - 1].start + w_world_batches[i - 1].next;
                        //}
                    }

                }
                else if(!strcmp(in, bsp_node_record_tag))
                {
                    /* read nodes... */
                    //while(strcmp(in, bsp_node_record_tag))in++;

                    node_record = (struct bsp_node_record_t *)in;
                    in += sizeof(struct bsp_node_record_t) + sizeof(struct bsp_pnode_t) * (record_start->node_count - 1);

                    w_world_nodes = memory_Calloc(record_start->node_count, sizeof(struct bsp_pnode_t));
                    w_world_nodes_count = record_start->node_count;

                    for(i = 0; i < w_world_nodes_count; i++)
                    {
                        w_world_nodes[i] = node_record->nodes[i];
                    }
                }
                else if(!strcmp(in, bsp_leaf_record_tag))
                {
                    /* read leaves... */
                    //while(strcmp(in, bsp_leaf_record_tag))in++;

                    leaf_record = (struct bsp_leaf_record_t *)in;
                    in += sizeof(struct bsp_leaf_record_t) + sizeof(struct bsp_dleaf_t) * (record_start->leaf_count - 1);

                    w_world_leaves = memory_Calloc(record_start->leaf_count, sizeof(struct bsp_dleaf_t));
                    w_world_leaves_count = record_start->leaf_count;

                    for(i = 0; i < w_world_leaves_count; i++)
                    {
                        w_world_leaves[i] = leaf_record->leaves[i];
                    }
                }
                else if(!strcmp(in, bsp_triangle_record_tag))
                {
                    /* read triangles... */
                   // while(strcmp(in, bsp_triangle_record_tag))in++;

                    triangle_record = (struct bsp_triangle_record_t *)in;
                    in += sizeof(struct bsp_triangle_record_t) + sizeof(struct bsp_striangle_t ) * (record_start->triangle_count - 1);

                    w_world_triangle_count = record_start->triangle_count;

                    for(i = 0; i < w_world_leaves_count; i++)
                    {
                        w_world_leaves[i].tris = memory_Calloc(leaf_record->leaves[i].tris_count, sizeof(struct bsp_striangle_t));
                        w_world_leaves[i].tris_count = leaf_record->leaves[i].tris_count;

                        for(j = 0; j < leaf_record->leaves[i].tris_count; j++)
                        {
                            w_world_leaves[i].tris[j].first_vertex = (triangle_record->triangles + (int)leaf_record->leaves[i].tris + j)->first_vertex;
                            w_world_leaves[i].tris[j].batch = (triangle_record->triangles + (int)leaf_record->leaves[i].tris + j)->batch;
                        }

                        //memcpy(w_world_leaves[i].tris, triangle_record->triangles + (int)leaf_record->leaves[i].tris, sizeof(struct bsp_striangle_t) * w_world_leaves[i].tris_count);
                    }
                }
                else if(!strcmp(in, bsp_record_end_tag))
                {
                    break;
                }
                else
                {
                    in++;
                }
            }


            #if 0

			/* read pvs... */
			while(strcmp(in, bsp_pvs_record_tag))in++;

			pvs_record =  (struct bsp_pvs_record_t *)in;
			in += sizeof(struct bsp_pvs_record_t ) + pvs_record->byte_count - 1;

			for(i = 0; i < w_world_leaves_count; i++)
			{
				w_world_leaves[i].pvs_lenght = leaf_record->leaves[i].pvs_lenght;
                w_world_leaves[i].pvs = memory_Calloc(w_world_leaves[i].pvs_lenght, 1);

				//log_LogMessage(LOG_MESSAGE_NOTIFY, "deserialize leaf %d pvs:\n", i);

                for(j = 0; j < w_world_leaves[i].pvs_lenght; j++)
				{
					w_world_leaves[i].pvs[j] = *(pvs_record->pvs + (int)leaf_record->leaves[i].pvs + j);
					//log_LogMessage(LOG_MESSAGE_NOTIFY, "%d ", w_world_leaves[i].pvs[j]);
				}

				//log_LogMessage(LOG_MESSAGE_NOTIFY, "\n");

                //memcpy(w_world_leaves[i].pvs, pvs_record->pvs + (int)leaf_record->leaves[i].pvs, w_world_leaves[i].pvs_lenght);
			}

			#endif



			log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "bsp deserialized succesfully!\nNodes: %d\nLeaves: %d\nTriangles: %d\nBatches: %d",
																											record_start->node_count,
																											record_start->leaf_count,
																											record_start->triangle_count,
																											record_start->batch_count);


			world_Update();



		}
		else if(!strcmp(in, bsp_record_end_tag))
		{
			record_end = (struct bsp_record_end_t *)in;
			in += sizeof(struct bsp_record_end_t);
		}
		else
		{
			in++;
		}
	}

	*buffer = in;

}


void bsp_SaveBsp(char *output_name)
{
	void *file_buffer;
	int file_buffer_size;
	FILE *file;

	char *file_name;

    bsp_SerializeBsp(&file_buffer, &file_buffer_size);

    file_name = path_AddExtToName(output_name, "bsp");

    file = fopen(file_name, "wb");
    fwrite(file_buffer, file_buffer_size, 1, file);
    fclose(file);
}

void bsp_LoadBsp(char *file_name)
{
    void *file_buffer;
    void *read_buffer;
    unsigned long file_buffer_size;
    FILE *file;

    char *file_path;
    file = path_TryOpenFile(file_name);


	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "bsp_LoadBsp: load file [%s]", file_name);

    if(file)
	{
		file_buffer_size = path_GetFileSize(file);

		file_buffer = memory_Calloc(1, file_buffer_size);
		fread(file_buffer, file_buffer_size, 1, file);
		fclose(file);

		read_buffer = file_buffer;

		log_LogMessage(LOG_MESSAGE_NOTIFY, 1, "bsp_LoadBsp: deserializing map [%s]", file_name);

		bsp_DeserializeBsp(&read_buffer);

		memory_Free(file_buffer);
	}
	else
	{
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "bsp_LoadBsp: couldn't open file [%s]", file_name);
	}
}


#ifdef __cplusplus
}
#endif









