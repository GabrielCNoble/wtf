#include <stdio.h>
#include <stdlib.h>

#include "SDL2\SDL.h"
#include "GL\glew.h"

#include "camera.h"
#include "bsp.h"
#include "bsp_cmp.h"
#include "pvs.h"
#include "l_main.h"
#include "c_memory.h"
#include "log.h"

//#define LOG_STUFF

#define USE_MEMORY_MALLOC

#ifdef USE_MEMORY_MALLOC
#define malloc ((void *)0)
#define free ((void *)0)
#define calloc ((void *)0)
#define strdup ((void *)0)
#endif


/* from world.c */
extern int w_world_nodes_count;
extern struct bsp_pnode_t *w_world_nodes;
extern bsp_polygon_t *w_node_polygons;			/* necessary to quickly build portals... */
extern int w_world_leaves_count;
extern struct bsp_dleaf_t *w_world_leaves;


//timedout_leaf_t *timed_out_leaves = NULL;

bsp_portal_t *world_portals = NULL;

static unsigned long long int performance_frequency;

static float delta_time;
static unsigned long long start_delta;
static unsigned long long end_delta;

int b_calculating_pvs = 0;
int b_step = 0;

extern SDL_mutex *polygon_copy_mutex;
extern SDL_sem *step_semaphore;
extern int b_draw_pvs_steps;


SDL_mutex *stop_flag_mutex = NULL;
SDL_mutex *job_list_mutex = NULL;
SDL_sem *pvs_threads_semaphore = NULL;
int b_stop = 0;

#define SMALL_START_TIME_OUT 15000.0
#define SMALL_TIME_OUT_INCREMENT 15000.0

#define START_TIME_OUT 60000.0
#define MAX_TIME_OUT 14400000.0				/* maximum of 4 hours per leaf... */
#define TIME_OUT_INCREMENT 60000.0



/* to allow pvs calculation debugging from the main thread... */

//pvs_for_leaf_stack *pvs_calc_stack;

#define MAX_RECURSIVE_PVS_STACK_DEPTH 256

pvs_for_leaf_stack_t *pvs_for_leaf_stack;


pvs_job_t *pvs_jobs = NULL;
pvs_job_t *last_job = NULL;


void bsp_InitPvsTimer()
{
	performance_frequency = SDL_GetPerformanceFrequency();
	//start_delta = 0;
	//end_delta = 0;
	//delta_time = 0.0;
	//_malloc_dbg();
	start_delta = SDL_GetPerformanceCounter();
}

float bsp_DeltaTime()
{
	unsigned long long cur = SDL_GetPerformanceCounter();
	return (float)((cur - start_delta) * 1000) / (float)performance_frequency;
}

void bsp_StopPvsTimer()
{

}



/* number of samples per meter cubed... */
#define BSP_VIS_SAMPLE_DENSITY 10
#define BSP_VIS_RESOLUTION 512

void bsp_GenVisProbes(struct bsp_node_t *bsp)
{
    vec3_t max;
    vec3_t min;

    int x_count;
    int y_count;
    int z_count;

    struct bsp_vis_probe_t *probes;

    if(bsp)
    {
        bsp_BspBounds(bsp, &max, &min);
    }












}



int bsp_ClassifyPortalVertex(struct bsp_pnode_t *node, vec3_t point)
{
	float d;

	d = dot3(point, node->normal) - node->dist;

	if(d > FUZZY_ZERO)
	{
		return POINT_FRONT;
	}
	else if(d < -FUZZY_ZERO)
	{
		return POINT_BACK;
	}

	return POINT_CONTAINED;

}


int bsp_ClassifyPortal(struct bsp_pnode_t *node, bsp_portal_t *portal)
{
	bsp_polygon_t *p;
	float d;
	int front = 0;
	int back = 0;
	int i;
	int c;

	p = portal->portal_polygon;
	c = p->vert_count;

	for(i = 0; i < c; i++)
	{
		d = dot3(p->vertices[i].position, node->normal) - node->dist;

		if(d > FUZZY_ZERO)
		{
			front++;
		}
		else if(d < -FUZZY_ZERO)
		{
			back++;
		}
	}


	if(front && back)
	{
		return PORTAL_STRADDLING;
	}
	else
	{
		if(front)
		{
			return PORTAL_FRONT;
		}
		else if(back)
		{
			return PORTAL_BACK;
		}
		else
		{
			return PORTAL_CONTAINED;
		}
	}

}


void bsp_SplitPortal(struct bsp_pnode_t *node, bsp_portal_t *portal, bsp_portal_t **front, bsp_portal_t **back)
{
	vertex_t clip_vertex;

	int front_vertex_count = 0;
	vertex_t front_vertexes[128];

	int back_vertex_count = 0;
	vertex_t back_vertexes[128];

	bsp_polygon_t *polygon;

	float time;

	int i = 0;
	int c;
	int r;

	int pa;
	int pb;

	float d0;
	float d1;

	int cur_side = 0;

	bsp_portal_t *f;
	bsp_portal_t *b;

	polygon = portal->portal_polygon;

	pa = bsp_ClassifyPortalVertex(node, polygon->vertices[0].position);
	pb = bsp_ClassifyPortalVertex(node, polygon->vertices[1].position);
	c = polygon->vert_count;

	//assert(pa != pb);
	assert(c > 2);


	if(pa == POINT_CONTAINED)
	{
		cur_side = pb;
	}
	else
	{
		cur_side = pa;
	}

	//cur_side = pa;

	//printf("before split edges\n");
	for(i = 0; i < c; i++)
	{
		r = (i + 1) % c;

		pa = bsp_ClassifyPortalVertex(node, polygon->vertices[i].position);
		pb = bsp_ClassifyPortalVertex(node, polygon->vertices[r].position);

		//assert(pa != POINT_CONTAINED && pb != POINT_CONTAINED);

		/* this edge straddles this splitter, so clip it... */
		if((pa | pb) == (POINT_FRONT | POINT_BACK))
		{
			//bsp_ClipEdge(polygon->vertices[i], polygon->vertices[r], point, normal, &clip_vertex, &time);


			d0 = dot3(polygon->vertices[i].position, node->normal) - node->dist;
			d1 = dot3(polygon->vertices[r].position, node->normal) - node->dist;

			time = d0 / (d0 - d1);


			clip_vertex.position.x = polygon->vertices[i].position.x * (1.0 - time) + polygon->vertices[r].position.x * time;
			clip_vertex.position.y = polygon->vertices[i].position.y * (1.0 - time) + polygon->vertices[r].position.y * time;
			clip_vertex.position.z = polygon->vertices[i].position.z * (1.0 - time) + polygon->vertices[r].position.z * time;

			if(cur_side == POINT_FRONT)
			{
				cur_side = POINT_BACK;

				front_vertexes[front_vertex_count++] = polygon->vertices[i];
				front_vertexes[front_vertex_count++] = clip_vertex;
				back_vertexes[back_vertex_count++] = clip_vertex;
			}
			else
			{
				cur_side = POINT_FRONT;

				back_vertexes[back_vertex_count++] = polygon->vertices[i];
				back_vertexes[back_vertex_count++] = clip_vertex;
				front_vertexes[front_vertex_count++] = clip_vertex;
			}

			//clip_vertex_count++;

		}
		else
		{
			if(cur_side == POINT_FRONT)
			{
				front_vertexes[front_vertex_count++] = polygon->vertices[i];

				if(pa == POINT_CONTAINED)
				{
					back_vertexes[back_vertex_count++] = polygon->vertices[i];
					cur_side = pb;
				}
			}
			else
			{
				back_vertexes[back_vertex_count++] = polygon->vertices[i];

				if(pa == POINT_CONTAINED)
				{
					front_vertexes[front_vertex_count++] = polygon->vertices[i];
					cur_side = pb;
				}
			}

		}
	}

	/* if this polygon straddles this splitter, there HAS
	to be two clipping points + at least one vertex that
	belongs to the polygon...  */
	assert(back_vertex_count > 2);
	assert(front_vertex_count > 2);

	//printf("front: %d   back: %d\n", front_vertex_count, back_vertex_count);

	//assert(clip_vertex_count > 0);

	#ifndef USE_MEMORY_MALLOC
	f = malloc(sizeof(bsp_portal_t));
	#else
	f = memory_Malloc(sizeof(bsp_portal_t ));
	#endif
	f->leaf0 = portal->leaf0;
	f->leaf1 = portal->leaf1;
	f->next = NULL;

	#ifndef USE_MEMORY_MALLOC
	f->portal_polygon = malloc(sizeof(bsp_polygon_t));
	#else
	f->portal_polygon = memory_Malloc(sizeof(bsp_polygon_t));
	#endif

	f->portal_polygon->normal = polygon->normal;
	f->portal_polygon->vert_count = front_vertex_count;

	#ifndef USE_MEMORY_MALLOC
	f->portal_polygon->vertices = malloc(sizeof(vertex_t) * (front_vertex_count));
	#else
	f->portal_polygon->vertices = memory_Malloc(sizeof(vertex_t) * front_vertex_count);
	#endif
	f->portal_polygon->next = NULL;

	for(i = 0; i < front_vertex_count; i++)
	{
		f->portal_polygon->vertices[i] = front_vertexes[i];
	}


	#ifndef USE_MEMORY_MALLOC
	b = malloc(sizeof(bsp_portal_t ));
	#else
	b = memory_Malloc(sizeof(bsp_portal_t));
	#endif

	b->leaf0 = portal->leaf0;
	b->leaf1 = portal->leaf1;
	b->next = NULL;

	#ifndef USE_MEMORY_MALLOC
	b->portal_polygon = malloc(sizeof(bsp_polygon_t));
	#else
	b->portal_polygon = memory_Malloc(sizeof(bsp_polygon_t));
	#endif

	b->portal_polygon->normal = polygon->normal;
	b->portal_polygon->vert_count = back_vertex_count;

	#ifndef USE_MEMORY_MALLOC
	b->portal_polygon->vertices = malloc(sizeof(vertex_t) * (back_vertex_count));
	#else
	b->portal_polygon->vertices = memory_Malloc(sizeof(vertex_t) * back_vertex_count);
	#endif
	b->portal_polygon->next = NULL;


	for(i = 0; i < back_vertex_count; i++)
	{
		b->portal_polygon->vertices[i] = back_vertexes[i];
	}

	*front = f;
	*back = b;
}


void bsp_BspBounds(bsp_node_t *bsp, vec3_t *maxs, vec3_t *mins)
{
	bsp_polygon_t *polygon;
	bsp_leaf_t *leaf;
	int i;
	int c;
	if(bsp->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)bsp;

		if(!(leaf->bm_flags & BSP_SOLID))
		{
			polygon = leaf->polygons;

			while(polygon)
			{
				c = polygon->vert_count;

				for(i = 0; i < c; i++)
				{
					if(polygon->vertices[i].position.x > maxs->x) maxs->x = polygon->vertices[i].position.x;
					if(polygon->vertices[i].position.y > maxs->y) maxs->y = polygon->vertices[i].position.y;
					if(polygon->vertices[i].position.z > maxs->z) maxs->z = polygon->vertices[i].position.z;

					if(polygon->vertices[i].position.x < mins->x) mins->x = polygon->vertices[i].position.x;
					if(polygon->vertices[i].position.y < mins->y) mins->y = polygon->vertices[i].position.y;
					if(polygon->vertices[i].position.z < mins->z) mins->z = polygon->vertices[i].position.z;
				}

				polygon = polygon->next;
			}
		}
	}
	else
	{
		bsp_BspBounds(bsp->front, maxs, mins);
		bsp_BspBounds(bsp->back, maxs, mins);
	}
}


int c_count = 0;
int t_count = 0;

bsp_portal_t *bsp_ClipPortalToBsp(bsp_node_t *bsp, bsp_portal_t *portal)
{

	int leaf_index;
	bsp_portal_t *front;
	bsp_portal_t *back;
	bsp_portal_t *f;
	bsp_portal_t *r;
	bsp_portal_t *n;

	if(bsp->type == BSP_LEAF)
	{
		if(bsp->bm_flags & BSP_SOLID)
		{
			#ifndef USE_MEMORY_MALLOC
			free(portal->portal_polygon->vertices);
			free(portal->portal_polygon);
			free(portal);
			#else
			memory_Free(portal->portal_polygon->vertices);
			memory_Free(portal->portal_polygon);
			memory_Free(portal);
			#endif

			return NULL;
		}

		if(!portal->leaf0)
		{
			portal->leaf0 = (bsp_leaf_t *)bsp;
		}
		else
		{
			portal->leaf1 = (bsp_leaf_t *)bsp;
		}

		portal->next = NULL;
		return portal;

	}

	else
	{
		switch(bsp_ClassifyPolygon(portal->portal_polygon, bsp->point, bsp->normal))
		{
			case POLYGON_FRONT:
				return bsp_ClipPortalToBsp(bsp->front, portal);
			break;

			case POLYGON_BACK:
				bsp_ClipPortalToBsp(bsp->back, portal);
			break;


			//case PORTAL_STRADDLING:
			case POLYGON_STRADDLING:

					#ifndef USE_MEMORY_MALLOC
					front = malloc(sizeof(bsp_portal_t));
					#else
					front = memory_Malloc(sizeof(bsp_portal_t));
					#endif

					front->leaf0 = portal->leaf0;
					front->leaf1 = portal->leaf1;
					front->next = NULL;

					#ifndef USE_MEMORY_MALLOC
					back = malloc(sizeof(bsp_portal_t));
					#else
					back = memory_Malloc(sizeof(bsp_portal_t));
					#endif
					back->leaf0 = portal->leaf0;
					back->leaf1 = portal->leaf1;
					back->next = NULL;

					bsp_SplitPolygon(portal->portal_polygon, bsp->point, bsp->normal, &front->portal_polygon, &back->portal_polygon);


				//bsp_SplitPortal(node, portal, &front, &back);

					//front = bsp_ClipPortalToBsp(&world_nodes[node->child[0]], front);
					//back = bsp_ClipPortalToBsp(&world_nodes[node->child[1]], back);

				//front = bsp_ClipPortalToBsp(node + node->child[0], front);
				//back = bsp_ClipPortalToBsp(node + node->child[1], back);

				front = bsp_ClipPortalToBsp(bsp->front, front);
				back = bsp_ClipPortalToBsp(bsp->back, back);

				#ifndef USE_MEMORY_MALLOC
				free(portal->portal_polygon->vertices);
				free(portal->portal_polygon);
				free(portal);
				#else
				memory_Free(portal->portal_polygon->vertices);
				memory_Free(portal->portal_polygon);
				memory_Free(portal);
				#endif

				if(front)
				{
					r = front;
					while(r->next)
					{
						r = r->next;
					}

					r->next = back;

					return front;
				}

				return back;

			break;


			//case PORTAL_CONTAINED:
			case POLYGON_CONTAINED_FRONT:
			case POLYGON_CONTAINED_BACK:
				front = NULL;
				portal->next = NULL;

				f = bsp_ClipPortalToBsp(bsp->front, portal);
				r = f;

				while(r)
				{
					n = r->next;
					r->next = NULL;

					back = bsp_ClipPortalToBsp(bsp->back, r);


					if(back)
					{
						r = back;
						while(r->next)
						{
							r = r->next;
						}
						r->next = front;
						front = back;
					}

					r = n;

				}

				return front;

			break;

		}
	}


}


bsp_portal_t *g_portal_list = NULL;
bsp_portal_t *g_next_portal = NULL;


void bsp_ClipPortalsToBsp(bsp_node_t *bsp, bsp_portal_t **portals)
{
	bsp_portal_t *p = *portals;
	bsp_portal_t *n;
	bsp_portal_t *r;
	bsp_portal_t *t = NULL;
	bsp_portal_t *s;
	int portal_count = 0;
	//static int i = 0;
	while(p)
	{
		//i++;
		//printf("%d\n", i);

		n = p->next;
		p->next = NULL;
		r = bsp_ClipPortalToBsp(bsp, p);
		s = r;
		if(r)
		{
			while(r->next)
			{
				r = r->next;
			}
			r->next = t;
			t = s;
		}

		p = n;
	}

	*portals = t;
}

void bsp_RemoveBadPortals(bsp_portal_t **portals)
{
	bsp_portal_t *p = NULL;
	bsp_portal_t *prev_p = NULL;
	bsp_portal_t *n = NULL;
	bsp_portal_t *prev_n = NULL;
	bsp_portal_t *s = NULL;
	bsp_portal_t *q;
	bsp_portal_t *biggest;
	bsp_polygon_t *polygon;
	float biggest_area = 0.0;
	vec3_t center;
	vec3_t v;
	vec3_t v0;
	vec3_t v1;

	float x;
	float y;

	float x_max;
	float y_max;
	float y_min;
	float x_min;

	float w;
	float h;

	float num;
	float denum;

	int i;

/*	p = *portals;
	while(p)
	{

		if(p->portal_polygon->vert_count > 200)
		{
			engine_BreakPoint();
		}

		p = p->next;
	}*/


	p = *portals;
	while(p)
	{
		/* bad portal... */
		if(!p->leaf0 || !p->leaf1)
		{
			_remove:

			if(n)
			{
				n->next = p->next;
			}
			else
			{
				n = p->next;

				if(p == *portals)
				{
					*portals = n;
				}
			}

			#ifndef USE_MEMORY_MALLOC
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(p);
			#else
			memory_Free(p->portal_polygon->vertices);
			memory_Free(p->portal_polygon);
			memory_Free(p);
			#endif

			p = n;
			n = NULL;
			continue;
		}


		center.x = 0;
		center.y = 0;
		center.z = 0;
		polygon = p->portal_polygon;

		for(i = 0; i < polygon->vert_count; i++)
		{
			center.x += polygon->vertices[i].position.x;
			center.y += polygon->vertices[i].position.y;
			center.z += polygon->vertices[i].position.z;
		}

		center.x /= polygon->vert_count;
		center.y /= polygon->vert_count;
		center.z /= polygon->vert_count;

		v0.x = center.x - polygon->vertices[0].position.x;
		v0.y = center.y - polygon->vertices[0].position.y;
		v0.z = center.z - polygon->vertices[0].position.z;

		/* orthonormal basis... */
		v0 = normalize3(v0);
		v1 = cross(v0, polygon->normal);
		//v0 = cross(polygon->normal, v1);

		x_max = -999999999.9;
		y_max = -999999999.9;
		x_min = 999999999.9;
		y_min = 999999999.9;

		for(i = 0; i < polygon->vert_count; i++)
		{
			v.x = center.x - polygon->vertices[i].position.x;
			v.y = center.y - polygon->vertices[i].position.y;
			v.z = center.z - polygon->vertices[i].position.z;

			/* project the vertices onto the
			plane that contains this portal... */
			x = dot3(v, v0);
			y = dot3(v, v1);

			/* ... and keep the axis
			aligned extents... */
			if(x > x_max) x_max = x;
			if(x < x_min) x_min = x;

			if(y > y_max) y_max = y;
			if(y < y_min) y_min = y;
		}

		/* calculate the approximate area of this portal to enable
		selecting the largest amongst duplicated portals... */
		w = x_max - x_min;
		h = y_max - y_min;

		p->approx_area = w * h;

		/* remove if too small... */
	//	if(p->approx_area < 0.05)
	//	{
	//		goto _remove;
	//	}

		/* remove if one of the dimensions is zero... */
		if(!w || !h)
		{
			goto _remove;
		}


		if(w > h)
		{
			denum = w;
			num = h;
		}
		else
		{
			denum = h;
			num = w;
		}

		/* remove if it doesn't have
		a nice ratio between it's sides... */

		/* NOTE: THIS can cause problems... */
		/* NOTE: pesky crack portals... */
	//	if((num / denum) < 0.001)
	//	{
	//		goto _remove;
	//	}

		n = p;
		p = p->next;
	}

	#if 0

	p = *portals;
	while(p)
	{

		n = p->next;
		prev_n = p;
		while(n)
		{
			/* duplicated... */
			if((p->leaf0 == n->leaf0 && p->leaf1 == n->leaf1) ||
			   (p->leaf1 == n->leaf0 && p->leaf0 == n->leaf1))
			{

				prev_n->next = n->next;

				if(p->approx_area >= n->approx_area)
				{
					#ifndef USE_MEMORY_MALLOC
					free(n->portal_polygon->vertices);
					free(n->portal_polygon);
					#else
					memory_Free(n->portal_polygon->vertices);
					memory_Free(n->portal_polygon);
					#endif
				}
				else
				{
					#ifndef USE_MEMORY_MALLOC
					free(p->portal_polygon->vertices);
					free(p->portal_polygon);
					#else
					memory_Free(p->portal_polygon->vertices);
					memory_Free(p->portal_polygon);
					#endif

					p->portal_polygon = n->portal_polygon;
					p->approx_area = n->approx_area;
					p->leaf0 = n->leaf0;
					p->leaf1 = n->leaf1;
				}

				#ifndef USE_MEMORY_MALLOC
				free(n);
				#else
				memory_Free(n);
				#endif
				n = prev_n->next;
				continue;
			}

			prev_n = n;
			n = n->next;
		}

		prev_p = p;
		p = p->next;

	}

	#endif

}

void bsp_LinkBack(bsp_portal_t *portals)
{
	bsp_portal_t *p;
	bsp_leaf_t *leaf;
	p = portals;

	while(p)
	{
		leaf = p->leaf0;
		leaf->portals[leaf->portal_count++] = p;
		assert(leaf->portal_count < MAX_PORTALS_PER_LEAF);

		leaf = p->leaf1;
		leaf->portals[leaf->portal_count++] = p;
		assert(leaf->portal_count < MAX_PORTALS_PER_LEAF);

		p = p->next;
	}

}


void bsp_ClipPortalsToBounds(bsp_node_t *bsp, bsp_portal_t *portals)
{
	int i;
	int c;

	vec3_t max = {-999999999.9, -999999999.9,-999999999.9};
	vec3_t min = { 999999999.9,  999999999.9, 999999999.9};

	float x_max = -50000.9;
	float y_max = -50000.9;
	float z_max = -50000.9;


	float x_min = 50000.9;
	float y_min = 50000.9;
	float z_min = 50000.9;

	bsp_portal_t *p;
	bsp_polygon_t *front;
	bsp_polygon_t *back;

	bsp_BspBounds(bsp, &max, &min);


	/*for(i = 0; i < world_leaves_count; i++)
	{


		max.x = world_leaves[i].center.x + world_leaves[i].extents.x;
		max.y = world_leaves[i].center.y + world_leaves[i].extents.y;
		max.z = world_leaves[i].center.z + world_leaves[i].extents.z;

		min.x = world_leaves[i].center.x - world_leaves[i].extents.x;
		min.y = world_leaves[i].center.y - world_leaves[i].extents.y;
		min.z = world_leaves[i].center.z - world_leaves[i].extents.z;


		if(max.x > x_max) x_max = max.x;
		if(max.y > y_max) y_max = max.y;
		if(max.z > z_max) z_max = max.z;

		if(min.x < x_min) x_min = min.x;
		if(min.y < y_min) y_min = min.y;
		if(min.z < z_min) z_min = min.z;
	}*/

	p = portals;

	x_max = max.x;
	y_max = max.y;
	z_max = max.z;


	x_min = min.x;
	y_min = min.y;
	z_min = min.z;



	x_max += 2.5;
	y_max += 2.5;
	z_max += 2.5;

	x_min -= 2.5;
	y_min -= 2.5;
	z_min -= 2.5;

	while(p)
	//while(0)
	{

		if(bsp_ClassifyPolygon(p->portal_polygon, vec3_t_c(0.0, y_max, 0.0), vec3_t_c(0.0, -1.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3_t_c(0.0, y_max, 0.0), vec3_t_c(0.0, -1.0, 0.0), &front, &back);

			#ifndef USE_MEMORY_MALLOC
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);
			#else
			memory_Free(p->portal_polygon->vertices);
			memory_Free(p->portal_polygon);
			memory_Free(back->vertices);
			memory_Free(back);
			#endif

			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, y_max, 0.0), vec3(0.0, -1.0, 0.0));
		}


		if(bsp_ClassifyPolygon(p->portal_polygon, vec3_t_c(0.0, y_min, 0.0), vec3_t_c(0.0, 1.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3_t_c(0.0, y_min, 0.0), vec3_t_c(0.0, 1.0, 0.0), &front, &back);

			#ifndef USE_MEMORY_MALLOC
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);
			#else
			memory_Free(p->portal_polygon->vertices);
			memory_Free(p->portal_polygon);
			memory_Free(back->vertices);
			memory_Free(back);
			#endif

			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, y_min, 0.0), vec3(0.0, 1.0, 0.0));
		}


		if(bsp_ClassifyPolygon(p->portal_polygon, vec3_t_c(x_max, 0.0, 0.0), vec3_t_c(-1.0, 0.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3_t_c(x_max, 0.0, 0.0), vec3_t_c(-1.0, 0.0, 0.0), &front, &back);

			#ifndef USE_MEMORY_MALLOC
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);
			#else
			memory_Free(p->portal_polygon->vertices);
			memory_Free(p->portal_polygon);
			memory_Free(back->vertices);
			memory_Free(back);
			#endif

			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(x_max, 0.0, 0.0), vec3(-1.0, 0.0, 0.0));
		}


		if(bsp_ClassifyPolygon(p->portal_polygon, vec3_t_c(x_min, 0.0, 0.0), vec3_t_c(1.0, 0.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3_t_c(x_min, 0.0, 0.0), vec3_t_c(1.0, 0.0, 0.0), &front, &back);

			#ifndef USE_MEMORY_MALLOC
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);
			#else
			memory_Free(p->portal_polygon->vertices);
			memory_Free(p->portal_polygon);
			memory_Free(back->vertices);
			memory_Free(back);
			#endif

			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(x_min, 0.0, 0.0), vec3(1.0, 0.0, 0.0));
		}


		if(bsp_ClassifyPolygon(p->portal_polygon, vec3_t_c(0.0, 0.0, z_max), vec3_t_c(0.0, 0.0, -1.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3_t_c(0.0, 0.0, z_max), vec3_t_c(0.0, 0.0, -1.0), &front, &back);

			#ifndef USE_MEMORY_MALLOC
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);
			#else
			memory_Free(p->portal_polygon->vertices);
			memory_Free(p->portal_polygon);
			memory_Free(back->vertices);
			memory_Free(back);
			#endif

			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, 0.0, z_max), vec3(0.0, 0.0, -1.0));
		}


		if(bsp_ClassifyPolygon(p->portal_polygon, vec3_t_c(0.0, 0.0, z_min), vec3_t_c(0.0, 0.0, 1.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3_t_c(0.0, 0.0, z_min), vec3_t_c(0.0, 0.0, 1.0), &front, &back);

			#ifndef USE_MEMORY_MALLOC
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);
			#else
			memory_Free(p->portal_polygon->vertices);
			memory_Free(p->portal_polygon);
			memory_Free(back->vertices);
			memory_Free(back);
			#endif

			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, 0.0, z_min), vec3(0.0, 0.0, 1.0));
		}



		p = p->next;
	}


}


void bsp_LinkPortalAndLeaves(struct bsp_dleaf_t *leaves, bsp_portal_t *portals)
{

}


//void bsp_GeneratePortals(bsp_pnode_t *nodes, bsp_polygon_t *portal_polygons, bsp_portal_t **portals)
void bsp_GeneratePortals(bsp_node_t *bsp, bsp_portal_t **portals)
{
	bsp_portal_t *portal_list = NULL;
	bsp_portal_t *t = NULL;
	bsp_polygon_t *polygon;
	bsp_polygon_t *next;
	bsp_portal_t *p;

	bsp_BuildNodePolygons(bsp, &polygon);


	if(polygon)
	{
		while(polygon)
		{
			next = polygon->next;

			#ifndef USE_MEMORY_MALLOC
			t = malloc(sizeof(bsp_portal_t));
			#else
			t = memory_Malloc(sizeof(bsp_portal_t));
			#endif
			t->leaf0 = NULL;
			t->leaf1 = NULL;
			t->pass_through0 = -1;
			t->pass_through1 = -1;
			t->go_through = -1;
			t->portal_polygon = polygon;
			t->approx_area = -1.0;
			polygon->next = NULL;

			/*if(polygon->vert_count > 200)
			{
				engine_BreakPoint();
			}*/


			t->next = portal_list;
			portal_list = t;

			polygon = next;
		}
	}

	bsp_ClipPortalsToBounds(bsp, portal_list);
	bsp_ClipPortalsToBsp(bsp, &portal_list);
	bsp_RemoveBadPortals(&portal_list);
	bsp_LinkBack(portal_list);

	/*p = portal_list;

	while(p)
	{
		if(p->portal_polygon->vert_count > 200)
		{
			engine_BreakPoint();
		}

		printf("portal polygon: %d\n", p->portal_polygon->vert_count);

		p = p->next;
	}*/

	*portals = portal_list;
	//world_portals = portal_list;
}



#define NODE_POLYGON_SCALE 1000.0
void bsp_RecursiveBuildNodePolygons(bsp_node_t *root, bsp_polygon_t **node_polygons)
{

	bsp_polygon_t *polygon;
	vec3_t v0;
	vec3_t v1;
	vec3_t vertex;

	if(root->type == BSP_LEAF)
		return;

	bsp_RecursiveBuildNodePolygons(root->front, node_polygons);

	#ifndef USE_MEMORY_MALLOC
	polygon = malloc(sizeof(bsp_polygon_t));
	polygon->vertices = malloc(sizeof(vertex_t) * 4);
	#else
	polygon = memory_Malloc(sizeof(bsp_polygon_t));
	polygon->vertices = memory_Malloc(sizeof(vertex_t) * 4);
	#endif

	/* make sure we have an orthonormal basis... */
	v0 = cross(root->tangent, root->normal);
	v1 = cross(v0, root->normal);

	vertex.x = root->point.x - v0.x * NODE_POLYGON_SCALE + v1.x * NODE_POLYGON_SCALE;
	vertex.y = root->point.y - v0.y * NODE_POLYGON_SCALE + v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z - v0.z * NODE_POLYGON_SCALE + v1.z * NODE_POLYGON_SCALE;

	polygon->vertices[3].position = vertex;

	vertex.x = root->point.x - v0.x * NODE_POLYGON_SCALE - v1.x * NODE_POLYGON_SCALE;
	vertex.y = root->point.y - v0.y * NODE_POLYGON_SCALE - v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z - v0.z * NODE_POLYGON_SCALE - v1.z * NODE_POLYGON_SCALE;

	polygon->vertices[2].position = vertex;

	vertex.x = root->point.x + v0.x * NODE_POLYGON_SCALE - v1.x * NODE_POLYGON_SCALE;
	vertex.y = root->point.y + v0.y * NODE_POLYGON_SCALE - v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z + v0.z * NODE_POLYGON_SCALE - v1.z * NODE_POLYGON_SCALE;

	polygon->vertices[1].position = vertex;

	vertex.x = root->point.x + v0.x * NODE_POLYGON_SCALE + v1.x * NODE_POLYGON_SCALE;
	vertex.y = root->point.y + v0.y * NODE_POLYGON_SCALE + v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z + v0.z * NODE_POLYGON_SCALE + v1.z * NODE_POLYGON_SCALE;

	polygon->vertices[0].position = vertex;

	polygon->normal = root->normal;
	polygon->vert_count = 4;

	//poly_count++;

	polygon->next = *node_polygons;
	*node_polygons = polygon;

	bsp_RecursiveBuildNodePolygons(root->back, node_polygons);
}



void bsp_BuildNodePolygons(bsp_node_t *root, bsp_polygon_t **node_polygons)
{
	//poly_count = 0;
	bsp_polygon_t *polygons = NULL;
	bsp_RecursiveBuildNodePolygons(root, &polygons);
	*node_polygons = polygons;
	//printf("poly_count: %d\n", poly_count);
}




void bsp_DeletePortals(bsp_portal_t *portals)
{
	bsp_portal_t *n;

	while(portals)
	{
		n = portals->next;
		#ifndef USE_MEMORY_MALLOC
		free(portals->portal_polygon->vertices);
		free(portals->portal_polygon);
		free(portals);
		#else
		memory_Free(portals->portal_polygon->vertices);
		memory_Free(portals->portal_polygon);
		memory_Free(portals);
		#endif
		portals = n;
	}
}



//pvs_for_leaf_stack *pvs_calc_stack;
//int pvs_calc_stack_pointer;

void bsp_Wait()
{
	SDL_SemWait(step_semaphore);
}

void bsp_Lock()
{
	SDL_LockMutex(polygon_copy_mutex);
}

void bsp_Unlock()
{
	SDL_UnlockMutex(polygon_copy_mutex);
}


#if 0
int bsp_RecursivePvsForLeaf(bsp_leaf_t *src_leaf, bsp_portal_t *src_portal, bsp_leaf_t *dst_leaf, bsp_portal_t **in_valid_dst_portals, int in_valid_dst_portal_count)
{
	int i;
	int j;
	int k;
	int src_poly_vert_count;
	int dst_poly_vert_count;
	int dst_dst_leaf_portal_count;


	int src_side;
	int dst_side;
	int cls;

	recursive_pvs_for_leaf_stack_t *stack;

	//int out_valid_dst_portal_count;
	//bsp_portal_t *valid_portals;
	//bsp_portal_t **out_valid_dst_portals;
	//bsp_portal_t *dst_portal;

	//bsp_leaf_t *dst_dst_leaf;
	//bsp_portal_t *dst_dst_portal;


	//int clipplane_count;
	int rtrn = 0;
	//bsp_clipplane_t *clipplanes;
	//bsp_clipplane_t clipping_planes[MAX_CLIPPING_PLANES];

	//bsp_polygon_t *src_portal_polygon;
	//bsp_polygon_t *dst_portal_polygon;

	bsp_polygon_t *front_split;
	bsp_polygon_t *back_split;

	vec3_t v0;
	vec3_t v1;

	vec3_t e0;
	vec3_t e1;

	vec3_t point;
	vec3_t edge0;
	vec3_t edge1;
	vec3_t normal;

	int src_leaf_side;
	int dst_leaf_side;
	int dst_dst_portal_side;

	bsp_leaf_t *leaf0;
	bsp_leaf_t *leaf1;

	float d0;
	float d1;

	//printf("%f\n", bsp_DeltaTime());

	//if(b_step)
	//{
	//	bsp_Wait();
	//}
	//else
	//{
		/*if(bsp_DeltaTime() > START_TIME_OUT)
		return -1;	*/
//	}




	if(dst_leaf == src_leaf)
		return 0;

	src_leaf->pvs[dst_leaf->leaf_index >> 3] |= 1 << (dst_leaf->leaf_index % 8);
	dst_leaf->pvs[src_leaf->leaf_index >> 3] |= 1 << (src_leaf->leaf_index % 8);

	//return;

	/* this leaf is a terminal leaf... */
	if(!in_valid_dst_portal_count)
		return 0;


	if(dst_leaf->pvs[dst_leaf->leaf_index >> 3] & (1 << (dst_leaf->leaf_index % 8)))
		return 0;



	/* mark this leaf on it's own pvs to avoid cyclic recursion chains... */
	dst_leaf->pvs[dst_leaf->leaf_index >> 3] |= 1 << (dst_leaf->leaf_index % 8);

	//return;
//	printf(">>>leaf %d\n", dst_leaf->leaf_index);

	//pvs_for_leaf_stack.recursive_stack_pointer++;

	assert(pvs_for_leaf_stack->recursive_stack_pointer + 1 < MAX_RECURSIVE_PVS_STACK_DEPTH);



	stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer + 1;

	stack->src_leaf = src_leaf;
	stack->src_portal = src_portal;
	stack->src_portal_polygon = stack->src_portal->portal_polygon;
	src_poly_vert_count = stack->src_portal_polygon->vert_count;


	stack->dst_leaf = dst_leaf;


	pvs_for_leaf_stack->recursive_stack_pointer++;


	//valid_portals = stack->valid_portals;
	//out_valid_dst_portals = stack->out_valid_dst_portals;
	//clipplanes = stack->clipplanes;
	/* alloc this stuff in the heap given that deep enough trees
	will actually cause a stack overflow... */
	//valid_portals = malloc(sizeof(bsp_portal_t ) * (MAX_OUT_VALID_PORTALS + 512));
	//out_valid_dst_portals = malloc(sizeof(bsp_portal_t *) * (MAX_OUT_VALID_PORTALS + 512));
	//clipplanes = malloc(sizeof(bsp_clipplane_t ) * (MAX_CLIP_PLANES + 512));

	for(i = 0; i < in_valid_dst_portal_count; i++)
	{

		if(b_draw_pvs_steps)
			bsp_Lock();

		stack->clipplane_count = 0;
		stack->out_valid_dst_portal_count = 0;


		stack->dst_portal = in_valid_dst_portals[i];
		stack->dst_portal_polygon = stack->dst_portal->portal_polygon;

		dst_poly_vert_count = stack->dst_portal_polygon->vert_count;

		if(b_draw_pvs_steps)
			bsp_Unlock();

	//	if(b_step)
		//	bsp_Wait();

		/* skip coplanar portals... */
		switch(bsp_ClassifyPolygon(stack->dst_portal_polygon, stack->src_portal_polygon->vertices[0].position, stack->src_portal_polygon->normal))
		{
			case POLYGON_CONTAINED_BACK:
			case POLYGON_CONTAINED_FRONT:
				continue;
			break;
		}

		/*for(j = 0; j < src_poly_vert_count; j++)
		{
			for(k = 0; k < dst_poly_vert_count; k++)
			{

			}
		}*/


		/* build clipping planes... */
		for(j = 0; j < src_poly_vert_count; j++)
		{

			assert(src_poly_vert_count > 2);

			for(k = 0; k < dst_poly_vert_count; k++)
			{

				assert(dst_poly_vert_count > 2);
				point = stack->src_portal_polygon->vertices[j].position;

				v0.x = point.x - stack->dst_portal_polygon->vertices[k].position.x;
				v0.y = point.y - stack->dst_portal_polygon->vertices[k].position.y;
				v0.z = point.z - stack->dst_portal_polygon->vertices[k].position.z;

				edge0.x = -v0.x;
				edge0.y = -v0.y;
				edge0.z = -v0.z;


				//point3 = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position;

				v1.x = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.x - stack->dst_portal_polygon->vertices[k].position.x;
				v1.y = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.y - stack->dst_portal_polygon->vertices[k].position.y;
				v1.z = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.z - stack->dst_portal_polygon->vertices[k].position.z;

				edge1.x = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.x - point.x;
				edge1.y = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.y - point.y;
				edge1.z = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.z - point.z;


				if(fabs(dot3(v0, stack->dst_portal_polygon->normal)) < FUZZY_ZERO)
				{
					normal = stack->dst_portal_polygon->normal;
					goto _add_plane0;
				}


				normal = cross(v0, v1);
				normal = normalize3(normal);

				/* is this still necessary? */
			//	if(normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
			//		continue;


				d0 = dot3(stack->src_portal_polygon->normal, edge0);
				d1 = dot3(stack->src_portal_polygon->normal, edge1);


				src_side = bsp_ClassifyPolygon(stack->src_portal_polygon, point, normal);
				dst_side = bsp_ClassifyPolygon(stack->dst_portal_polygon, point, normal);

				cls = src_side | dst_side;

				switch(cls)
				{
					case POLYGON_FRONT | POLYGON_BACK:
					//case POLYGON_FRONT | POLYGON_CONTAINED_BACK:
					//case POLYGON_BACK | POLYGON_CONTAINED_FRONT:
					//case POLYGON_FRONT | POLYGON_STRADDLING:
					//case POLYGON_BACK  | POLYGON_STRADDLING:

					/*	if(cls == (POLYGON_FRONT | POLYGON_STRADDLING) || cls == (POLYGON_BACK | POLYGON_STRADDLING))
						{
							if(dst_side == POLYGON_STRADDLING)
								break;
						}*/

						_add_plane0:

						stack->clipplanes[stack->clipplane_count].point = point;
						stack->clipplanes[stack->clipplane_count].edge0 = edge0;
						stack->clipplanes[stack->clipplane_count].edge1 = edge1;
						stack->clipplanes[stack->clipplane_count].normal = normal;
						stack->clipplane_count++;

					break;

					/*default:

					if(fabs(d0) <= FUZZY_ZERO && fabs(d1) <= FUZZY_ZERO)
					{
						normal.x = -normal.x;
						normal.y = -normal.y;
						normal.z = -normal.z;

						goto _add_plane0;
					}


					break;*/


				/*
					default:

						if(src_side == POLYGON_CONTAINED_FRONT)
							if(dst_side == POLYGON_BACK)
								goto _add_plane0;


						else if(src_side == POLYGON_CONTAINED_BACK)
							if(dst_side == POLYGON_FRONT)
								goto _add_plane0;


					break;*/

				}
			}

			//if(k < dst_poly_vert_count)
			//	break;
		}




		#if 1

		/* build clipping planes... */
		for(j = 0; j < dst_poly_vert_count; j++)
		{

			//assert(src_poly_vert_count > 2);

			for(k = 0; k < src_poly_vert_count; k++)
			{

				//assert(dst_poly_vert_count > 2);
				point = stack->dst_portal_polygon->vertices[j].position;

				//point2 = stack->dst_portal_polygon->vertices[k].position;

				v0.x = point.x - stack->src_portal_polygon->vertices[k].position.x;
				v0.y = point.y - stack->src_portal_polygon->vertices[k].position.y;
				v0.z = point.z - stack->src_portal_polygon->vertices[k].position.z;


				edge0.x = v0.x;
				edge0.y = v0.y;
				edge0.z = v0.z;


				//point3 = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position;

				v1.x = stack->src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.x - stack->src_portal_polygon->vertices[k].position.x;
				v1.y = stack->src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.y - stack->src_portal_polygon->vertices[k].position.y;
				v1.z = stack->src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.z - stack->src_portal_polygon->vertices[k].position.z;

				edge1.x = point.x - stack->src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.x;
				edge1.y = point.y - stack->src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.y;
				edge1.z = point.z - stack->src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.z;


				if(fabs(dot3(v0, stack->src_portal_polygon->normal)) < FUZZY_ZERO)
				{
					normal = stack->src_portal_polygon->normal;
					goto _add_plane1;
				}

				normal = cross(v0, v1);
				normal = normalize3(normal);

				/* is this still necessary? */
				//if(normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
				//	continue;


				d0 = dot3(stack->dst_portal_polygon->normal, edge0);
				d1 = dot3(stack->dst_portal_polygon->normal, edge1);

				src_side = bsp_ClassifyPolygon(stack->src_portal_polygon, point, normal);
				dst_side = bsp_ClassifyPolygon(stack->dst_portal_polygon, point, normal);

				cls = src_side | dst_side;

				switch(cls)
				{
					case POLYGON_FRONT | POLYGON_BACK:
					//case POLYGON_FRONT | POLYGON_CONTAINED_BACK:
					//case POLYGON_BACK | POLYGON_CONTAINED_FRONT:
					//case POLYGON_FRONT | POLYGON_STRADDLING:
					//case POLYGON_BACK  | POLYGON_STRADDLING:

					/*	if(cls == (POLYGON_FRONT | POLYGON_STRADDLING) || cls == (POLYGON_BACK | POLYGON_STRADDLING))
						{
							if(dst_side == POLYGON_STRADDLING)
								break;
						}*/
						_add_plane1:

						stack->clipplanes[stack->clipplane_count].point = point;
						stack->clipplanes[stack->clipplane_count].edge0 = edge0;
						stack->clipplanes[stack->clipplane_count].edge1 = edge1;
						stack->clipplanes[stack->clipplane_count].normal = normal;
						stack->clipplane_count++;

					break;

					/*default:

					if(fabs(d0) <= FUZZY_ZERO && fabs(d1) <= FUZZY_ZERO)
					{
						normal.x = -normal.x;
						normal.y = -normal.y;
						normal.z = -normal.z;

						goto _add_plane1;
					}


					break;*/
				}
			}
		}


		#endif




		src_leaf_side = bsp_ClassifyPoint(stack->src_leaf->center, stack->src_portal->portal_polygon->vertices[0].position, stack->src_portal->portal_polygon->normal);
		dst_leaf_side = bsp_ClassifyPoint(stack->dst_leaf->center, stack->dst_portal->portal_polygon->vertices[0].position, stack->dst_portal->portal_polygon->normal);

		//if(stack->clipplane_count < stack->dst_portal_polygon->vert_count)
		//	continue;

		//if(b_step)
		//	bsp_Wait();



		/*if(!clipplane_count)
			printf("no clip planes!\n");
		else
			printf("clip planes!\n");*/

		/*if(!clipplane_count)
			continue;

		if(clipplane_count < 3)
			continue;*/

		//assert(clipplane_count > 2);



		if(b_step)
		{
			printf("portal from %d to %d generated %d planes\n", stack->src_leaf->leaf_index, stack->dst_leaf->leaf_index, stack->clipplane_count);
			bsp_Wait();
		}


		if(stack->dst_portal->leaf0 == dst_leaf)
		{
			stack->dst_dst_leaf = stack->dst_portal->leaf1;
		}
		else
		{
			stack->dst_dst_leaf = stack->dst_portal->leaf0;
		}



		if(b_step)
		{
			printf("generator leaf %d has %d generator portals\n", stack->dst_dst_leaf->leaf_index, stack->dst_dst_leaf->portal_count);
			bsp_Wait();
		}



		//printf("portal from leaf %d to leaf %d generated %d planes\n", dst_leaf->leaf_index, stack->dst_dst_leaf->leaf_index, stack->clipplane_count);


		/* skip this leaf if it's already in the pvs... */
	/*	if(src_leaf->pvs[dst_dst_leaf->leaf_index >> 3] & (1 << (dst_dst_leaf->leaf_index % 8)))
			continue;*/

		/* skip this leaf if we recursed through it and didn't return yet... */
		if(stack->dst_dst_leaf->pvs[stack->dst_dst_leaf->leaf_index >> 3] & (1 << (stack->dst_dst_leaf->leaf_index % 8)))
			continue;

		dst_dst_leaf_portal_count = stack->dst_dst_leaf->portal_count;



		for(j = 0; j < dst_dst_leaf_portal_count; j++)
		{
			stack->dst_dst_portal = stack->dst_dst_leaf->portals[j];

			/* skip coplanar portals... */
			switch(bsp_ClassifyPolygon(stack->dst_dst_portal->portal_polygon, stack->dst_portal->portal_polygon->vertices[0].position, stack->dst_portal->portal_polygon->normal))
			{
				case POLYGON_CONTAINED_FRONT:
				case POLYGON_CONTAINED_BACK:
					continue;
				break;
			}

			/*if(dst_dst_portal->leaf0 == dst_dst_leaf)
				if(src_leaf->pvs[dst_dst_portal->leaf1->leaf_index >> 3] & (1 << (dst_dst_portal->leaf1->leaf_index % 8)))
					continue;

			if(dst_dst_portal->leaf1 == dst_dst_leaf)
				if(src_leaf->pvs[dst_dst_portal->leaf0->leaf_index >> 3] & (1 << (dst_dst_portal->leaf0->leaf_index % 8)))
					continue;*/

			/* skip this portal if it leads to a leaf
			we recursed through and didn't return from yet...  */
			if(stack->dst_dst_portal->leaf0 == stack->dst_dst_leaf)
				if(stack->dst_dst_portal->leaf1->pvs[stack->dst_dst_portal->leaf1->leaf_index >> 3] & (1 << (stack->dst_dst_portal->leaf1->leaf_index % 8)))
					continue;


			/* skip this portal if it leads to a leaf
			we recursed through and didn't return from yet... */
			if(stack->dst_dst_portal->leaf1 == stack->dst_dst_leaf)
				if(stack->dst_dst_portal->leaf0->pvs[stack->dst_dst_portal->leaf0->leaf_index >> 3] & (1 << (stack->dst_dst_portal->leaf0->leaf_index % 8)))
					continue;

			/* HACK HACK HACK -- skip this portal if we already went through it... */
		//	if(stack->dst_dst_portal->go_through == src_leaf->leaf_index)
		//		continue;


			/* skip this portal if it leads to the source leaf... */
			if(stack->dst_dst_portal->leaf0 == src_leaf || stack->dst_dst_portal->leaf1 == src_leaf)
				continue;


			/* NOTE: if a generator portal straddles the source portal plane, the
			generator portal should be clipped, and only the portion 'outside' the leaf should be
			used, given that the source portal cannot look towards the general direction
			of the source leaf... */

			/* NOTE: THIS CODE IS FAILING!!!! */
			dst_dst_portal_side = bsp_ClassifyPolygon(stack->dst_dst_portal->portal_polygon, stack->src_portal->portal_polygon->vertices[0].position, stack->src_portal->portal_polygon->normal);

			if(dst_dst_portal_side == POLYGON_FRONT)
			{
				if(src_leaf_side == POINT_FRONT)
					continue;
			}
			else if(dst_dst_portal_side == POLYGON_BACK)
			{
				if(src_leaf_side == POINT_BACK)
					continue;
			}

			dst_dst_portal_side = bsp_ClassifyPolygon(stack->dst_dst_portal->portal_polygon, stack->dst_portal->portal_polygon->vertices[0].position, stack->dst_portal->portal_polygon->normal);


			if(dst_dst_portal_side == POLYGON_FRONT)
			{
				if(dst_leaf_side == POINT_FRONT)
					continue;
			}
			else if(dst_dst_portal_side == POLYGON_BACK)
			{
				if(dst_leaf_side == POINT_BACK)
					continue;
			}


			stack->valid_portals[stack->out_valid_dst_portal_count].leaf0 = stack->dst_dst_portal->leaf0;
			stack->valid_portals[stack->out_valid_dst_portal_count].leaf1 = stack->dst_dst_portal->leaf1;

			if(b_draw_pvs_steps)
				bsp_Lock();

			stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon = bsp_DeepCopyPolygon(stack->dst_dst_portal->portal_polygon);


			/* NOTE: even if the clipping planes form a closed clipping volume, portals outside it can be considered
			valid if they get tested against a clipping plane that contains the source portal... */

			for(k = 0; k < stack->clipplane_count; k++)
			{
				src_side = bsp_ClassifyPolygon(stack->src_portal_polygon, stack->clipplanes[k].point, stack->clipplanes[k].normal);
				dst_side = bsp_ClassifyPolygon(stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon, stack->clipplanes[k].point, stack->clipplanes[k].normal);

				if(src_side == dst_side)
				{

					_drop_portal:

					free(stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon->vertices);
					free(stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon);
					break;
				}

				if(dst_side == POLYGON_STRADDLING)
				{

					bsp_SplitPolygon(stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon, stack->clipplanes[k].point, stack->clipplanes[k].normal, &front_split, &back_split);
					free(stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon->vertices);
					free(stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon);

					if(src_side == POLYGON_FRONT)
					{
						free(front_split->vertices);
						free(front_split);
						stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon = back_split;
					}
					else
					{
						//assert(src_side == POLYGON_BACK);

						free(back_split->vertices);
						free(back_split);
						stack->valid_portals[stack->out_valid_dst_portal_count].portal_polygon = front_split;
					}
				}
				else
				{
					if(src_side == POLYGON_CONTAINED_BACK)
						if(dst_side == POLYGON_BACK)
							goto _drop_portal;

					if(src_side == POLYGON_CONTAINED_FRONT)
						if(dst_side == POLYGON_FRONT)
							goto _drop_portal;

					if(dst_side == POLYGON_CONTAINED_FRONT)
						if(src_side == POLYGON_FRONT)
							goto _drop_portal;

					if(dst_side == POLYGON_CONTAINED_BACK)
						if(src_side == POLYGON_BACK)
							goto _drop_portal;

				}


			}

			if(k >= stack->clipplane_count)
			{
				/* TODO: Check to see if the portal has an acceptable area. If it has
				check if it has an acceptable ratio between the sides of it's bounding box. If it
				has, keep it. Drop it, otherwise. */

				leaf0 = stack->valid_portals[stack->out_valid_dst_portal_count].leaf0;
				leaf1 = stack->valid_portals[stack->out_valid_dst_portal_count].leaf1;


				/* make the leaves on both sides of this portal see each other
				(just so the map becomes visible faster)... */
				leaf0->pvs[leaf1->leaf_index >> 3] |= (1 << (leaf1->leaf_index % 8));
				leaf1->pvs[leaf0->leaf_index >> 3] |= (1 << (leaf0->leaf_index % 8));


				stack->valid_portals[stack->out_valid_dst_portal_count].go_through = src_leaf->leaf_index;
				stack->out_valid_dst_portal_count++;
			}

			if(b_draw_pvs_steps)
				bsp_Unlock();
		}

		if(b_draw_pvs_steps)
			bsp_Lock();

		for(j = 0; j < stack->out_valid_dst_portal_count; j++)
		{
			stack->out_valid_dst_portals[j] = &stack->valid_portals[j];
		}


		if(b_draw_pvs_steps)
			bsp_Unlock();


		if(b_step)
		{
			printf("%d valid portals\n", stack->out_valid_dst_portal_count);
			bsp_Wait();
		}


	//	printf("dst_leaf: %d  generator_leaf: %d\n", stack->dst_leaf->leaf_index, stack->dst_dst_leaf->leaf_index);

		//if(stack->out_valid_dst_portal_count)
		//{
		//	printf("leaf %d to leaf %d\n", stack->dst_leaf->leaf_index, stack->dst_dst_leaf->leaf_index);
		//}
		//printf("dst_dst_leaf: %d\n", stack->dst_dst_leaf->leaf_index);



		rtrn = bsp_RecursivePvsForLeaf(stack->src_leaf, stack->src_portal, stack->dst_dst_leaf, stack->out_valid_dst_portals, stack->out_valid_dst_portal_count);

		if(b_draw_pvs_steps)
			bsp_Lock();

		for(j = 0; j < stack->out_valid_dst_portal_count; j++)
		{
			free(stack->valid_portals[j].portal_polygon->vertices);
			free(stack->valid_portals[j].portal_polygon);
		}

		stack->out_valid_dst_portal_count = 0;

		if(b_draw_pvs_steps)
			bsp_Unlock();

		if(rtrn < 0)
			break;

	}

	dst_leaf->pvs[dst_leaf->leaf_index >> 3] &= ~(1 << (dst_leaf->leaf_index % 8));

	_bail:

	//assert(valid_portals);
	//assert(out_valid_dst_portals);
	//assert(clipplanes);

	//free(valid_portals);
	//free(out_valid_dst_portals);
	//free(clipplanes);

	if(b_draw_pvs_steps)
		bsp_Lock();

	pvs_for_leaf_stack->recursive_stack_pointer--;

	stack->dst_dst_leaf = NULL;
	stack->dst_leaf = NULL;
	stack->dst_dst_portal = NULL;
	stack->dst_portal = NULL;
	stack->out_valid_dst_portals[0] = NULL;
	stack->out_valid_dst_portal_count = 0;
	stack->clipplane_count = 0;

	if(b_draw_pvs_steps)
		bsp_Unlock();

//	printf("<<<leaf %d\n", dst_leaf->leaf_index);

	return rtrn;

	/* clear this leaf's bit from it's own pvs, to signal we're done recursing
	through this leaf, and it's safe to go through it again if so is needed... */



}

#endif



int bsp_RecursivePvsForLeaf(bsp_leaf_t *src_leaf, bsp_portal_t *src_portal, bsp_leaf_t *dst_leaf, bsp_portal_t **in_valid_dst_portals, int in_valid_dst_portal_count, float time_out)
{
	int i;
	int j;
	int k;
	int src_poly_vert_count = 0;
	int dst_poly_vert_count = 0;
	int dst_dst_leaf_portal_count = 0;


	int src_side;
	int dst_side;
	int cls;

	//recursive_pvs_for_leaf_stack_t *stack;

	int out_valid_dst_portal_count = 0;
	bsp_portal_t *valid_portals = NULL;
	bsp_portal_t **out_valid_dst_portals = NULL;
	bsp_portal_t *dst_portal = NULL;

	bsp_leaf_t *dst_dst_leaf = NULL;
	bsp_portal_t *dst_dst_portal = NULL;


	int clipplane_count;
	int rtrn = 0;
	bsp_clipplane_t *clipplanes;
	//bsp_clipplane_t clipping_planes[MAX_CLIPPING_PLANES];

	bsp_polygon_t *src_portal_polygon;
	bsp_polygon_t *dst_portal_polygon;

	bsp_polygon_t *front_split;
	bsp_polygon_t *back_split;

	vec3_t v0;
	vec3_t v1;

	vec3_t e0;
	vec3_t e1;

	vec3_t point;
	vec3_t edge0;
	vec3_t edge1;
	vec3_t normal;

	int src_leaf_side;
	int dst_leaf_side;
	int dst_dst_portal_side;

	bsp_leaf_t *leaf0;
	bsp_leaf_t *leaf1;

	float d0;
	float d1;

	if(b_stop)
		return -2;

	//if(bsp_DeltaTime() > time_out)
	//	return -1;

	if(dst_leaf == src_leaf)
		return 0;

	src_leaf->pvs[dst_leaf->leaf_index >> 3] |= 1 << (dst_leaf->leaf_index % 8);
	dst_leaf->pvs[src_leaf->leaf_index >> 3] |= 1 << (src_leaf->leaf_index % 8);

	//return;

	/* this leaf is a terminal leaf... */
	if(!in_valid_dst_portal_count)
		return 0;


	if(dst_leaf->pvs[dst_leaf->leaf_index >> 3] & (1 << (dst_leaf->leaf_index % 8)))
		return 0;



	/* mark this leaf on it's own pvs to avoid cyclic recursion... */
	dst_leaf->pvs[dst_leaf->leaf_index >> 3] |= 1 << (dst_leaf->leaf_index % 8);

	src_portal_polygon = src_portal->portal_polygon;
	src_poly_vert_count = src_portal_polygon->vert_count;

	/* alloc this stuff in the heap given that deep enough trees
	will actually cause a stack overflow... */


	#ifdef LOG_STUFF
	log_LogMessage(LOG_MESSAGE_NOTIFY, "before alloc'ing stuff for current recursive call");
	memory_CheckCorrupted();
	#endif

	#ifndef USE_MEMORY_MALLOC
	valid_portals = malloc(sizeof(bsp_portal_t ) * (256));
	out_valid_dst_portals = malloc(sizeof(bsp_portal_t *) * (256));
	clipplanes = malloc(sizeof(bsp_clipplane_t ) * (256));
	#else
	valid_portals = memory_Malloc(sizeof(bsp_portal_t ) * (256));
	out_valid_dst_portals = memory_Malloc(sizeof(bsp_portal_t *) * (256));
	clipplanes = memory_Malloc(sizeof(bsp_clipplane_t ) * (256));
	#endif

	#ifdef LOG_STUFF
	log_LogMessage(LOG_MESSAGE_NOTIFY, "after alloc'ing stuff for current recursive call");
	memory_CheckCorrupted();
	#endif


	for(i = 0; i < in_valid_dst_portal_count; i++)
	{
		if(in_valid_dst_portals[i]->portal_polygon->vert_count > 10)
		{
			//engine_BreakPoint();
		}
	}


	for(i = 0; i < in_valid_dst_portal_count; i++)
	{

		clipplane_count = 0;
		out_valid_dst_portal_count = 0;


		dst_portal = in_valid_dst_portals[i];
		dst_portal_polygon = dst_portal->portal_polygon;

		dst_poly_vert_count = dst_portal_polygon->vert_count;

		//printf("dst_poly_vert_count %d\n", dst_poly_vert_count);

		/*if(dst_poly_vert_count > 200)
		{
			engine_BreakPoint();
		}*/

		/*if(b_draw_pvs_steps)
			bsp_Unlock();*/

	//	if(b_step)
		//	bsp_Wait();

		/* skip coplanar portals... */
		switch(bsp_ClassifyPolygon(dst_portal_polygon, src_portal_polygon->vertices[0].position, src_portal_polygon->normal))
		{
			case POLYGON_CONTAINED_BACK:
			case POLYGON_CONTAINED_FRONT:
				continue;
			break;
		}


		/* build clipping planes... */
		for(j = 0; j < src_poly_vert_count; j++)
		{

			assert(src_poly_vert_count > 2);

			for(k = 0; k < dst_poly_vert_count; k++)
			{

				assert(dst_poly_vert_count > 2);
				point = src_portal_polygon->vertices[j].position;

				v0.x = point.x - dst_portal_polygon->vertices[k].position.x;
				v0.y = point.y - dst_portal_polygon->vertices[k].position.y;
				v0.z = point.z - dst_portal_polygon->vertices[k].position.z;

				edge0.x = -v0.x;
				edge0.y = -v0.y;
				edge0.z = -v0.z;


				//point3 = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position;

				v1.x = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.x - dst_portal_polygon->vertices[k].position.x;
				v1.y = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.y - dst_portal_polygon->vertices[k].position.y;
				v1.z = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.z - dst_portal_polygon->vertices[k].position.z;

				edge1.x = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.x - point.x;
				edge1.y = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.y - point.y;
				edge1.z = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position.z - point.z;


				if(fabs(dot3(v0, dst_portal_polygon->normal)) < FUZZY_ZERO)
				{
					continue;
					//normal = dst_portal_polygon->normal;
					//goto _add_plane0;
				}


				normal = cross(v0, v1);
				normal = normalize3(normal);

				//d0 = dot3(src_portal_polygon->normal, edge0);
				//d1 = dot3(src_portal_polygon->normal, edge1);


				src_side = bsp_ClassifyPolygon(src_portal_polygon, point, normal);
				dst_side = bsp_ClassifyPolygon(dst_portal_polygon, point, normal);

				cls = src_side | dst_side;

				switch(cls)
				{
					case POLYGON_FRONT | POLYGON_BACK:
					case POLYGON_CONTAINED_FRONT | POLYGON_CONTAINED_BACK:

						_add_plane0:

						clipplanes[clipplane_count].point = point;
						clipplanes[clipplane_count].edge0 = edge0;
						clipplanes[clipplane_count].edge1 = edge1;
						clipplanes[clipplane_count].normal = normal;
						clipplane_count++;

					break;

				}
			}

			//if(k < dst_poly_vert_count)
			//	break;
		}


		/* build clipping planes... */
		for(j = 0; j < dst_poly_vert_count; j++)
		{

			for(k = 0; k < src_poly_vert_count; k++)
			{

				//assert(dst_poly_vert_count > 2);
				point = dst_portal_polygon->vertices[j].position;

				//point2 = stack->dst_portal_polygon->vertices[k].position;

				v0.x = point.x - src_portal_polygon->vertices[k].position.x;
				v0.y = point.y - src_portal_polygon->vertices[k].position.y;
				v0.z = point.z - src_portal_polygon->vertices[k].position.z;


				edge0.x = v0.x;
				edge0.y = v0.y;
				edge0.z = v0.z;


				//point3 = stack->dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].position;

				v1.x = src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.x - src_portal_polygon->vertices[k].position.x;
				v1.y = src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.y - src_portal_polygon->vertices[k].position.y;
				v1.z = src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.z - src_portal_polygon->vertices[k].position.z;

				edge1.x = point.x - src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.x;
				edge1.y = point.y - src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.y;
				edge1.z = point.z - src_portal_polygon->vertices[(k + 1) % src_poly_vert_count].position.z;


				if(fabs(dot3(v0, src_portal_polygon->normal)) < FUZZY_ZERO)
				{
					continue;
					//normal = src_portal_polygon->normal;
					//goto _add_plane1;
				}

				normal = cross(v0, v1);
				normal = normalize3(normal);

				//d0 = dot3(dst_portal_polygon->normal, edge0);
				//d1 = dot3(dst_portal_polygon->normal, edge1);

				src_side = bsp_ClassifyPolygon(src_portal_polygon, point, normal);
				dst_side = bsp_ClassifyPolygon(dst_portal_polygon, point, normal);

				cls = src_side | dst_side;

				switch(cls)
				{
					case POLYGON_FRONT | POLYGON_BACK:
					case POLYGON_CONTAINED_FRONT | POLYGON_CONTAINED_BACK:
						_add_plane1:

						clipplanes[clipplane_count].point = point;
						clipplanes[clipplane_count].edge0 = edge0;
						clipplanes[clipplane_count].edge1 = edge1;
						clipplanes[clipplane_count].normal = normal;
						clipplane_count++;

					break;

				}
			}
		}





		src_leaf_side = bsp_ClassifyPoint(src_leaf->center, src_portal->portal_polygon->vertices[0].position, src_portal->portal_polygon->normal);
		dst_leaf_side = bsp_ClassifyPoint(dst_leaf->center, dst_portal->portal_polygon->vertices[0].position, dst_portal->portal_polygon->normal);

		if(dst_portal->leaf0 == dst_leaf)
		{
			dst_dst_leaf = dst_portal->leaf1;
		}
		else
		{
			dst_dst_leaf = dst_portal->leaf0;
		}

		/* skip this leaf if we recursed through it and didn't return yet... */
		//if(dst_dst_leaf->pvs[dst_dst_leaf->leaf_index >> 3] & (1 << (dst_dst_leaf->leaf_index % 8)))
		//	continue;

		dst_dst_leaf_portal_count = dst_dst_leaf->portal_count;

		/*for(j = 0; j < dst_dst_leaf_portal_count; j++)
		{
			dst_dst_portal = dst_dst_leaf->portals[j];

			if(dst_dst_portal->portal_polygon->vert_count > 10)
			{
				engine_BreakPoint();
			}

		}*/


		for(j = 0; j < dst_dst_leaf_portal_count; j++)
		{
			dst_dst_portal = dst_dst_leaf->portals[j];

		/*	if(dst_dst_portal->portal_polygon->vert_count > 200)
			{
				engine_BreakPoint();
			}*/

			/* skip coplanar portals... */
			switch(bsp_ClassifyPolygon(dst_dst_portal->portal_polygon, dst_portal->portal_polygon->vertices[0].position, dst_portal->portal_polygon->normal))
			{
				case POLYGON_CONTAINED_FRONT:
				case POLYGON_CONTAINED_BACK:
					continue;
				break;
			}


			/* skip this portal if it leads to a leaf
			we recursed through and didn't return from yet...  */
			//if(dst_dst_portal->leaf0 == dst_dst_leaf)
			//	if(dst_dst_portal->leaf1->pvs[dst_dst_portal->leaf1->leaf_index >> 3] & (1 << (dst_dst_portal->leaf1->leaf_index % 8)))
			//		continue;


			/* skip this portal if it leads to a leaf
			we recursed through and didn't return from yet... */
			//if(dst_dst_portal->leaf1 == dst_dst_leaf)
			//	if(dst_dst_portal->leaf0->pvs[dst_dst_portal->leaf0->leaf_index >> 3] & (1 << (dst_dst_portal->leaf0->leaf_index % 8)))
			//		continue;


			/* skip this portal if it leads to the source leaf... */
			if(dst_dst_portal->leaf0 == src_leaf || dst_dst_portal->leaf1 == src_leaf)
				continue;


			/* NOTE: if a generator portal straddles the source portal plane, the
			generator portal should be clipped, and only the portion 'outside' the leaf should be
			used, given that the source portal cannot look towards the general direction
			of the source leaf... */

			/* NOTE: THIS CODE IS FAILING!!!! */
			/*dst_dst_portal_side = bsp_ClassifyPolygon(dst_dst_portal->portal_polygon, src_portal->portal_polygon->vertices[0].position, src_portal->portal_polygon->normal);

			if(dst_dst_portal_side == POLYGON_FRONT)
			{
				if(src_leaf_side == POINT_FRONT)
					continue;
			}
			else if(dst_dst_portal_side == POLYGON_BACK)
			{
				if(src_leaf_side == POINT_BACK)
					continue;
			}

			dst_dst_portal_side = bsp_ClassifyPolygon(dst_dst_portal->portal_polygon, dst_portal->portal_polygon->vertices[0].position, dst_portal->portal_polygon->normal);


			if(dst_dst_portal_side == POLYGON_FRONT)
			{
				if(dst_leaf_side == POINT_FRONT)
					continue;
			}
			else if(dst_dst_portal_side == POLYGON_BACK)
			{
				if(dst_leaf_side == POINT_BACK)
					continue;
			}*/


			valid_portals[out_valid_dst_portal_count].leaf0 = dst_dst_portal->leaf0;
			valid_portals[out_valid_dst_portal_count].leaf1 = dst_dst_portal->leaf1;

			#ifdef LOG_STUFF
			log_LogMessage(LOG_MESSAGE_NOTIFY, "before bsp_DeepCopyPolygon");
			memory_CheckCorrupted();
			#endif

			valid_portals[out_valid_dst_portal_count].portal_polygon = bsp_DeepCopyPolygon(dst_dst_portal->portal_polygon);

			#ifdef LOG_STUFF
			log_LogMessage(LOG_MESSAGE_NOTIFY, "after bsp_DeepCopyPolygon");
			memory_CheckCorrupted();
			#endif

			/* NOTE: even if the clipping planes form a closed clipping volume, portals outside it can be considered
			valid if they get tested against a clipping plane that contains the source portal... */

			for(k = 0; k < clipplane_count; k++)
			{
				src_side = bsp_ClassifyPolygon(src_portal_polygon, clipplanes[k].point, clipplanes[k].normal);
				dst_side = bsp_ClassifyPolygon(valid_portals[out_valid_dst_portal_count].portal_polygon, clipplanes[k].point, clipplanes[k].normal);

				if(src_side == dst_side)
				{

					_drop_portal:

					#ifdef LOG_STUFF
					log_LogMessage(LOG_MESSAGE_NOTIFY, "before dropping portal");
					memory_CheckCorrupted();
					#endif

					#ifndef USE_MEMORY_MALLOC
					free(valid_portals[out_valid_dst_portal_count].portal_polygon->vertices);
					free(valid_portals[out_valid_dst_portal_count].portal_polygon);
					#else
					memory_Free(valid_portals[out_valid_dst_portal_count].portal_polygon->vertices);
					memory_Free(valid_portals[out_valid_dst_portal_count].portal_polygon);
					#endif

					#ifdef LOG_STUFF
					log_LogMessage(LOG_MESSAGE_NOTIFY, "after dropping portal");
					memory_CheckCorrupted();
					#endif


					break;
				}

				if(dst_side == POLYGON_STRADDLING)
				{

					#ifdef LOG_STUFF
					log_LogMessage(LOG_MESSAGE_NOTIFY, "before splitting portal polygon");
					memory_CheckCorrupted();
					#endif

					bsp_SplitPolygon(valid_portals[out_valid_dst_portal_count].portal_polygon, clipplanes[k].point, clipplanes[k].normal, &front_split, &back_split);

					#ifdef LOG_STUFF
					log_LogMessage(LOG_MESSAGE_NOTIFY, "after splitting portal polygon");
					memory_CheckCorrupted();
					#endif


					#ifdef LOG_STUFF
					log_LogMessage(LOG_MESSAGE_NOTIFY, "before dropping old portal polygon");
					memory_CheckCorrupted();
					#endif

					#ifndef USE_MEMORY_MALLOC
					free(valid_portals[out_valid_dst_portal_count].portal_polygon->vertices);
					free(valid_portals[out_valid_dst_portal_count].portal_polygon);
					#else
					memory_Free(valid_portals[out_valid_dst_portal_count].portal_polygon->vertices);
					memory_Free(valid_portals[out_valid_dst_portal_count].portal_polygon);
					#endif

					#ifdef LOG_STUFF
					log_LogMessage(LOG_MESSAGE_NOTIFY, "after dropping old portal polygon");
					memory_CheckCorrupted();
					#endif


					if(src_side == POLYGON_FRONT)
					{
						#ifdef LOG_STUFF
						log_LogMessage(LOG_MESSAGE_NOTIFY, "before dropping front split");
						memory_CheckCorrupted();
						#endif

						#ifndef USE_MEMORY_MALLOC
						free(front_split->vertices);
						free(front_split);
						#else
						memory_Free(front_split->vertices);
						memory_Free(front_split);
						#endif

						#ifdef LOG_STUFF
						log_LogMessage(LOG_MESSAGE_NOTIFY, "after dropping front split");
						memory_CheckCorrupted();
						#endif

						valid_portals[out_valid_dst_portal_count].portal_polygon = back_split;
					}
					else
					{

						#ifdef LOG_STUFF
						log_LogMessage(LOG_MESSAGE_NOTIFY, "before dropping back split");
						memory_CheckCorrupted();
						#endif

						#ifndef USE_MEMORY_MALLOC
						free(back_split->vertices);
						free(back_split);
						#else
						memory_Free(back_split->vertices);
						memory_Free(back_split);
						#endif

						#ifdef LOG_STUFF
						log_LogMessage(LOG_MESSAGE_NOTIFY, "after dropping back split");
						memory_CheckCorrupted();
						#endif

						valid_portals[out_valid_dst_portal_count].portal_polygon = front_split;
					}
				}
			}

			if(k >= clipplane_count)
			{
				out_valid_dst_portal_count++;
			}
		}

		for(j = 0; j < out_valid_dst_portal_count; j++)
		{
			out_valid_dst_portals[j] = &valid_portals[j];

		/*	if(out_valid_dst_portals[j]->portal_polygon->vert_count > 200)
			{
				engine_BreakPoint();
			}*/

		}




		rtrn = bsp_RecursivePvsForLeaf(src_leaf, src_portal, dst_dst_leaf, out_valid_dst_portals, out_valid_dst_portal_count, time_out);

		#ifdef LOG_STUFF
		log_LogMessage(LOG_MESSAGE_NOTIFY, "before freeing old valid portals");
		memory_CheckCorrupted();
		#endif

		for(j = 0; j < out_valid_dst_portal_count; j++)
		{
			#ifndef USE_MEMORY_MALLOC
			free(valid_portals[j].portal_polygon->vertices);
			free(valid_portals[j].portal_polygon);
			#else
			memory_Free(valid_portals[j].portal_polygon->vertices);
			memory_Free(valid_portals[j].portal_polygon);
			#endif
		}

		#ifdef LOG_STUFF
		log_LogMessage(LOG_MESSAGE_NOTIFY, "after freeing old valid portals");
		memory_CheckCorrupted();
		#endif

		out_valid_dst_portal_count = 0;

		if(rtrn < 0)
			break;

	}

	dst_leaf->pvs[dst_leaf->leaf_index >> 3] &= ~(1 << (dst_leaf->leaf_index % 8));

	_bail:
	#ifdef LOG_STUFF
	log_LogMessage(LOG_MESSAGE_NOTIFY, "before freeing stuff for current recursive call");
	memory_CheckCorrupted();
	#endif

	#ifndef USE_MEMORY_MALLOC
	free(valid_portals);
	free(out_valid_dst_portals);
	free(clipplanes);
	#else
	memory_Free(valid_portals);
	memory_Free(out_valid_dst_portals);
	memory_Free(clipplanes);
	#endif

	#ifdef LOG_STUFF
	log_LogMessage(LOG_MESSAGE_NOTIFY, "after freeing stuff for current recursive call");
	memory_CheckCorrupted();
	#endif



	//printf("<<<leaf %d\n", dst_leaf->leaf_index);

	return rtrn;

	/* clear this leaf's bit from it's own pvs, to signal we're done recursing
	through this leaf, and it's safe to go through it again if so is needed... */



}



#if 0

void bsp_PvsForLeaf(bsp_leaf_t *leaf)
{
	int i;
	int c;

	int j;
	int k;

	float elapsed;

	int leaf_index;
	//bsp_portal_t **portals;
	//bsp_leaf_t *dst_leaf;

	//int in_dst_portal_count = 0;
	//bsp_portal_t *in_dst_portals[512];

	c = leaf->portal_count;

	//pvs_for_leaf_stack.portals = leaf->portals;
	pvs_for_leaf_stack = leaf->stack;
	pvs_for_leaf_stack->portals = leaf->portals;



	printf("leaf %d... ", leaf->leaf_index);

	/*for(i = 0; i < leaf->pvs_size; i++)
	{
		leaf->pvs[i] = 0;
	}*/

	bsp_InitPvsTimer();

	//leaf->pvs[leaf->leaf_index >> 3] &= ~(1 << (leaf->leaf_index % 8));


	for(i = 0; i < c; i++)
	{
		//portal_clip_plane_count = 0;

		if(b_step)
		{
			bsp_Wait();
		}

		if(pvs_for_leaf_stack->portals[i]->leaf0 != leaf)
		{
			pvs_for_leaf_stack->dst_leaf = pvs_for_leaf_stack->portals[i]->leaf0;
		}
		else
		{
			pvs_for_leaf_stack->dst_leaf = pvs_for_leaf_stack->portals[i]->leaf1;
		}

		/* add the leaf connected to this portal to the
		src leaf's pvs, and recurse down... */
		//leaf_index = dst_leaf->leaf_index;
		//leaf->pvs[leaf_index >> 3] |= 1 << (leaf_index % 8);

		pvs_for_leaf_stack->in_dst_portal_count = 0;
		k = pvs_for_leaf_stack->dst_leaf->portal_count;

		for(j = 0; j < k; j++)
		{
			if(pvs_for_leaf_stack->dst_leaf->portals[j] != pvs_for_leaf_stack->portals[i])
			{
				pvs_for_leaf_stack->in_dst_portals[pvs_for_leaf_stack->in_dst_portal_count] = pvs_for_leaf_stack->dst_leaf->portals[j];

				//printf("%x\n", in_dst_portals[in_dst_portal_count]);

				pvs_for_leaf_stack->in_dst_portal_count++;

			}
		}

		if(b_step)
		{
			bsp_Wait();
			printf("dst_leaf: %d\n", pvs_for_leaf_stack->dst_leaf->leaf_index);
		}




		if(b_step)
			bsp_Wait();

		if(bsp_RecursivePvsForLeaf(leaf, pvs_for_leaf_stack->portals[i], pvs_for_leaf_stack->dst_leaf, pvs_for_leaf_stack->in_dst_portals, pvs_for_leaf_stack->in_dst_portal_count) < 0)
		{
			printf("timed out!\n", leaf->leaf_index);
			return;
			/* this leaf timed out... */
		}

	}

	elapsed = bsp_DeltaTime();

	printf("ok! (%f s)\n", elapsed / 1000.0);
}

#endif



int bsp_PvsForLeaf(bsp_leaf_t *leaf, float time_out, int portal_index)
{
	int i;
	int c;

	int j;
	int k;

	float elapsed;

	int leaf_index;
	bsp_portal_t **portals;
	bsp_leaf_t *dst_leaf;
	bsp_portal_t *p;

	int in_dst_portal_count = 0;
	bsp_portal_t *in_dst_portals[64];

	c = leaf->portal_count;

	//pvs_for_leaf_stack.portals = leaf->portals;
	//pvs_for_leaf_stack = leaf->stack;
	portals = leaf->portals;


	p = world_portals;

	printf("leaf %d... ", leaf->leaf_index);

	/*for(i = 0; i < leaf->pvs_size; i++)
	{
		leaf->pvs[i] = 0;
	}*/

	bsp_InitPvsTimer();

	//leaf->pvs[leaf->leaf_index >> 3] &= ~(1 << (leaf->leaf_index % 8));
	/*while(p)
	{
		printf("%d\n", p->portal_polygon->vert_count);
		p = p->next;
	}*/

	#ifdef LOG_STUFF
	log_LogMessage(LOG_MESSAGE_NOTIFY, "before calculating pvs for leaf %d", leaf->leaf_index);
	#endif
	//memory_CheckCorrupted();

	for(i = portal_index; i < c; i++)
	{

		if(portals[i]->leaf0 != leaf)
		{
			dst_leaf = portals[i]->leaf0;
		}
		else
		{
			dst_leaf = portals[i]->leaf1;
		}

		in_dst_portal_count = 0;
		k = dst_leaf->portal_count;

		for(j = 0; j < k; j++)
		{
			if(dst_leaf->portals[j] != portals[i])
			{
				in_dst_portals[in_dst_portal_count] = dst_leaf->portals[j];

				/*if(dst_leaf->portals[j]->portal_polygon->vert_count > 10)
				{
					engine_BreakPoint();
				}*/

				//printf("%x\n", in_dst_portals[in_dst_portal_count]->portal_polygon);

				in_dst_portal_count++;

			}
		}

		if(bsp_RecursivePvsForLeaf(leaf, portals[i], dst_leaf, in_dst_portals, in_dst_portal_count, time_out) < 0)
		{
			//printf("timed out! (%.02f s)\n", time_out / 1000.0);
			return i;
			/* this leaf timed out... */
		}

		//memory_CheckCorrupted();

	}

	#ifdef LOG_STUFF
	log_LogMessage(LOG_MESSAGE_NOTIFY, "after calculating pvs for leaf %d", leaf->leaf_index);
	#endif
	//memory_CheckCorrupted();

	elapsed = bsp_DeltaTime();

	printf("ok! (%.02f s)\n", elapsed / 1000.0);

	//w_world_leaves[leaf->leaf_index].pvs = leaf->pvs;

	w_world_leaves[leaf->leaf_index].pvs = bsp_CompressPvs(leaf->pvs, 1 + (w_world_leaves_count >> 3), &w_world_leaves[leaf->leaf_index].pvs_lenght);


	return -1;

}




void bsp_RecursivePvsForLeaves(bsp_node_t *bsp)
{
	bsp_leaf_t *leaf;

	if(bsp->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)bsp;
		if(!(leaf->bm_flags & BSP_SOLID))
		{
			bsp_PvsForLeaf(leaf, 300000.0, 0);
		}
	}
	else
	{
		bsp_RecursivePvsForLeaves(bsp->front);
		bsp_RecursivePvsForLeaves(bsp->back);
	}

}

void bsp_PvsForLeaves(bsp_node_t *bsp, bsp_portal_t *portals)
{

	int leaf_count = 0;
	int node_count = 0;
	int pvs_size;
	int i;
	bsp_portal_t *portal = portals;
	bsp_leaf_t *leaf0;
	bsp_leaf_t *leaf1;
	bsp_leaf_t *leaf;
	pvs_job_t *job;

	//SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

	//while(leaf = bsp_GetNextLeaf())
	//while(job =  bsp_GetNextJob())
	//{

 	//	i = bsp_PvsForLeaf(job->leaf, job->time_out, job->src_portal_index);

	//	if(i > -1)
	//	{
	//		job->src_portal_index = i;
	//		printf("timed out after %.02fs and has been requeued...\n", job->time_out / 1000.0);
	//		bsp_RequeueJob(job);
	//	}
	//	else
	//	{
	//		memory_Free(job);
	//	}

	//	if(b_stop)
	//	{
	//		printf("pvs calculation stopped!\n");
	//		return;
	//	}

//	}

	//SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);

	bsp_RecursivePvsForLeaves(bsp);

}

int bsp_PvsForLeavesThreadFn(void *data)
{
	int leaf_count = 0;
	int node_count = 0;
	int pvs_size;
	int i;
	//bsp_portal_t *portal = portals;
	bsp_leaf_t *leaf0;
	bsp_leaf_t *leaf1;
	bsp_leaf_t *leaf;
	pvs_job_t *job;

	while(job =  bsp_GetNextJob())
	{

		i = bsp_PvsForLeaf(job->leaf, job->time_out, job->src_portal_index);

		if(i > -1)
		{
			job->src_portal_index = i;
			bsp_RequeueJob(job);
			printf("leaf %d requeued, will restart on portal %d...\n", job->leaf->leaf_index, i);
		}
		else
		{
			#ifndef USE_MEMORY_MALLOC
			free(job);
			#else
			memory_Free(job);
			#endif
		}

		if(b_stop)
		{
			printf("pvs calculation stopped!\n");
			return 0;
		}
	}

}



//#define EMPTY_PVS
#define TRACE_COUNT 8
void bsp_CalculatePvs(bsp_node_t *bsp)
{
	int i;
	int c;
	int j;
	int k;
	int t;
	int pvs_size;
	vec3_t end;
	trace_t trace;
	bsp_portal_t *portals = NULL;
	bsp_leaf_t *leaf0;
	bsp_leaf_t *leaf1;
	int leaf_count = 0;
	int node_count = 0;
	int cpu_count;
	//timedout_leaf_t *timedout = timed_out_leaves;
	//timedout_leaf_t *next;
	//c = leaf_count;

	//pvs_size = 1 + c >> 5;

	b_calculating_pvs = 1;
	b_stop = 0;


	/*if(!stop_flag_mutex)
	{
		stop_flag_mutex = SDL_CreateMutex();
	}*/

	job_list_mutex = SDL_CreateMutex();
	cpu_count = SDL_GetCPUCount();


	//pvs_for_leaf_stack.recursive_stack = malloc(sizeof(recursive_pvs_for_leaf_stack_t) * MAX_RECURSIVE_PVS_STACK);
	//pvs_for_leaf_stack.recursive_stack_pointer = -1;
	//step_mutex = SDL_CreateMutex();
	//step_semaphore = SDL_CreateSemaphore(0);

	//pvs_calc_stack = malloc(sizeof(pvs_for_leaf_stack) * 1024);
	//pvs_calc_stack_pointer = 0;

	job_list_mutex = SDL_CreateMutex();

	//printf("before bsp_PvsForLeaves...\n");
	//log_LogMessage(LOG_MESSAGE_NOTIFY, "before bsp_PvsForLeaves...");
	//memory_CheckCorrupted();
	bsp_GeneratePortals(bsp, &world_portals);
	//memory_CheckCorrupted();
	bsp_BuildPvsJobList(bsp);
	bsp_PvsForLeaves(bsp, world_portals);

	//printf("after bsp_PvsForLeaves...\n");
	//log_LogMessage(LOG_MESSAGE_NOTIFY, "after bsp_PvsForLeaves...");
	//memory_CheckCorrupted();
	bsp_DeletePvsJobList();



	SDL_DestroyMutex(job_list_mutex);
	/*while(timedout)
	{
		next = (timedout_leaf_t *)timedout->next;
		free(timedout);
		timedout = next;
	}
	*/

	//free(pvs_for_leaf_stack->recursive_stack);

	//SDL_DestroyMutex(step_mutex);
	//SDL_DestroySemaphore(step_semaphore);
	//free(pvs_calc_stack);

	b_calculating_pvs = 0;

}

int bsp_CalculatePvsAssync(void *data)
{
	printf("pvs thread has started\n");
	bsp_CalculatePvs((bsp_node_t *)data);
	light_ClearLightLeaves();

	printf("pvs thread has returned\n");

	return 0;
}

void bsp_BuildPvsJobList(bsp_node_t *bsp)
{
	pvs_job_t *job;
	bsp_leaf_t *leaf;
	if(bsp->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)bsp;
		if(!(leaf->bm_flags & BSP_SOLID))
		{
			#ifndef USE_MEMORY_MALLOC
			job = malloc(sizeof(pvs_job_t));
			#else
			job = memory_Malloc(sizeof(pvs_job_t));
			#endif
			job->leaf = leaf;
			job->time_out = SMALL_START_TIME_OUT;
			job->src_portal_index = 0;
			//job->last_time_out = 0.0;
			job->next = NULL;

			if(!pvs_jobs)
			{
				pvs_jobs = job;
			}
			else
			{
				last_job->next = job;
			}

			last_job = job;

			//leaf->stack = malloc(sizeof(pvs_for_leaf_stack_t));

			//leaf->stack->recursive_stack_pointer = -1;
			//leaf->stack->recursive_stack = malloc(sizeof(recursive_pvs_for_leaf_stack_t) * MAX_RECURSIVE_PVS_STACK_DEPTH);

			//assert(leaf->stack->recursive_stack);
		}
	}
	else
	{
		bsp_BuildPvsJobList(bsp->front);
		bsp_BuildPvsJobList(bsp->back);
	}

}

bsp_leaf_t *bsp_GetNextLeaf()
{
	/*bsp_leaf_t *leaf = NULL;
	pvs_job_t *next;

	if(pvs_jobs)
	{
		leaf = pvs_jobs->leaf;
		next = pvs_jobs->next;
		free(pvs_jobs);
		pvs_jobs = next;

		if(!pvs_jobs)
			last_job = NULL;

	}

	return leaf;*/
}

pvs_job_t *bsp_GetNextJob()
{
	//bsp_leaf_t *leaf = NULL;
	pvs_job_t *next;
	pvs_job_t *job = NULL;

	SDL_LockMutex(job_list_mutex);

	if(pvs_jobs)
	{
		job = pvs_jobs;
		pvs_jobs = pvs_jobs->next;

		job->next = NULL;

		if(!pvs_jobs)
			last_job = NULL;
	}

	SDL_UnlockMutex(job_list_mutex);

	return job;
}

void bsp_RequeueLeaf(bsp_leaf_t *leaf)
{
	/*pvs_job_t *job;
	if(leaf)
	{
		SDL_LockMutex(job_list_mutex);

		if(pvs_jobs)
		{
			job = malloc(sizeof(pvs_job_t));
			job->next = NULL;
			job->leaf = leaf;
			job->src_portal_index = -1;

			if(!pvs_jobs)
			{
				pvs_jobs = job;
			}
			else
			{
				last_job->next = job;
			}
			last_job = job;
		}

		SDL_UnlockMutex(job_list_mutex);
	}*/
}

void bsp_RequeueJob(pvs_job_t *job)
{
	if(job)
	{
		if(job->time_out < START_TIME_OUT)
		{
		//	if(job->time_out == SMALL_START_TIME_OUT)
		//	{
		//		job->time_out += 5000.0;
		//	}

			job->time_out += SMALL_TIME_OUT_INCREMENT;
		}
		else
		{
			job->time_out += TIME_OUT_INCREMENT;
		}


		if(job->time_out > MAX_TIME_OUT)
		{
			#ifndef USE_MEMORY_MALLOC
			free(job);
			#else
			memory_Free(job);
			#endif
			return;
		}

		if(pvs_jobs)
		{
			last_job->next = job;
			last_job = job;
		}
		else
		{
			pvs_jobs = job;
			last_job = job;
		}
	}
}

void bsp_DeletePvsJobList()
{
	while(pvs_jobs)
	{
		last_job = pvs_jobs->next;
		//free(pvs_jobs->leaf->stack->recursive_stack);
		//free(pvs_jobs->leaf->stack);
		#ifndef USE_MEMORY_MALLOC
		free(pvs_jobs);
		#else
		memory_Free(pvs_jobs);
		#endif
		pvs_jobs = last_job;
	}

	pvs_jobs = NULL;
	last_job = NULL;
}

void bsp_Stop()
{
	b_stop = 1;
}


void bsp_AllocPvs(bsp_node_t *bsp, bsp_portal_t *portals)
{
	/*int leaf_count = 0;
	int node_count = 0;
	int pvs_size;
	int i;
	bsp_portal_t *portal = portals;
	bsp_leaf_t *leaf0;
	bsp_leaf_t *leaf1;

	bsp_CountNodesAndLeaves(bsp, &leaf_count, &node_count);
	pvs_size = 4 + (leaf_count >> 3);

	while(portal)
	{
		leaf0 = portal->leaf0;
		leaf1 = portal->leaf1;

		if(!leaf0->pvs)
		{
			leaf0->pvs = malloc(pvs_size);
			for(i = 0; i < pvs_size; i++)
			{
				leaf0->pvs[i] = 0;
			}
		}

		if(!leaf1->pvs)
		{
			leaf1->pvs = malloc(pvs_size);
			for(i = 0; i < pvs_size; i++)
			{
				leaf1->pvs[i] = 0;
			}
		}

		portal = portal->next;
	}	*/
}


void bsp_ApproximatePvsForLeaf(bsp_leaf_t *src_leaf, vec3_t *src_center, bsp_node_t *node, bsp_node_t *bsp)
{
	bsp_leaf_t *leaf;
	vec3_t dst_center;
	vec3_t dst_maxs = {-99999999999.9, -99999999999.9, -99999999999.9};
	vec3_t dst_mins = {99999999999.9, 99999999999.9, 99999999999.9};
	vec3_t dst_corner;

	bsp_polygon_t *polygons;
	int i;
	int vert_count;
	int total_vert_count = 0;


	static float checks[] = {-1.0, 1.0, 1.0,
							 -1.0,-1.0, 1.0,
							  1.0,-1.0, 1.0,
							  1.0, 1.0, 1.0,

							 -1.0, 1.0, -1.0,
							 -1.0,-1.0, -1.0,
							  1.0,-1.0, -1.0,
							  1.0, 1.0, -1.0};

	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;
		if(leaf == src_leaf)
			return;

		if(!(leaf->bm_flags & BSP_SOLID))
		{

			dst_center.x = 0.0;
			dst_center.y = 0.0;
			dst_center.z = 0.0;

			polygons = leaf->polygons;

			while(polygons)
			{

				vert_count = polygons->vert_count;

				for(i = 0; i < vert_count; i++)
				{
					/*dst_center.x += polygons->vertices[i].x;
					dst_center.y += polygons->vertices[i].y;
					dst_center.z += polygons->vertices[i].z;*/

					if(polygons->vertices[i].position.x > dst_maxs.x) dst_maxs.x = polygons->vertices[i].position.x;
					if(polygons->vertices[i].position.y > dst_maxs.y) dst_maxs.y = polygons->vertices[i].position.y;
					if(polygons->vertices[i].position.z > dst_maxs.z) dst_maxs.z = polygons->vertices[i].position.z;


					if(polygons->vertices[i].position.x < dst_mins.x) dst_mins.x = polygons->vertices[i].position.x;
					if(polygons->vertices[i].position.y < dst_mins.y) dst_mins.y = polygons->vertices[i].position.y;
					if(polygons->vertices[i].position.z < dst_mins.z) dst_mins.z = polygons->vertices[i].position.z;

				}

				//total_vert_count += vert_count;

				polygons = polygons->next;
			}

			/*dst_center.x /= total_vert_count;
			dst_center.y /= total_vert_count;
			dst_center.z /= total_vert_count;*/


			dst_center.x = (dst_maxs.x + dst_mins.x) / 2.0;
			dst_center.y = (dst_maxs.y + dst_mins.y) / 2.0;
			dst_center.z = (dst_maxs.z + dst_mins.z) / 2.0;

			if(bsp_LineOfSight(bsp, src_center, &dst_center))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}

 			#define SMALL_EPSILON2 0.001

 			/*for(i = 0; i < sizeof(checks);)
			 {
			 	dst_corner.x = dst_center.x + dst_maxs.x * checks[i];
			 	i++;
			 	dst_corner.y = dst_center.y + dst_maxs.y * checks[i];
			 	i++;
			 	dst_corner.z = dst_center.z + dst_maxs.z * checks[i];
			 	i++;

			 	if(bsp_LineOfSight(bsp, src_center, &dst_corner))
				{
					src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
					return;
	 			}
			 }*/



 			/*dst_corner.x = dst_center.x - dst_maxs.x + SMALL_EPSILON2;
 			dst_corner.y = dst_center.y + dst_maxs.y - SMALL_EPSILON2;
 			dst_corner.z = dst_center.z + dst_maxs.z - SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}

 			dst_corner.x = dst_center.x - dst_maxs.x + SMALL_EPSILON2;
 			dst_corner.y = dst_center.y - dst_maxs.y + SMALL_EPSILON2;
 			dst_corner.z = dst_center.z + dst_maxs.z - SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}

 			dst_corner.x = dst_center.x + dst_maxs.x - SMALL_EPSILON2;
 			dst_corner.y = dst_center.y - dst_maxs.y + SMALL_EPSILON2;
 			dst_corner.z = dst_center.z + dst_maxs.z - SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}

 			dst_corner.x = dst_center.x + dst_maxs.x - SMALL_EPSILON2;
 			dst_corner.y = dst_center.y + dst_maxs.y - SMALL_EPSILON2;
 			dst_corner.z = dst_center.z + dst_maxs.z - SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}




 			dst_corner.x = dst_center.x - dst_maxs.x + SMALL_EPSILON2;
 			dst_corner.y = dst_center.y + dst_maxs.y - SMALL_EPSILON2;
 			dst_corner.z = dst_center.z - dst_maxs.z + SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}

 			dst_corner.x = dst_center.x - dst_maxs.x + SMALL_EPSILON2;
 			dst_corner.y = dst_center.y - dst_maxs.y + SMALL_EPSILON2;
 			dst_corner.z = dst_center.z - dst_maxs.z + SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}

 			dst_corner.x = dst_center.x + dst_maxs.x - SMALL_EPSILON2;
 			dst_corner.y = dst_center.y - dst_maxs.y + SMALL_EPSILON2;
 			dst_corner.z = dst_center.z - dst_maxs.z + SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}

 			dst_corner.x = dst_center.x + dst_maxs.x - SMALL_EPSILON2;
 			dst_corner.y = dst_center.y + dst_maxs.y - SMALL_EPSILON2;
 			dst_corner.z = dst_center.z - dst_maxs.z + SMALL_EPSILON2;
 			if(bsp_LineOfSight(bsp, src_center, &dst_corner))
			{
				src_leaf->pvs[leaf->leaf_index >> 3] |= 1 << (leaf->leaf_index % 8);
				return;
 			}*/

		}
	}
	else
	{
		bsp_ApproximatePvsForLeaf(src_leaf, src_center, node->front, bsp);
		bsp_ApproximatePvsForLeaf(src_leaf, src_center, node->back, bsp);
	}
}

void bsp_ApproximatePvsForLeaves(bsp_node_t *node, bsp_node_t *bsp, int leaf_count)
{
	/*bsp_leaf_t *leaf;
	vec3_t center;
	bsp_polygon_t *polygons;
	int i;
	int vert_count;
	int total_vert_count = 0;

	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;

		if(!(leaf->bm_flags & BSP_SOLID))
		{
			center.x = 0.0;
			center.y = 0.0;
			center.z = 0.0;

			polygons = leaf->polygons;

			while(polygons)
			{

				vert_count = polygons->vert_count;

				for(i = 0; i < vert_count; i++)
				{
					center.x += polygons->vertices[i].position.x;
					center.y += polygons->vertices[i].position.y;
					center.z += polygons->vertices[i].position.z;
				}

				total_vert_count += vert_count;

				polygons = polygons->next;
			}

			center.x /= total_vert_count;
			center.y /= total_vert_count;
			center.z /= total_vert_count;


			if(!leaf->pvs)
			{
				leaf->pvs = malloc(leaf_count >> 3);
				for(i = 0; i < leaf_count >> 3; i++)
				{
					leaf->pvs[i] = 0;
				}
			}

			bsp_ApproximatePvsForLeaf(leaf, &center, bsp, bsp);
		}
	}
	else
	{
		bsp_ApproximatePvsForLeaves(node->front, bsp, leaf_count);
		bsp_ApproximatePvsForLeaves(node->back, bsp, leaf_count);
	}*/
}

void bsp_CalculateApproximatePvs(bsp_node_t *bsp)
{
	int leaf_count = 0;
	int node_count = 0;
	bsp_CountNodesAndLeaves(bsp, &leaf_count, &node_count);

	bsp_ApproximatePvsForLeaves(bsp, bsp, leaf_count);
}

int bsp_LineOfSight(bsp_node_t *root, vec3_t *start, vec3_t *end)
{
	bsp_leaf_t *leaf;
	float d0;
	float d1;

	float frac;
	vec3_t v;
	if(root->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)root;

		if(!(leaf->bm_flags & BSP_SOLID))
			return 1;


		return 0;
	}
	else
	{
		v.x = start->x - root->point.x;
		v.y = start->y - root->point.y;
		v.z = start->z - root->point.z;

		d0 = dot3(v, root->normal);

		v.x = end->x - root->point.x;
		v.y = end->y - root->point.y;
		v.z = end->z - root->point.z;

		d1 = dot3(v, root->normal);


		if(d0 >= 0.0 && d1 >= 0.0)
		{
			return bsp_LineOfSight(root->front, start, end);
		}
		else if(d0 < 0.0 && d1 < 0.0)
		{
			return bsp_LineOfSight(root->back, start, end);
		}


		frac = (d0) / (d0 - d1);

		if(frac < 0.0) frac = 0.0;
		else if(frac > 1.0) frac = 1.0;

		v.x = start->x + (end->x - start->x) * frac;
		v.y = start->y + (end->y - start->y) * frac;
		v.z = start->z + (end->z - start->z) * frac;


		return bsp_LineOfSight(root->front, start, &v) & bsp_LineOfSight(root->back, &v, end);

	}
}

void bsp_DrawPortals()
{
	#if 0
	bsp_portal_t *p;
	bsp_polygon_t *poly;
	camera_t *active_camera = camera_GetActiveCamera();
	int i;
	int c;

	vec3_t center;
	vec3_t v;

	if(!world_portals)
		return;


	glUseProgram(0);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);




	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	p = world_portals;
	glLineWidth(1.0);
	glBegin(GL_LINES);
	//glColor3f(1.0, 1.0, 1.0);

	while(p)
	{
		poly = p->portal_polygon;
		c = poly->vert_count;

		center.x = 0.0;
		center.y = 0.0;
		center.z = 0.0;


		#if 0
		if((p->leaf0->leaf_index == 0 && p->leaf1->leaf_index == 1) ||
	   	   (p->leaf0->leaf_index == 1 && p->leaf1->leaf_index == 0) ||
			(p->leaf0->leaf_index == 1 || p->leaf1->leaf_index == 1) /*||
			(p->leaf0->leaf_index == 2 || p->leaf1->leaf_index == 2)*/)
		#endif

		/*if((p->leaf0->leaf_index == 0 && p->leaf1->leaf_index == 2) ||
		   (p->leaf1->leaf_index == 0 && p->leaf0->leaf_index == 2))*/


		//if(p->leaf0->leaf_index == 1 || p->leaf1->leaf_index == 1)
		{

			if(p->leaf0 && p->leaf1)
			{
				glColor3f(1.0, 1.0, 1.0);
			}
			else
			{
				glColor3f(1.0, 0.0, 0.0);
			}


			for(i = 0; i < c; i++)
			{
				center.x += poly->vertices[i].position.x;
				center.y += poly->vertices[i].position.y;
				center.z += poly->vertices[i].position.z;
			}

			center.x /= c;
			center.y /= c;
			center.z /= c;


			#define SCALE 0.95

			for(i = 0; i < c;)
			{

				v.x = (poly->vertices[i%c].position.x - center.x) * SCALE;
				v.y = (poly->vertices[i%c].position.y - center.y) * SCALE;
				v.z = (poly->vertices[i%c].position.z - center.z) * SCALE;

				glVertex3f(center.x + v.x, center.y + v.y, center.z + v.z);
				i++;

				v.x = (poly->vertices[i%c].position.x - center.x) * SCALE;
				v.y = (poly->vertices[i%c].position.y - center.y) * SCALE;
				v.z = (poly->vertices[i%c].position.z - center.z) * SCALE;

				glVertex3f(center.x + v.x, center.y + v.y, center.z + v.z);
			}
		}

		glColor3f(fabs(poly->normal.x), fabs(poly->normal.y), fabs(poly->normal.z));
		glVertex3f(center.x, center.y, center.z);
		glVertex3f(center.x + poly->normal.x, center.y + poly->normal.y, center.z + poly->normal.z);

		p = p->next;
	}

	glEnd();
	glLineWidth(1.0);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#endif
}



//int p_p_count = 0;

#if 0

void bsp_NextPortal()
{
	bsp_portal_t *r;
	bsp_portal_t *t;
	bsp_portal_t *next;
	if(world_portals)
	{
		//if(!g_next_portal)
	//	{
		g_next_portal = world_portals;
	//	}

		/*p_p_count++;

		if(p_p_count == 9)
		{
			printf("breakpoint!\n");

			printf("breakpoint!\n");
		}*/


		next = g_next_portal->next;

		g_next_portal->next = NULL;

		g_next_portal = bsp_ClipPortalToBsp(world_nodes, g_next_portal);

		r = next;
		if(r)
		{
			while(r->next)
			{
				r = r->next;
			}
			r->next = g_next_portal;
			world_portals = next;
		}


		//g_next_portal = next;

		if(!g_next_portal)
		{
			printf("all portals done\n");
		}


	}
}


#endif

















