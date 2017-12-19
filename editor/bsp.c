#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "GL\glew.h"

#include "bsp.h"
#include "brush.h"
#include "camera.h"
#include "material.h"
#include "shader.h"
#include "l_main.h"
#include "input.h"


//extern bsp_node_t *world_bsp;
extern int world_vertices_count;
extern vertex_t *world_vertices;
extern int global_triangle_group_count;
extern triangle_group_t *global_triangle_groups;


extern int world_nodes_count;
extern bsp_pnode_t *world_nodes;
extern int world_leaves_count;
extern bsp_dleaf_t *world_leaves;

//extern bsp_leaf_t *world_leaves;

int draw_bsp_shader;

bsp_pnode_t *entity_hull = NULL;


/* from light.c */
extern int visible_light_count;



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



void bsp_Init()
{

}

void bsp_Finish()
{
	//bsp_DeleteSolidLeaf(world_bsp);
	bsp_DeleteBsp();
}

void bsp_LoadFile(char *file_name)
{
	
}


void bsp_DeleteBsp()
{
	
	int i;
	int c = world_leaves_count;
	
	if(world_nodes)
	{
		
		for(i = 0; i < c; i++)
		{
			free(world_leaves[i].pvs);
			free(world_leaves[i].tris);
		}
		
		free(world_nodes);
		free(world_leaves);
		
		world_nodes = NULL;
		world_leaves = NULL;
	}
	
	
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
	
}



/*
==============
bsp_HullForEntity
==============
*/
bsp_pnode_t *bsp_HullForEntity(vec3_t mins, vec3_t max)
{
	
}


/*
==============
bsp_SolidPoint
==============
*/
int bsp_SolidPoint(bsp_pnode_t *node, vec3_t point)
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



/*
==============
bsp_FirstHit
==============
*/
int bsp_FirstHit(bsp_pnode_t *world_nodes, vec3_t start, vec3_t end, trace_t *trace)
{
	trace->frac = 1.0;
	trace->bm_flags = TRACE_ALL_SOLID;	

	return bsp_RecursiveFirstHit(world_nodes, &start, &end, 0, 1, trace);
}




#define DIST_EPSILON 0.03125
/*
==============
bsp_RecursiveFirstHit
==============
*/
int bsp_RecursiveFirstHit(bsp_pnode_t *node, vec3_t *start, vec3_t *end, float t0, float t1, trace_t *trace)
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
	
	
	//bsp_node_t *near;
	//bsp_node_t *far;
	bsp_dleaf_t *leaf;
	
	
	if(node->child[0] == BSP_SOLID_LEAF)
	{
		return 0;		
	}
	else if(node->child[0] == BSP_EMPTY_LEAF)
	{
		trace->bm_flags &= ~TRACE_ALL_SOLID;
		return 1;
	}
	else
	{
		d0 = dot3(*start, node->normal) - node->dist;
		d1 = dot3(*end, node->normal) - node->dist;
		
		
		if(d0 >= 0.0 && d1 >= 0.0)
		{
			return bsp_RecursiveFirstHit(node + node->child[0], start, end, t0, t1, trace); /* 0 */
		}
		else if(d0 < 0.0 && d1 < 0.0)
		{
			return bsp_RecursiveFirstHit(node + node->child[1], start, end, t0, t1, trace); /* 0 */
		}
		
		
		if(d0 < 0.0)
		{
			/* nudge the intersection away from the plane
			on both sides... */
			frac = (d0 + DIST_EPSILON) / (d0 - d1);
			frac2 = (d0 - DIST_EPSILON) / (d0 - d1);
			near_index = 1;
			
		}
		else
		{
			frac = (d0 - DIST_EPSILON) / (d0 - d1);
			frac2 = (d0 + DIST_EPSILON) / (d0 - d1);	
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

		if(frac2 > 1.0) frac2 = 1.0;
		else if(frac2 < 0.0) frac2 = 0.0;

		midf2 = t0 + (t1 - t0) * frac2;
		
		mid2.x = start->x + (end->x - start->x) * frac2;
		mid2.y = start->y + (end->y - start->y) * frac2;
		mid2.z = start->z + (end->z - start->z) * frac2;

		if(bsp_RecursiveFirstHit(node + node->child[near_index ^ 1], &mid2, end, midf2, t1, trace))
		{
			return 1;	
		}

		
		
		/* I'm not satisfied with this... */
		if(frac < trace->frac)
		{
			
			trace->frac = frac;
			trace->dist = d0;
			trace->normal = node->normal;
			trace->position = mid;
		}
		
		return 0;
		
	}
	
}




#define STEP_HEIGHT 0.85
/*
==============
bsp_TryStepUp
==============
*/
int bsp_TryStepUp(vec3_t *position, vec3_t *velocity)
{
	vec3_t start;
	vec3_t end;
	
	trace_t trace;
	
	float diff;
	
	int in_open;
	
	start = *position;
	start.y += STEP_HEIGHT;
	
	
	end.x = start.x + velocity->x;
	end.y = start.y + velocity->y;
	end.z = start.z + velocity->z;
	
	
	/* kick the start position STEP_HEIGHT up, and test
	to see if we hit anything... */
	
	bsp_FirstHit(world_nodes, start, end, &trace);
	
	
	
	/* if the trace hits nothing or it does hit something */	
	if(trace.frac > 0)
	{
		
		/* step forward, but just enough to not clip anything on top
		of the step (like another step)... */
		start.x = start.x + (end.x - start.x) * trace.frac;
		start.z = start.z + (end.z - start.z) * trace.frac;
		
		
		
		end.x = start.x;
		end.y = start.y - STEP_HEIGHT;		/* this step can be STEP_HEIGHT high or less... */
		end.z = start.z;
		
		/* so trace downwards to find the actual height of the step... */
		bsp_FirstHit(world_nodes, start, end, &trace);
		
					
		position->x = start.x;
		/* nudge the adjusted position upwards a little bit so
		the final position doesn't end up inside solid space... */
		position->y = start.y + DIST_EPSILON + (end.y - start.y) * trace.frac; 
		position->z = start.z;
		
		/* dampen the speed when stepping up... */
		velocity->x *= 0.05;
		velocity->z *= 0.05;
		return 1;
	}
	
	return 0;
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
	int i;
	int c;
	
	trace_t trace;
	
	vec3_t end;
	vec3_t new_velocity = *velocity;
	
	//end.x = position->x + velocity.x;
	//end.y = position->y + velocity.y;
	//end.z = position->z + velocity.z;
	
	//static int b_break = 0;
	
	if(world_nodes)
	{
		
		end.x = position->x + velocity->x;
		end.y = position->y + velocity->y;
		end.z = position->z + velocity->z;
		
		
		/*if(!bsp_SolidPoint(bsp_root, end))
		{
			printf("point in open space!\n");
		}*/
		
	//	printf(">>>>>>>>>start bsp_Move()\n");
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
			
			
			bsp_FirstHit(world_nodes, *position, end, &trace);
			
			/* covered whole distance, bail out... */
			if(trace.frac == 1.0)
			{
				break;
			}
			else
			{
				
				
				
				#if 0
				/* hit a vertical-ish surface, test to see whether it's a step or a wall... */
				if(trace.normal.y < 0.2 && trace.normal.y > -0.2)
				{
					//printf("before: [%f %f %f]   after: [%f %f %f]\n", position->x, position->y, position->z, trace.position.x, trace.position.y, trace.position.z);
					/* this will probably fail if the speed is too low... */
					if(bsp_TryStepUp(position, &new_velocity))
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
	
}


bsp_dleaf_t *bsp_GetCurrentLeaf(bsp_pnode_t *node, vec3_t camera_position)
{
	
	float d;
	int node_index;
	int leaf_index;
	
	if(!node)
		return NULL;
		
	while(node->child[0] > BSP_EMPTY_LEAF && node->child[0] < BSP_SOLID_LEAF)
	{
		d = dot3(node->normal, camera_position) - node->dist;
		
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
		return &world_leaves[leaf_index];
	}
	
	return NULL;
}

#define MAX_VISIBLE_LEAVES 512

int potentially_visible_leaves_count;
bsp_dleaf_t *potentially_visible_leaves[MAX_VISIBLE_LEAVES];

bsp_dleaf_t **bsp_PotentiallyVisibleLeaves(int *leaf_count, vec3_t camera_position)
{
	int i;
	int leaf_index;
	int l = 0;
	
	bsp_pnode_t *node = &world_nodes[0];
	bsp_dleaf_t *cur_leaf = NULL;
	bsp_dleaf_t *leaf;
	
	
	float d;
	int node_index;
	
	if(!node)
		return NULL;
			
	while(node->child[0] > BSP_EMPTY_LEAF && node->child[0] < BSP_SOLID_LEAF)
	{
		d = dot3(node->normal, camera_position) - node->dist;
		
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
		cur_leaf =  &world_leaves[leaf_index];
		
		
		/* avoid going over the pvs if the current leaf
		didn't change between this and the last call... */
		
		/*if(cur_leaf == potentially_visible_leaves[0])
		{
			*leaf_count = potentially_visible_leaves_count;
			return &potentially_visible_leaves[0];
		}*/
		
		potentially_visible_leaves[l++] = cur_leaf;
		
		for(i = 0; i < world_leaves_count && i < MAX_VISIBLE_LEAVES; i++)
		{
			if(!cur_leaf->pvs) 
				break;
			
			//assert(cur_leaf->pvs);	
				
			if(cur_leaf->pvs[i >> 3] & (1 << (i % 8)))
			{
				potentially_visible_leaves[l++] = &world_leaves[i];
			}
		}
		
		*leaf_count = l;
		potentially_visible_leaves_count = l;
		return &potentially_visible_leaves[0];
	}
	
	
	return NULL;
	 
}










