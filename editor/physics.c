#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>

#include "GL\glew.h"

#include "physics.h"
#include "gpu.h"

#include "player.h"



extern int player_count;
extern player_t *players;

#define EXTRA_MARGIN 0.01




static float cube_bmodel_verts[] = 
{
	-1.0, 1.0, 1.0,
	-1.0,-1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0, 1.0, 1.0,
	-1.0, 1.0, 1.0,
	
	 1.0, 1.0, 1.0,
	 1.0,-1.0, 1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0, 1.0,-1.0,
	 1.0, 1.0, 1.0,
	 
	 1.0, 1.0,-1.0,
	 1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0, 1.0,-1.0,
	 1.0, 1.0,-1.0,
	 
	-1.0, 1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0, 1.0,
	-1.0,-1.0, 1.0,
	-1.0, 1.0, 1.0,
	-1.0, 1.0,-1.0,
	
	-1.0, 1.0,-1.0,
	-1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0,
	 1.0, 1.0,-1.0,
	-1.0, 1.0,-1.0,
	
	-1.0,-1.0, 1.0,
	-1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0, 1.0,
	-1.0,-1.0, 1.0,         
};





int block_list_size;
int block_count;
block_t *block_list;
static int *handles;

bvh_node_t *world_mesh_bvh;


static int max_static_world_mesh_triangles;
static int static_world_mesh_triangle_count;
//static vec3_t *static_world_mesh;
static collision_triangle_t *static_world_mesh;


void physics_Init()
{
	block_list_size = 64;
	block_count = 0;
	block_list = malloc(sizeof(block_t) * block_list_size);
	handles = malloc(sizeof(int) * block_list_size);
	
	world_mesh_bvh = NULL;
	
	
	max_static_world_mesh_triangles = 16000;
	static_world_mesh_triangle_count = 0;
	static_world_mesh = malloc(sizeof(collision_triangle_t) * max_static_world_mesh_triangles);
}

void physics_Finish()
{
	int i;
	
	physics_DeleteWorldMeshBVH();
	
	for(i = 0; i < block_count; i++)
	{
		free(block_list[i].verts);
	}
	
	free(block_list);
	free(handles);
	free(static_world_mesh);
	
	
}

void physics_CreateBlock(vec3_t position, vec3_t half_extents)
{
	block_t *block;
	int *h;
	
	int handle;
	int i;
	
	if(block_count >= block_list_size)
	{
		block = malloc(sizeof(block_t) * (block_list_size + 16));
		h = malloc(sizeof(int) * (block_list_size + 16));
		
		memcpy(block, block_list, sizeof(block_t) * block_list_size);
		memcpy(h, handles, sizeof(int) * block_list_size);
		
		free(block_list);
		free(handles);
		
		block_list = block;
		handles = h;
		
		block_list_size += 16;
	}
	block = &block_list[block_count];
	h = &handles[block_count];
	
	block_count++;
	
	block->position = position;
	block->half_extents = half_extents;
	block->verts = malloc(sizeof(vertex_t) * 36);
	
	
	for(i = 0; i < 36; i++)
	{
		block->verts[i].position.x = (cube_bmodel_verts[i * 3] * half_extents.x) + position.x;
		block->verts[i].position.y = (cube_bmodel_verts[i * 3 + 1] * half_extents.y) + position.y;
		block->verts[i].position.z = (cube_bmodel_verts[i * 3 + 2] * half_extents.z) + position.z;
	}
	
	
	
	*h = gpu_Alloc(sizeof(vertex_t) * 36);
	block->start = gpu_GetAllocStart(*h) / sizeof(vertex_t);
	gpu_Write(*h, 0, block->verts, sizeof(vertex_t) * 36, 0);
	
}

void physics_ProcessCollisions(float delta_time)
{
	int i;
	int c = player_count;
	int j; 
	
	float l;
	float d;
	
	vec3_t p_pos;
	vec3_t b_pos;
	vec3_t b_hext;
	
	vec3_t a;
	vec3_t b;
	
	//vec3_t v0;
	//vec3_t v1;
	
	//vec3_t closest_end;
	//vec3_t farthest_end;
	//vec3_t half_extents;
	vec3_t capsule_direction;
	vec3_t n_capsule_direction;
	
	//vec3_t node_position;
	//vec3_t node_capsule_vec;
	
	//vec3_t closest_point_on_box;
	//vec3_t closest_point_on_capsule;
	
	unsigned long long start;
	unsigned long long end;
	
	
	//bvh_node_t *p;
	//bvh_node_t *q;
	//start = _rdtsc();
	for(i = 0; i < c; i++)
	{
			 
		
			 
		l = players[i].delta.x * players[i].delta.x + players[i].delta.z * players[i].delta.z;
		
		if(l > MAX_HORIZONTAL_DELTA * MAX_HORIZONTAL_DELTA)
		{
			l = sqrt(l);
			l = MAX_HORIZONTAL_DELTA / l;
			
			players[i].delta.x *= l;
			players[i].delta.z *= l;
		}
		
		if(fabs(players[i].delta.y) > GRAVITY * 2.0)
		{
			players[i].bm_movement |= PLAYER_FLYING;
		}
		
		if(players[i].bm_movement & PLAYER_FLYING)
		{
			players[i].delta.x *= AIR_FRICTION;
			players[i].delta.z *= AIR_FRICTION;
		}
		else
		{
			players[i].delta.x *= GROUND_FRICTION;
			players[i].delta.z *= GROUND_FRICTION;
		}
		
		players[i].delta.y -= GRAVITY;
		
		
		
		
		players[i].player_position.x += players[i].delta.x;
		players[i].player_position.y += players[i].delta.y;
		players[i].player_position.z += players[i].delta.z;
		
		p_pos = players[i].player_position;
		
		
		/*if(p_pos.y - 2.0 < 0.0)
		{
			players[i].player_position.y = 2.0;	
			players[i].delta.y = 0.0;	
			players[i].bm_movement &= ~(PLAYER_JUMPED|PLAYER_FLYING);
		}*/
		
		
		a = players[i].player_position;
		a.y += PLAYER_CAPSULE_HEIGHT / 2.0;
		
		b = players[i].player_position;
		b.y -= PLAYER_CAPSULE_HEIGHT / 2.0;
		
		
		capsule_direction.x = (b.x - a.x);
		capsule_direction.y = (b.y - a.y);
		capsule_direction.z = (b.z - a.z);
		
		n_capsule_direction = normalize3(capsule_direction);
		
		
		
		physics_WalkBVH(world_mesh_bvh, &a, &b, &capsule_direction, &n_capsule_direction, &players[i]);
		
		
		
		
	}
	
	
	//end = _rdtsc();
	//printf("%llu\n", end - start);
}

void physics_UpdateWorldMesh(vertex_t *vertices, int vertex_count)
{
	int i;
	int q_normal;
	
	vec3_t normal;
	
	vertex_count /= 3;
	
	for(i = 0; i < vertex_count; i++)
	{
		static_world_mesh[i].a = vertices[i * 3].position;
		static_world_mesh[i].b = vertices[i * 3 + 1].position;
		static_world_mesh[i].c = vertices[i * 3 + 2].position;
		
		//normal = normalize3(cross(static_world_mesh[i].a, static_world_mesh[i].c));
		
		normal = vertices[i * 3].normal;
		//static_world_mesh[i].normal = normal;
		
		q_normal = 0;
		
		//printf("[%f %f %f]		", normal.x, normal.y, normal.z);
		
		q_normal |= (int)(0x3ff * (normal.z * 0.5 + 0.5));
		q_normal <<= 10;
		q_normal |= (int)(0x3ff * (normal.y * 0.5 + 0.5));
		q_normal <<= 10;
		q_normal |= (int)(0x3ff * (normal.x * 0.5 + 0.5));
		
		static_world_mesh[i].packed_normal = q_normal;
		
		
		normal.x = ((float)(0x3ff & q_normal) / 0x3ff) * 2.0 - 1.0;
		q_normal >>= 10;
		normal.y = ((float)(0x3ff & q_normal) / 0x3ff) * 2.0 - 1.0;
		q_normal >>= 10;
		normal.z = ((float)(0x3ff & q_normal) / 0x3ff) * 2.0 - 1.0;
		
		//printf("[%f %f %f]\n", normal.x, normal.y, normal.z);
		
		
	}
	
	static_world_mesh_triangle_count = vertex_count;
}

void physics_BuildWorldMeshBVH()
{
	int i;
	int j;
	int k;
	
	bvh_node_t *n;
	bvh_node_t *t;
	bvh_node_t *prev0;
	bvh_node_t *prev1;
	bvh_node_t *closest_prev;
	bvh_node_t *closest;
	bvh_node_t *in;
	bvh_node_t *out;
	
	bvh_node_t *start_list = NULL;
	bvh_node_t *last = NULL;
	
	
	bvh_node_t *lists[2];
	int active_list = 0;
	
	vec3_t v0;
	vec3_t v1;
	vec3_t v2;
	
	/*vec3_t e0;
	vec3_t e1;
	vec3_t e2;*/
	
	vec3_t edges[3];
	
	vec3_t position;
	vec3_t aabb;
	
	float max_x;
	float min_x;
	float max_y;
	float min_y;
	float max_z;
	float min_z;
	
	float d;
	float l0;
	float l1;
	float l;
	
	vec3_t lv;
	
	if(!static_world_mesh) return;
	
	//for(i = 0; i < block_count; i++)
	//{
		//for(j = 0; j < 36; j += 3)
		for(i = 0; i < static_world_mesh_triangle_count; i++)
		{
		
		
			
			//v0 = block_list[i].verts[j].position;
			//v1 = block_list[i].verts[j + 1].position;
			//v2 = block_list[i].verts[j + 2].position;
			
			v0 = static_world_mesh[i].a;
			v1 = static_world_mesh[i].b;
			v2 = static_world_mesh[i].c;
			
			edges[0] = v0;
			edges[1] = v1;
			edges[2] = v2;
			
			max_x = -999999999999.999999;
			min_x = 999999999999.999999;
			max_y = -999999999999.999999;
			min_y = 999999999999.999999;
			max_z = -999999999999.999999;
			min_z = 999999999999.999999;
			
			for(k = 0; k < 3; k++)
			{
				if(edges[k].x > max_x) max_x = edges[k].x;
				if(edges[k].x < min_x) min_x = edges[k].x;
				
				if(edges[k].y > max_y) max_y = edges[k].y;
				if(edges[k].y < min_y) min_y = edges[k].y;
				
				if(edges[k].z > max_z) max_z = edges[k].z;
				if(edges[k].z < min_z) min_z = edges[k].z;
			}
			
			position.x = (max_x + min_x) / 2.0;
			position.y = (max_y + min_y) / 2.0;
			position.z = (max_z + min_z) / 2.0;
			
			n = malloc(sizeof(bvh_node_t));
			n->position = position;
			n->left = NULL;
			n->right = NULL;
			
			n->start = i;
			
			/* AABBs will overestimate the size of the triangles in all
			but a few cases, but this overestimation will happen
			only in the triangle level. This still will cause
		    some of false positives... */
			n->half_extents.x = (max_x - min_x) / 2.0 + EXTRA_MARGIN;
			n->half_extents.y = (max_y - min_y) / 2.0 + EXTRA_MARGIN;
			n->half_extents.z = (max_z - min_z) / 2.0 + EXTRA_MARGIN;
			
			
			n->next = NULL;
			
			if(!start_list)
			{
				start_list = n;
				//last = n;
			}
			else
			{
				last->next = n;
			}
			
			
			last = n;
		}
	//}
	
	lists[(active_list + 1) % 2] = start_list;
	
	do
	{
		in = lists[(active_list + 1) % 2];
		out = NULL;
		
		i = 0;
		
		while(in)
		{
			
			//t = in->next;
			
			t = in;
			d = 999999999999.999;
			
			closest = NULL;
			
			
			while(t->next)
			{
				prev1 = t;
				t = t->next;
				
				
				v0.x = t->position.x - in->position.x;
				v0.y = t->position.y - in->position.y;
				v0.z = t->position.z - in->position.z;
					
				if(v0.x > in->half_extents.x) v0.x = in->half_extents.x;
				else if(v0.x < -in->half_extents.x) v0.x = -in->half_extents.x;
				
				if(v0.y > in->half_extents.y) v0.y = in->half_extents.y;
				else if(v0.y < -in->half_extents.y) v0.y = -in->half_extents.y;
				
				if(v0.z > in->half_extents.z) v0.z = in->half_extents.z;
				else if(v0.z < -in->half_extents.z) v0.z = -in->half_extents.z;
				
				v0.x += in->position.x;
				v0.y += in->position.y;
				v0.z += in->position.z;
	
				
				
				v1.x = in->position.x - t->position.x;
				v1.y = in->position.y - t->position.y;
				v1.z = in->position.z - t->position.z;
					
				if(v1.x > t->half_extents.x) v1.x = t->half_extents.x;
				else if(v1.x < -t->half_extents.x) v1.x = -t->half_extents.x;
				
				if(v1.y > t->half_extents.y) v1.y = t->half_extents.y;
				else if(v1.y < -t->half_extents.y) v1.y = -t->half_extents.y;
				
				if(v1.z > t->half_extents.z) v1.z = t->half_extents.z;
				else if(v1.z < -t->half_extents.z) v1.z = -t->half_extents.z;
				
				v1.x += t->position.x;
				v1.y += t->position.y;
				v1.z += t->position.z;
				
	
			
				v2.x = v1.x - v0.x;
				v2.y = v1.y - v0.y;
				v2.z = v1.z - v0.z;
				
				l = v2.x * v2.x + v2.y * v2.y + v2.z * v2.z;
				
				if(l < d)
				{
					//if((l < in->half_extents.x * in->half_extents.x || l < in->half_extents.y * in->half_extents.y  || l < in->half_extents.z * in->half_extents.z) || (!(in->next->next)))
					{
						d = l;				
						closest = t;
						closest_prev = prev1;
					}
				}
			
			}	
	
			t = in->next;
			
			if(closest)
			{		
				n = malloc(sizeof(bvh_node_t));
				n->left = in;
				n->right = closest;			
				
				n->left->parent = n;
				
				/* if the closest is the second element in the current
				list, advance the pointer again... */
				if(closest == t)
				{
					t = t->next;
				}
				else
				{
					closest_prev->next = n->right->next;
				}
				
				n->right->parent = n;
	
	
				l0 = n->left->position.x + n->left->half_extents.x;
				l1 = n->right->position.x + n->right->half_extents.x;
				
				if(l0 > l1) max_x = l0;
				else max_x = l1;
				
				l0 = n->left->position.x - n->left->half_extents.x;
				l1 = n->right->position.x - n->right->half_extents.x;
				
				if(l0 < l1) min_x = l0;
				else min_x = l1;
				
				
				
				l0 = n->left->position.y + n->left->half_extents.y;
				l1 = n->right->position.y + n->right->half_extents.y;
				
				if(l0 > l1) max_y = l0;
				else max_y = l1;
				
				l0 = n->left->position.y - n->left->half_extents.y;
				l1 = n->right->position.y - n->right->half_extents.y;
				
				if(l0 < l1) min_y = l0;
				else min_y = l1;
				
				
				
				l0 = n->left->position.z + n->left->half_extents.z;
				l1 = n->right->position.z + n->right->half_extents.z;
				
				if(l0 > l1) max_z = l0;
				else max_z = l1;
				
				l0 = n->left->position.z - n->left->half_extents.z;
				l1 = n->right->position.z - n->right->half_extents.z;
				
				if(l0 < l1) min_z = l0;
				else min_z = l1;	
				
				
				n->position.x = (max_x + min_x) / 2.0;
				n->position.y = (max_y + min_y) / 2.0;
				n->position.z = (max_z + min_z) / 2.0;
				
				n->half_extents.x = (max_x - min_x) / 2.0;
				n->half_extents.y = (max_y - min_y) / 2.0;
				n->half_extents.z = (max_z - min_z) / 2.0;
				
			}	
			else
			{
				/* there isn't a close enough volume to this one,
				so it goes alone on this level of the BVH...*/
				n = in;
			}
			
			n->next = NULL;
			
			
			if(!out)
			{
				out = n;
			}
			else
			{
				last->next = n;	
			}
			
			
			last = n;
			
			in = t;
			i++;
			
		}
		
		lists[active_list % 2] = out;
		active_list++;
		
	}while(i > 1);
	
	world_mesh_bvh = out;
}

void physics_DeleteWorldMeshBVH()
{
	bvh_node_t *p = world_mesh_bvh;
	bvh_node_t *r;
	
	while(p)
	{
		if(p->left)
		{
			r = p->left;
			p->left = NULL;
			p = r;
			continue;
		}		
		if(p->right)
		{
			r = p->right;
			p->right = NULL;
			p = r;
			continue;
		}
		
		r = p->parent;
		free(p);
		p = r;
	}
	
}

void physics_WalkBVH(bvh_node_t *root, vec3_t *capsule_a, vec3_t *capsule_b, vec3_t *capsule_direction, vec3_t *capsule_normalized_direction, player_t *player)
{
	
	vec3_t v0;
	vec3_t v1;
	vec3_t v2;
	vec3_t v3;
	vec3_t v4;
	
	vec3_t e0;
	vec3_t e1;
	vec3_t e2;
	vec3_t n0;
	
	//vec3_t closest_end;
	//vec3_t farthest_end;
	
	//#define USE_SSE
	
	
	/*#ifdef USE_SSE
	vec4_t half_extents;
	vec4_t node_position;
	#else*/
	
	vec3_t box_position;
	vec3_t box_half_extents;
	
	vec3_t p0;
	vec3_t p1;
	//vec3_t c0;
	//vec3_t c1;
	
	int clipped_count;
	vec3_t clipped_end_points[2];
	
	vec3_t capsule_position;
	vec3_t capsule_half_extents;
	//#endif
	//vec3_t c_direction;
	//vec3_t n_capsule_direction;
	
	unsigned int packed_normal;
	vec3_t normal;
	
	
	vec4_t node_capsule_vec;
	
	vec4_t closest_point_on_box;
	vec4_t closest_point_on_capsule;
	vec4_t closest_point_on_plane;
	vec3_t closest_vertex_on_tri;
	vec3_t closest_end;
	
	static int sign[4] = {0x80000000, 0x80000000, 0x80000000, 0x80000000}; 
	static float norm[4] = {4.0};
	
	vec4_t tri_capsule_vec;
	
	float l;
	float r;
	float s;
	
	float d0;
	float d1;
	float d;
	int colliding = 0;
	int b_flag;
	
	unsigned long long start;
	unsigned long long end;
	
	bvh_node_t *p;
	bvh_node_t *q;
	
	if(!root)
	{
		return ;
	}
	
	//v0 = *capsule_a;
	
	//start = _rdtsc();
	
	
	box_position = root->position;
	box_half_extents = root->half_extents;
	
	capsule_position = *capsule_a;
	capsule_position.y -= PLAYER_CAPSULE_HEIGHT / 2.0;
	
	
	capsule_half_extents.x = PLAYER_CAPSULE_RADIUS;
	capsule_half_extents.y = PLAYER_CAPSULE_RADIUS;
	capsule_half_extents.z = PLAYER_CAPSULE_RADIUS;
	
	//start = _rdtsc();
	glUseProgram(0);
	
	if(capsule_position.x + capsule_half_extents.x >= box_position.x - box_half_extents.x && capsule_position.x - capsule_half_extents.x <= box_position.x + box_half_extents.x)
	{
		if(capsule_position.y + capsule_half_extents.y >= box_position.y - box_half_extents.y && capsule_position.y - capsule_half_extents.y <= box_position.y + box_half_extents.y)
		{
			if(capsule_position.z + capsule_half_extents.z >= box_position.z - box_half_extents.z && capsule_position.z - capsule_half_extents.z <= box_position.z + box_half_extents.z)
			{
				colliding = 1;
			}
		}
	}
	
	
	//end = _rdtsc();
	
	//printf("%llu\n", end - start);
	
	
	#if 0
	
	#ifdef USE_SSE
	
	asm
	(

		
		//"mov esi, %[root]\n"						/* this box */
		"mov esi, [%[root]]\n"
		
		"movups xmm0, [esi]\n"						/* box position */		
		"movups xmm1, [esi + 12]\n"					/* half extents */		
		"movups xmm2, [%[sign]]\n"					/* sign mask */
		
		//"mov esi, %[capsule_a]\n"
		"mov esi, [%[capsule_a]]\n"
		"movups xmm3, [esi]\n"						/* capsule_a */
		
		"movups xmm4, xmm1\n"
		"orps xmm4, xmm2\n"							/* negated half extents */
		
		
		
		"movups xmm5, xmm3\n"
		"subps xmm5, xmm0\n"						/* node_capsule_vec = capsule_a - node_position */
		
		
		"maxps xmm5, xmm4\n"
		"minps xmm5, xmm1\n"						/* clamp node_capsule_vec to the box extents... */
		
			
		
		"movups xmm6, xmm5\n"
		"addps xmm6, xmm0\n"
		
		"movups [%[closest_point_on_box]], xmm6\n"
		
		
		"movups xmm5, xmm6\n"
		"subps xmm5, xmm3\n"
		
		"movups [%[node_capsule_vec]], xmm5\n"	

		
		:: [root] "rm" (root), 
		   [sign] "rm" (sign), 
		   [capsule_a] "rm" (capsule_a), 
		   [node_capsule_vec] "rm" (node_capsule_vec), 
		   [node_position] "rm" (node_position), 
		   [half_extents] "rm" (half_extents),
		   [closest_point_on_box] "rm" (closest_point_on_box)
		   
		    : "esi"			
	);
	
	
	#else
	
	node_position = root->position;
	half_extents = root->half_extents;
	
	node_capsule_vec.x = capsule_a->x - node_position.x;
	node_capsule_vec.y = capsule_a->y - node_position.y;
	node_capsule_vec.z = capsule_a->z - node_position.z;

	/* clamp the vector to the extents of the box... */
	if(node_capsule_vec.x > half_extents.x) node_capsule_vec.x = half_extents.x;
	else if(node_capsule_vec.x < -half_extents.x) node_capsule_vec.x = -half_extents.x;
			
	if(node_capsule_vec.y > half_extents.y) node_capsule_vec.y = half_extents.y;
	else if(node_capsule_vec.y < -half_extents.y) node_capsule_vec.y = -half_extents.y;
			
	if(node_capsule_vec.z > half_extents.z) node_capsule_vec.z = half_extents.z;
	else if(node_capsule_vec.z < -half_extents.z) node_capsule_vec.z = -half_extents.z;
			
	/* translate vector, to obtain an actual point... */
	
	
	closest_point_on_box.x = node_capsule_vec.x + node_position.x;
	closest_point_on_box.y = node_capsule_vec.y + node_position.y;
	closest_point_on_box.z = node_capsule_vec.z + node_position.z;
			
			
	/* here vector goes from capsule to node, so this would be better
	called 'capsule_node_vec'... */
	node_capsule_vec.x = closest_point_on_box.x - capsule_a->x;
	node_capsule_vec.y = closest_point_on_box.y - capsule_a->y;
	node_capsule_vec.z = closest_point_on_box.z - capsule_a->z;
	
	
	#endif
		
			
	/* project the vector onto the capsule normalized direction
	vector... */
	l = (capsule_normalized_direction->x * node_capsule_vec.x + 
	     capsule_normalized_direction->y * node_capsule_vec.y +
		 capsule_normalized_direction->z * node_capsule_vec.z) / PLAYER_CAPSULE_HEIGHT;
	
			
	/* clamp it... */	 
	if(l > 1.0) l = 1.0;
	else if(l <= 0.0) l = 0.0;	 
		
	//printf("%f\n", l);	
	
			
			
	/* closest point on the line that defines the capsule... */
	closest_point_on_capsule.x = capsule_a->x + capsule_direction->x * l;	 
	closest_point_on_capsule.y = capsule_a->y + capsule_direction->y * l;
	closest_point_on_capsule.z = capsule_a->z + capsule_direction->z * l;
	

			
			
	/* shortest distante between capsule and box... */
	node_capsule_vec.x = closest_point_on_box.x - closest_point_on_capsule.x;
	node_capsule_vec.y = closest_point_on_box.y - closest_point_on_capsule.y;
	node_capsule_vec.z = closest_point_on_box.z - closest_point_on_capsule.z;
	
	

			
	l = node_capsule_vec.x * node_capsule_vec.x + node_capsule_vec.y * node_capsule_vec.y + node_capsule_vec.z * node_capsule_vec.z; 
	
	
	#endif
	
	//end = _rdtsc();
	
	//printf("%llu\n", end - start);
	
	
	
	//printf("%f\n", l);
			
	//if(l < PLAYER_CAPSULE_RADIUS * PLAYER_CAPSULE_RADIUS)
	if(colliding)
	{
		
		if(root->left)
		{
			//printf("collision with node...\n");
			physics_WalkBVH(root->left, capsule_a, capsule_b, capsule_direction, capsule_normalized_direction, player);
		}
		if(root->right)
		{
			//printf("collision with node...\n");
			physics_WalkBVH(root->right, capsule_a, capsule_b, capsule_direction, capsule_normalized_direction, player);
		}
		
		if((!root->right) && (!root->left))
		{
			
			
			/*glLineWidth(4.0);
			glBegin(GL_LINES);
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			

			
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			
			
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			
			glEnd();
			glLineWidth(1.0);*/
			
			
			
			
			
			
			v0 = static_world_mesh[root->start].a;
			v1 = static_world_mesh[root->start].b;
			v2 = static_world_mesh[root->start].c;
			
			packed_normal = static_world_mesh[root->start].packed_normal;
			
			normal.x = ((float)(0x3ff & packed_normal) / 0x3ff) * 2.0 - 1.0;
			packed_normal >>= 10;
			normal.y = ((float)(0x3ff & packed_normal) / 0x3ff) * 2.0 - 1.0;
			packed_normal >>= 10;
			normal.z = ((float)(0x3ff & packed_normal) / 0x3ff) * 2.0 - 1.0;
			
			
			/*v0.x += normal.x;*/
			//v0.y += normal.y * 2.0;
			/*v0.z += normal.z;*/
			
			
			/*v1.x += normal.x;*/
			//v1.y += normal.y * 2.0;
			/*v1.z += normal.z;*/
			
			
			/*v2.x += normal.x;*/
			//v2.y += normal.y * 2.0;
			/*v2.z += normal.z;*/
			
			
			v3.x = capsule_a->x - v0.x;
			v3.y = capsule_a->y - v0.y - PLAYER_CAPSULE_HEIGHT / 2.0;
			v3.z = capsule_a->z - v0.z;
			
			d0 = (v3.x * normal.x + v3.y * normal.y + v3.z * normal.z);
			
			
			/*v3.x = capsule_b->x - v0.x;
			v3.y = capsule_b->y - v0.y;
			v3.z = capsule_b->z - v0.z;
			
			d1 = (v3.x * normal.x + v3.y * normal.y + v3.z * normal.z);
			
			
			if(d0 > d1)
			{
				d = d1;
			}
			else
			{
				d = d0;
			}*/
			
			
			if(fabs(d0) < PLAYER_CAPSULE_RADIUS)
			{
				
				v3 = *capsule_a;
				
				v3.y -= PLAYER_CAPSULE_HEIGHT / 2.0;
				
				closest_point_on_plane.vec3 = physics_ProjectPointOnPlane(v3, v0, normal);
						
				
				//closest_point_on_plane.vec3 = p0;

				
				
				v0.x -= closest_point_on_plane.x;
				v0.y -= closest_point_on_plane.y;
				v0.z -= closest_point_on_plane.z;
				
				v1.x -= closest_point_on_plane.x;
				v1.y -= closest_point_on_plane.y;
				v1.z -= closest_point_on_plane.z;
				
				v2.x -= closest_point_on_plane.x;
				v2.y -= closest_point_on_plane.y;
				v2.z -= closest_point_on_plane.z;
				
				e0 = cross(v1, v2);
				e1 = cross(v2, v0);
				
				if(dot3(e0, e1) < 0.0)
				{
					return;
				}
				
				e2 = cross(v0, v1);
				
				if(dot3(e0, e2) < 0.0)
				{
					return;
				}
						
				player->player_position.y += normal.y * (PLAYER_CAPSULE_RADIUS - d0);
				
				if(normal.y >= player->max_slope)
				{
					player->delta.y = 0.0;
					player->bm_movement &= ~(PLAYER_JUMPED|PLAYER_FLYING);
				}
				else
				{
					player->delta.y += normal.y * (PLAYER_CAPSULE_RADIUS - d0);
					player->delta.x += normal.x * (PLAYER_CAPSULE_RADIUS - d0);
					player->delta.z += normal.z * (PLAYER_CAPSULE_RADIUS - d0);
					
					
					player->player_position.z += normal.z * (PLAYER_CAPSULE_RADIUS - d0);
					player->player_position.x += normal.x * (PLAYER_CAPSULE_RADIUS - d0);
				}
				
				/*glPointSize(8.0);
				glBegin(GL_POINTS);
				glVertex3f(v0.x, v0.y, v0.z);
				glEnd();
				glPointSize(1.0);*/
				
				
				
				
				*capsule_a = player->player_position;
				capsule_a->y += PLAYER_CAPSULE_HEIGHT / 2.0;
				
				*capsule_b = player->player_position;
				capsule_b->y -= PLAYER_CAPSULE_HEIGHT / 2.0;
			}

		}
	}
	
}

void physics_RayCastBVH(bvh_node_t *root, vec3_t *origin, vec3_t *normalized_direction, float max_distance, ray_cast_result_t *result)
{
	vec3_t closest_point_on_ray;
	vec3_t closest_point_on_box;
	
	vec3_t box_position = root->position;
	vec3_t box_half_extents = root->half_extents;
	
	vec3_t ray_box_vec;
	vec3_t box_ray_vec;
	
	float l;
	
	
	box_ray_vec.x = origin->x - box_position.x;
	box_ray_vec.y = origin->y - box_position.y;
	box_ray_vec.z = origin->z - box_position.z;
	
	
	if(box_ray_vec.x > box_half_extents.x) box_ray_vec.x = box_half_extents.x;
	else if(box_ray_vec.x < -box_half_extents.x) box_ray_vec.x = -box_half_extents.x;
	
	if(box_ray_vec.y > box_half_extents.y) box_ray_vec.y = box_half_extents.y;
	else if(box_ray_vec.y < -box_half_extents.y) box_ray_vec.y = -box_half_extents.y;
	
	if(box_ray_vec.z > box_half_extents.z) box_ray_vec.z = box_half_extents.z;
	else if(box_ray_vec.z < -box_half_extents.z) box_ray_vec.z = -box_half_extents.z;
	
	
	closest_point_on_box.x = box_ray_vec.x + box_position.x;
	closest_point_on_box.y = box_ray_vec.y + box_position.y;
	closest_point_on_box.z = box_ray_vec.z + box_position.z;
	
	
	ray_box_vec.x = closest_point_on_box.x - origin->x;
	ray_box_vec.y = closest_point_on_box.y - origin->y;
	ray_box_vec.z = closest_point_on_box.z - origin->z;
	
	
	l = ray_box_vec.x * normalized_direction->x + ray_box_vec.y * normalized_direction->y + ray_box_vec.z * normalized_direction->z;
	
	if(l < 0.0) l = 0.0;
	else if(l > max_distance) l = max_distance;
	
	closest_point_on_ray.x = origin->x + normalized_direction->x * l;
	closest_point_on_ray.y = origin->y + normalized_direction->y * l;
	closest_point_on_ray.z = origin->z + normalized_direction->z * l;


	/*glPointSize(8.0);
	glBegin(GL_POINTS);
	glVertex3f(closest_point_on_ray.x, closest_point_on_ray.y, closest_point_on_ray.z);
	glEnd();
	glPointSize(1.0);*/
	
	/*printf("[%f %f %f]\n", closest_point_on_ray.x, closest_point_on_ray.y, closest_point_on_ray.z);*/

	/*ray_box_vec.x = closest_point_on_ray.x - box_position.x;
	ray_box_vec.y = closest_point_on_ray.y - box_position.y;
	ray_box_vec.z = closest_point_on_ray.z - box_position.z;
	
	if(ray_box_vec.x > box_half_extents.x) ray_box_vec.x = box_half_extents.x;
	else if(ray_box_vec.x < -box_half_extents.x) ray_box_vec.x = -box_half_extents.x;
	
	if(ray_box_vec.y > box_half_extents.y) ray_box_vec.y = box_half_extents.y;
	else if(ray_box_vec.y < -box_half_extents.y) ray_box_vec.y = -box_half_extents.y;
	
	if(ray_box_vec.z > box_half_extents.z) ray_box_vec.z = box_half_extents.z;
	else if(ray_box_vec.z < -box_half_extents.z) ray_box_vec.z = -box_half_extents.z;
	
	closest_point_on_box.x = box_position.x + ray_box_vec.x;
	closest_point_on_box.y = box_position.y + ray_box_vec.y;
	closest_point_on_box.z = box_position.z + ray_box_vec.z;*/
	
	ray_box_vec.x = closest_point_on_ray.x - closest_point_on_box.x;
	ray_box_vec.y = closest_point_on_ray.y - closest_point_on_box.y;
	ray_box_vec.z = closest_point_on_ray.z - closest_point_on_box.z;
	
	l = ray_box_vec.x * ray_box_vec.x + ray_box_vec.y * ray_box_vec.y + ray_box_vec.z * ray_box_vec.z;
	
	if(l <= 0.00000004)
//	if(closest_point_on_ray.x == closest_point_on_box.x && closest_point_on_ray.y == closest_point_on_box.y && closest_point_on_ray.z == closest_point_on_box.z)
	{
		if(root->left)
		{
			physics_RayCastBVH(root->left, origin, normalized_direction, max_distance, result);
		}
		
		if(root->right)
		{
			physics_RayCastBVH(root->right, origin, normalized_direction, max_distance, result);
		}
		
		
		if((!(root->left)) && (!(root->right)))
		{
			glPointSize(8.0);
			glBegin(GL_POINTS);
			glVertex3f(closest_point_on_box.x, closest_point_on_box.y, closest_point_on_box.z);
			glEnd();
			glPointSize(1.0);
		}
	}
	
	
}


void physics_RayCast(vec3_t *origin, vec3_t *normalized_direction, float max_distance, ray_cast_result_t *result)
{	
	physics_RayCastBVH(world_mesh_bvh, origin, normalized_direction, max_distance, result);
}

int physics_CheckProjectileCollisionBVH(bvh_node_t *root, projectile_t *projectile, vec3_t *position, vec3_t *normal, int *collided)
{
	
	vec3_t v0;
	vec3_t v1;
	vec3_t v2;
	vec3_t v3;
	vec3_t v4;
	
	vec3_t e0;
	vec3_t e1;
	vec3_t e2;
	vec3_t n0;
	
	//vec3_t closest_end;
	//vec3_t farthest_end;
	
	//#define USE_SSE
	
	
	/*#ifdef USE_SSE
	vec4_t half_extents;
	vec4_t node_position;
	#else*/
	
	vec3_t box_position;
	vec3_t box_half_extents;
	
	vec3_t projectile_box_position;
	vec3_t projectile_box_half_extents;
	
	//vec3_t p0;
	//vec3_t p1;
	//vec3_t c0;
	//vec3_t c1;
	
	//int clipped_count;
	//vec3_t clipped_end_points[2];
	
	//vec3_t capsule_position;
	//vec3_t capsule_half_extents;
	//#endif
	//vec3_t c_direction;
	//vec3_t n_capsule_direction;
	
	unsigned int packed_normal;
	vec3_t plane_normal;
	
	
	vec4_t node_capsule_vec;
	
	//vec4_t closest_point_on_box;
	//vec4_t closest_point_on_capsule;
	vec4_t closest_point_on_plane;
	//vec3_t closest_vertex_on_tri;
	//vec3_t closest_end;
	
	//static int sign[4] = {0x80000000, 0x80000000, 0x80000000, 0x80000000}; 
	//static float norm[4] = {4.0};
	
	//vec4_t tri_capsule_vec;
	
	//float l;
	//float r;
	//float s;
	
	float d0;
	float d1;
	float d;
	int colliding = 0;
	//int b_flag;
	
	unsigned long long start;
	unsigned long long end;
	
	float radius;
	
	//bvh_node_t *p;
	//bvh_node_t *q;
	
	if(!root)
	{
		return 0;
	}
	

	
	
	box_position = root->position;
	box_half_extents = root->half_extents;
	
	
	projectile_box_half_extents.x = fabs(projectile->delta.x) * 0.5;
	projectile_box_half_extents.y = fabs(projectile->delta.y) * 0.5;
	projectile_box_half_extents.z = fabs(projectile->delta.z) * 0.5;
	
	projectile_box_position.x = projectile->position.x + projectile->delta.x * 0.5;
	projectile_box_position.y = projectile->position.y + projectile->delta.y * 0.5;
	projectile_box_position.z = projectile->position.z + projectile->delta.z * 0.5;
	
	
	if(projectile_box_position.x + projectile_box_half_extents.x >= box_position.x - box_half_extents.x && projectile_box_position.x - projectile_box_half_extents.x <= box_position.x + box_half_extents.x)
	{		
		if(projectile_box_position.y + projectile_box_half_extents.y >= box_position.y - box_half_extents.y && projectile_box_position.y - projectile_box_half_extents.y <= box_position.y + box_half_extents.y)
		{
			if(projectile_box_position.z + projectile_box_half_extents.z >= box_position.z - box_half_extents.z && projectile_box_position.z - projectile_box_half_extents.z <= box_position.z + box_half_extents.z)
			{
				//printf("box box\n");
				colliding = 1;
			}
		}
	}
	
	
	/*glLineWidth(4.0);
	glBegin(GL_LINES);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
			

			
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
			
			
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x + projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z - projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y + projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
	glVertex3f(projectile_box_position.x - projectile_box_half_extents.x, projectile_box_position.y - projectile_box_half_extents.y, projectile_box_position.z + projectile_box_half_extents.z);
			
	glEnd();
	glLineWidth(1.0);*/

		
	
	if(colliding)
	{
		
		if(root->left)
		{
			physics_CheckProjectileCollisionBVH(root->left, projectile, position, normal, collided);
			//printf("collision with node...\n");
			//physics_WalkBVH(root->left, capsule_a, capsule_b, capsule_direction, capsule_normalized_direction, player);
		}
		if(root->right)
		{
			physics_CheckProjectileCollisionBVH(root->right, projectile, position, normal, collided);
			//printf("collision with node...\n");
			//physics_WalkBVH(root->right, capsule_a, capsule_b, capsule_direction, capsule_normalized_direction, player);
		}
		
		if((!root->right) && (!root->left))
		{
			
			
			/*glLineWidth(4.0);
			glBegin(GL_LINES);
			glColor3f(1.0, 1.0, 0.0);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			

			
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			
			
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x + box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z - box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y + box_half_extents.y, box_position.z + box_half_extents.z);
			glVertex3f(box_position.x - box_half_extents.x, box_position.y - box_half_extents.y, box_position.z + box_half_extents.z);
			
			glEnd();
			glLineWidth(1.0);*/
			
			
			
			
			
			
			v0 = static_world_mesh[root->start].a;
			v1 = static_world_mesh[root->start].b;
			v2 = static_world_mesh[root->start].c;
			
			packed_normal = static_world_mesh[root->start].packed_normal;
			
			plane_normal.x = ((float)(0x3ff & packed_normal) / 0x3ff) * 2.0 - 1.0;
			packed_normal >>= 10;
			plane_normal.y = ((float)(0x3ff & packed_normal) / 0x3ff) * 2.0 - 1.0;
			packed_normal >>= 10;
			plane_normal.z = ((float)(0x3ff & packed_normal) / 0x3ff) * 2.0 - 1.0;
			
			
			d = physics_IntersectPlane(v0, plane_normal, projectile->position, projectile->delta);
			
			//printf("%f\n", d);

			if(d >= 0.0 && d <= 1.0)
			{
				
				//printf("a: %f\n", d);		

				//closest_point_on_plane.vec3 = physics_ProjectPointOnPlane(projectile->position, v0, plane_normal);
				
				//closest_point_on_plane.vec3 = lerp3(projectile->position, projectile->delta, d);
				
				closest_point_on_plane.x = projectile->position.x + projectile->delta.x * d;
				closest_point_on_plane.y = projectile->position.y + projectile->delta.y * d;
				closest_point_on_plane.z = projectile->position.z + projectile->delta.z * d;
				
				/*e0.x = v1.x - v0.x;	
				e0.y = v1.y - v0.y;
				e0.z = v1.z - v0.z;	
				
				v3.x = closest_point_on_plane.x - v0.x;
				v3.y = closest_point_on_plane.y - v0.y;
				v3.z = closest_point_on_plane.z - v0.z;
				
				v3 = cross(v3, e0);
				
				if(dot3(v3, plane_normal) > 0.0)
				{
					return 0;
				}
				
				
				e0.x = v2.x - v1.x;	
				e0.y = v2.y - v1.y;
				e0.z = v2.z - v1.z;	
				
				v3.x = closest_point_on_plane.x - v1.x;
				v3.y = closest_point_on_plane.y - v1.y;
				v3.z = closest_point_on_plane.z - v1.z;
				
				v3 = cross(v3, e0);
				
				if(dot3(v3, plane_normal) > 0.0)
				{
					return 0;
				}
				
				
				e0.x = v0.x - v2.x;	
				e0.y = v0.y - v2.y;
				e0.z = v0.z - v2.z;	
				
				v3.x = closest_point_on_plane.x - v2.x;
				v3.y = closest_point_on_plane.y - v2.y;
				v3.z = closest_point_on_plane.z - v2.z;
				
				v3 = cross(v3, e0);
				
				if(dot3(v3, plane_normal) > 0.0)
				{
					return 0;
				}*/
							
				
				v0.x -= closest_point_on_plane.x;
				v0.y -= closest_point_on_plane.y;
				v0.z -= closest_point_on_plane.z;
				
				v1.x -= closest_point_on_plane.x;
				v1.y -= closest_point_on_plane.y;
				v1.z -= closest_point_on_plane.z;
				
				v2.x -= closest_point_on_plane.x;
				v2.y -= closest_point_on_plane.y;
				v2.z -= closest_point_on_plane.z;
				
				e0 = cross(v1, v2);
				e1 = cross(v2, v0);
				
				if(dot3(e0, e1) < 0.0)
				{
					return 0;
				}
				
				e2 = cross(v0, v1);
				
				if(dot3(e0, e2) < 0.0)
				{
					return 0;
				}
				
				//printf("c: %f\n", d);
				
				//printf("point on triangle!\n");
				
				if(position) *position = closest_point_on_plane.vec3;
				if(normal) *normal = plane_normal;
				
				*collided = 1;
				
				return 1;
			}

		}
	}
	return 0;
}

int physics_CheckProjectileCollisionPlayers(projectile_t *projectile, player_t **hit)
{
	int i;
	int c = player_count;
	int colliding;
	
	vec3_t v0;
	vec3_t v1;
	
	vec3_t projectile_box_position;
	vec3_t projectile_box_half_extents;
	
	vec3_t box_position;
	vec3_t box_half_extents;
	
	float radius;
	float d;
	
	for(i = 0; i < c; i++)
	{
		
		/*radius = PLAYER_CAPSULE_RADIUS;
					
		glBegin(GL_LINES);
		glColor3f(0.0, 1.0, 0.0);
					
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y + radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y + radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y + radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y + radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y - radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y - radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y - radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y - radius, players[i].player_position.z - radius);
					
					
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y + radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y + radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y - radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y - radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y + radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y + radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y - radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y - radius, players[i].player_position.z - radius);
					
					
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y + radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y - radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y + radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y - radius, players[i].player_position.z + radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y + radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x + radius, players[i].player_position.y - radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y + radius, players[i].player_position.z - radius);
		glVertex3f(players[i].player_position.x - radius, players[i].player_position.y - radius, players[i].player_position.z - radius);
							
					//glVertex3f(projectiles[i].position.x, projectiles[i].position.y, projectiles[i].position.z);
		glEnd();*/
		
		if(projectile->player_index == i) continue;
		
		projectile_box_half_extents.x = fabs(projectile->delta.x) * 0.5;
		projectile_box_half_extents.y = fabs(projectile->delta.y) * 0.5;
		projectile_box_half_extents.z = fabs(projectile->delta.z) * 0.5;
		
		projectile_box_position.x = projectile->position.x + projectile->delta.x * 0.5;
		projectile_box_position.y = projectile->position.y + projectile->delta.y * 0.5;
		projectile_box_position.z = projectile->position.z + projectile->delta.z * 0.5;
		
		box_position = players[i].player_position;
		//box_half_e
		
		colliding = 0;
		if(projectile_box_position.x + projectile_box_half_extents.x >= box_position.x - PLAYER_CAPSULE_RADIUS && projectile_box_position.x - projectile_box_half_extents.x <= box_position.x + PLAYER_CAPSULE_RADIUS)
		{		
			if(projectile_box_position.y + projectile_box_half_extents.y >= box_position.y - PLAYER_CAPSULE_RADIUS && projectile_box_position.y - projectile_box_half_extents.y <= box_position.y + PLAYER_CAPSULE_RADIUS)
			{
				if(projectile_box_position.z + projectile_box_half_extents.z >= box_position.z - PLAYER_CAPSULE_RADIUS && projectile_box_position.z - projectile_box_half_extents.z <= box_position.z + PLAYER_CAPSULE_RADIUS)
				{
					//printf("box box\n");
					//colliding = 1;
					
					
				}
			}
		}
		
	}
}

int physics_CheckProjectileCollision(projectile_t *projectile, vec3_t *position, vec3_t *normal, player_t **hit)
{
	int collided = 0;
	physics_CheckProjectileCollisionBVH(world_mesh_bvh, projectile, position, normal, &collided);
	//physics_CheckProjectileCollisionPlayers(projectile, hit);
	return collided;
}




float physics_ClipEdge(vec3_t plane_point, vec3_t plane_normal, vec3_t a, vec3_t b)
{
	float t;
	vec3_t v0;
	vec3_t v1;
	
	v0.x = plane_point.x - a.x;
	v0.y = plane_point.y - a.y;
	v0.z = plane_point.z - a.z;
	
	v1.x = b.x - a.x;
	v1.y = b.y - a.y;
	v1.z = b.z - a.z;
	
	
	t = dot3(v0, plane_normal) / dot3(v1, plane_normal);
	
	
	if(t > 1.0) t = 1.0;
	else if(t < 0.0) t = 0.0;
	
	return t;
}

float physics_IntersectPlane(vec3_t plane_point, vec3_t plane_normal, vec3_t ray_origin, vec3_t ray_direction)
{
	float t = -1.0;
	float d;
	
	vec3_t v0;
	
	v0.x = plane_point.x - ray_origin.x;
	v0.y = plane_point.y - ray_origin.y;
	v0.z = plane_point.z - ray_origin.z;
	
	d = dot3(ray_direction, plane_normal);
	
	if(d != 0.0)
	{
		t = dot3(v0, plane_normal) / d;
	}
		
	return t;
	
}




vec3_t physics_ProjectPointOnPlane(vec3_t a, vec3_t plane_point, vec3_t plane_normal)
{
	vec3_t v;
	float l;
	
	v.x = a.x - plane_point.x;
	v.y = a.y - plane_point.y;
	v.z = a.z - plane_point.z;
	
	l = v.x * plane_normal.x + v.y * plane_normal.y + v.z * plane_normal.z;
	
	v.x = a.x - plane_normal.x * l;
	v.y = a.y - plane_normal.y * l;
	v.z = a.z - plane_normal.z * l;
	
	return v;
}





