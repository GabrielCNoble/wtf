
#ifndef REMOVE_ALL_DEPENDENCIES

	#include "bsp.h"
	#include "bsp_cmp.h"
	#include "pvs.h"
	#include "world.h"
	#include "camera.h"
	#include "player.h"
	#include "engine.h"
	#include "physics.h"
	#include "c_memory.h"
	#include "SDL2\SDL.h"
	#include "GL\glew.h"




	/* from brush.c */
	//extern int brush_list_size;
	//extern int brush_count;
	//extern brush_t *brushes;
	//extern int expanded_brush_count;
	//extern brush_t *expanded_brushes;

	//bsp_polygon_t *expanded_polygons = NULL;

	/* from light.c */
	extern int visible_light_count;

	/* from brush.c */
	//extern vec3_t cube_bmodel_collision_verts[];
	//extern vec3_t cube_bmodel_collision_normals[];

	/* from world.c */
	bsp_node_t *world_bsp = NULL;
	bsp_node_t *collision_bsp = NULL;
	extern int w_world_vertices_count;
	extern vertex_t *w_world_vertices;
	extern int w_world_triangle_count;
	extern unsigned int w_max_visible_indexes;
	extern unsigned int w_max_visible_batches;
	//extern int world_triangle_group_count;
	//extern triangle_group_t *world_triangle_groups;
	extern int w_world_batch_count;
	extern struct batch_t *w_world_batches;
	extern int w_world_nodes_count;
	extern struct bsp_pnode_t *w_world_nodes;
	//extern int collision_nodes_count;
	//extern bsp_pnode_t *collision_nodes;
	//extern bsp_polygon_t *node_polygons;			/* necessary to quickly build portals... */
	extern int w_world_leaves_count;
	extern struct bsp_dleaf_t *w_world_leaves;
	extern int visited_leaves_count;
	extern struct bsp_dleaf_t *visited_leaves;

	bsp_polygon_t *node_polygons = NULL;			/* necessary to quickly build portals... */

	static SDL_Thread *bsp_build_thread;

	SDL_mutex *polygon_copy_mutex = NULL;
	SDL_sem *step_semaphore = NULL;
	SDL_mutex *stop_flags_mutex = NULL;

	SDL_mutex *brush_polygons_lock = NULL;

	bsp_polygon_t *beveled_polygons = NULL;
	bsp_edge_t *bevel_edges = NULL;

	bsp_polygon_t *world_polygons_debug = NULL;

	int b_compiling = 0;
	//int b_stop = 0;

	extern int b_calculating_pvs;


	//#define BSP_POOL_SIZE 50000

	//int node_pool_cursor = 0;
	//bsp_node_t *node_pool = NULL;

	//int leaf_pool_cursor = 0;
	//bsp_leaf_t *leaf_pool = NULL;


	//#define DRAW_EXPANDED_BRUSHES
	//#define DRAW_BEVEL_EDGES
	//#define DRAW_WORLD_POLYGONS




	#define USE_MEMORY_MALLOC

#else


#endif


#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <fenv.h>




#ifdef USE_MEMORY_MALLOC
#define malloc ((void *)0)
#define free ((void *)0)
#define calloc ((void *)0)
#define strdup ((void *)0)
#endif



/*
==============
bsp_ClassifyPoint
==============
*/
int bsp_ClassifyPoint(vec3_t point, vec3_t splitter_point, vec3_t splitter_normal)
{
	vec3_t v;
	float d;
	float r;
	v.x = point.x - splitter_point.x;
	v.y = point.y - splitter_point.y;
	v.z = point.z - splitter_point.z;
	d = splitter_normal.x * v.x + splitter_normal.y * v.y + splitter_normal.z * v.z;

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


/*
==============
bsp_ClassifyPolygon
==============
*/
int bsp_ClassifyPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal)
{
	int front = 0;
	int back = 0;
	int a = 0;
	//int contained = 0;

	int i;
	int c = polygon->vert_count;

	//assert(polygon);
	//assert(splitter);

	for(i = 0; i < c; i++)
	{
		switch(bsp_ClassifyPoint(polygon->vertices[i].position, point, normal))
		{
			case POINT_FRONT:
				front++;
			break;

			case POINT_BACK:
				back++;
			break;
		}
	}

	if(front && back)
	{
		return POLYGON_STRADDLING;
	}
	else
	{
		if(front)
		{
			return POLYGON_FRONT;
		}
		else if(back)
		{
			return POLYGON_BACK;
		}
		else
		{
			//return POLYGON_CONTAINED;
			if(dot3(polygon->normal, normal) < 0.0)
			{
				return POLYGON_CONTAINED_BACK;
			}
			else
			{
				return POLYGON_CONTAINED_FRONT;
			}
		}
	}

}


/*
==============
bsp_ClipEdge
==============
*/
int bsp_ClipEdge(vec3_t a, vec3_t b, vec3_t point, vec3_t normal, vec3_t *clip_vertex, float *time)
{

	vec3_t v;
	float n;
	float d;
	float t;

	int a_side;
	int b_side;

	vec3_t edge;

	v.x = point.x - a.x;
	v.y = point.y - a.y;
	v.z = point.z - a.z;

	edge.x = b.x - a.x;
	edge.y = b.y - a.y;
	edge.z = b.z - a.z;

	n = (v.x * normal.x + v.y * normal.y + v.z * normal.z);
	d = (edge.x * normal.x + edge.y * normal.y + edge.z * normal.z);


	if(fabs(d) > FUZZY_ZERO)
	{
		t = n / d;

		//printf(">>>%f %f\n", n, d);

		if(t <= 1.0 && t >= 0.0)
		{
			*time = t;

			clip_vertex->x = a.x + edge.x * t;
			clip_vertex->y = a.y + edge.y * t;
			clip_vertex->z = a.z + edge.z * t;
			return 1;
		}
	}

	return 0;
}



/*
==============
bsp_SplitPolygon
==============
*/
int bsp_SplitPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal, bsp_polygon_t **front, bsp_polygon_t **back)
{
	int clip_vertex_count = 0;
	vec3_t clip_vertexes[2];
	vertex_t clip_vertex;
	vec3_t front_back_vertex;
	vec3_t back_front_vertex;

	int front_vertex_count = 0;
	vertex_t front_vertexes[128];

	int back_vertex_count = 0;
	vertex_t back_vertexes[128];


	bsp_triangle_t *front_triangles = NULL;
	bsp_triangle_t *back_triangles = NULL;
	bsp_triangle_t *partial_front_triangles = NULL;
	bsp_triangle_t *partial_back_triangles = NULL;

	bsp_triangle_t *t = NULL;
	bsp_triangle_t *ft = NULL;
	bsp_triangle_t *bt = NULL;

	float time;

	int i = 0;
	int c;
	int r;

	int pa;
	int pb;

	//int front_count = 0;
	//int front_start = -1;
	//int front_end = -1;

	//int back_count = 0;
	//int back_start = -1;
	//int back_end = -1;

	int cur_side = 0;

	bsp_polygon_t *f;
	bsp_polygon_t *b;


	pa = bsp_ClassifyPoint(polygon->vertices[0].position, point, normal);
	pb = bsp_ClassifyPoint(polygon->vertices[1].position, point, normal);
	c = polygon->vert_count;

	//assert(pa != pb);
	assert(c > 2);

	//assert(c < 128);


	if(pa == POINT_CONTAINED)
	{
		cur_side = pb;
	}
	else
	{
		cur_side = pa;
	}

	//cur_side = pa;

	for(i = 0; i < c; i++)
	{
		r = (i + 1) % c;

		pa = bsp_ClassifyPoint(polygon->vertices[i].position, point, normal);
		pb = bsp_ClassifyPoint(polygon->vertices[r].position, point, normal);

		//assert(pa != POINT_CONTAINED && pb != POINT_CONTAINED);

		/* this edge straddles this splitter, so clip it... */
		if((pa | pb) == (POINT_FRONT | POINT_BACK))
		{
			bsp_ClipEdge(polygon->vertices[i].position, polygon->vertices[r].position, point, normal, &clip_vertex.position, &time);

			clip_vertex.normal.x = (1.0 - time) * polygon->vertices[i].normal.x + time * polygon->vertices[r].normal.x;
			clip_vertex.normal.y = (1.0 - time) * polygon->vertices[i].normal.y + time * polygon->vertices[r].normal.y;
			clip_vertex.normal.z = (1.0 - time) * polygon->vertices[i].normal.z + time * polygon->vertices[r].normal.z;

			clip_vertex.tangent.x = (1.0 - time) * polygon->vertices[i].tangent.x + time * polygon->vertices[r].tangent.x;
			clip_vertex.tangent.y = (1.0 - time) * polygon->vertices[i].tangent.y + time * polygon->vertices[r].tangent.y;
			clip_vertex.tangent.z = (1.0 - time) * polygon->vertices[i].tangent.z + time * polygon->vertices[r].tangent.z;

			clip_vertex.tex_coord.x = (1.0 - time) * polygon->vertices[i].tex_coord.x + time * polygon->vertices[r].tex_coord.x;
			clip_vertex.tex_coord.y = (1.0 - time) * polygon->vertices[i].tex_coord.y + time * polygon->vertices[r].tex_coord.y;


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

			clip_vertex_count++;

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
	//assert(back_vertex_count > 2);
	//assert(front_vertex_count > 2);
	//assert(clip_vertex_count > 0);

	#ifndef USE_MEMORY_MALLOC
	f = malloc(sizeof(bsp_polygon_t ));
	#else
	f = memory_Calloc(1, sizeof(bsp_polygon_t ));
	#endif
	f->normal = polygon->normal;
	f->vert_count = front_vertex_count;

	#ifndef USE_MEMORY_MALLOC
	f->vertices = malloc(sizeof(vertex_t) * (f->vert_count));
	#else
	f->vertices = memory_Calloc(f->vert_count, sizeof(vertex_t));
	#endif

	f->b_used = polygon->b_used;
	f->next = NULL;
	f->material_index = polygon->material_index;
	f->triangle_group = polygon->triangle_group;
	f->brush_index = polygon->brush_index;

	for(i = 0; i < f->vert_count; i++)
	{
		f->vertices[i] = front_vertexes[i];
	}

	#ifndef USE_MEMORY_MALLOC
	b = malloc(sizeof(bsp_polygon_t ));
	#else
	b = memory_Calloc(1, sizeof(bsp_polygon_t ));
	#endif
	b->normal = polygon->normal;
	b->vert_count = back_vertex_count;


	#ifndef USE_MEMORY_MALLOC
	b->vertices = malloc(sizeof(vertex_t) * (b->vert_count));
	#else
	b->vertices = memory_Calloc(b->vert_count, sizeof(vertex_t));
	#endif

	b->next = NULL;
	b->b_used = polygon->b_used;
	b->material_index = polygon->material_index;
	b->triangle_group = polygon->triangle_group;
	b->brush_index = polygon->brush_index;

	for(i = 0; i < b->vert_count; i++)
	{
		b->vertices[i] = back_vertexes[i];
	}


	*front = f;
	*back = b;

}



bsp_polygon_t *bsp_DeepCopyPolygon(bsp_polygon_t *src)
{
	int i;
	bsp_polygon_t *p = NULL;

	if(src)
	{
		#ifndef USE_MEMORY_MALLOC
		p = malloc(sizeof(bsp_polygon_t));
		#else
		p = memory_Calloc(1, sizeof(bsp_polygon_t));
		#endif

		p->normal = src->normal;
		p->tiling = src->tiling;
		p->brush_index = src->brush_index;
		p->b_used = src->b_used;
		//p->next = src->next;
		p->next = NULL;
		p->vert_count = src->vert_count;

		#ifndef USE_MEMORY_MALLOC
		p->vertices = malloc(sizeof(vertex_t) * p->vert_count);
		#else
		p->vertices = memory_Calloc(p->vert_count, sizeof(vertex_t));
		#endif

		p->material_index = src->material_index;
		p->triangle_group = src->triangle_group;


		for(i = 0; i < p->vert_count; i++)
		{
			p->vertices[i] = src->vertices[i];
		}
	}



	return p;
}


bsp_polygon_t *bsp_DeepCopyPolygons(bsp_polygon_t *src)
{
	bsp_polygon_t *copy = NULL;
	bsp_polygon_t *r = NULL;

	int i;

	while(src)
	{
		#ifndef USE_MEMORY_MALLOC
		r = malloc(sizeof(bsp_polygon_t ));
		#else
		r = memory_Calloc(1, sizeof(bsp_polygon_t ));
		#endif

		r->brush_index = src->brush_index;
		r->b_used = src->b_used;
		r->normal = src->normal;
		r->tiling = src->tiling;
		r->vert_count = src->vert_count;

		#ifndef USE_MEMORY_MALLOC
		r->vertices = malloc(sizeof(vertex_t) * r->vert_count);
		#else
		r->vertices = memory_Calloc(r->vert_count, sizeof(vertex_t));
		#endif

		r->material_index = src->material_index;
		r->triangle_group = src->triangle_group;

		for(i = 0; i < r->vert_count; i++)
		{
			r->vertices[i] = src->vertices[i];
		}

		r->next = copy;
		copy = r;

		src = src->next;
	}

	return copy;
}


void bsp_DeletePolygons(bsp_polygon_t *polygons)
{
	bsp_polygon_t *next;

	while(polygons)
	{
		next = polygons->next;
		#ifndef USE_MEMORY_MALLOC
		free(polygons->vertices);
		free(polygons);
		#else
		memory_Free(polygons->vertices);
		memory_Free(polygons);
		#endif
		polygons = next;
	}
}


int bsp_TrimPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal)
{

	#if 0
	vertex_t clip_vertex;
	int front_vertex_count = 0;
	vertex_t front_vertexes[128];


	float time;

	int i = 0;
	int c;
	int r;

	int pa;
	int pb;

	int cur_side = 0;

	c = polygon->vert_count;
	assert(c > 2);



	/*if(bsp_ClassifyPolygon(polygon, point, normal) != POLYGON_STRADDLING)
	{
		return 0;
	}*/

	switch(bsp_ClassifyPolygon(polygon, point, normal))
	{
		case POLYGON_FRONT:
		case POLYGON_CONTAINED_FRONT:
			return 1;

		case POLYGON_BACK:
		case POLYGON_CONTAINED_BACK:
			return -1;
	}

/*	if(i == POLYGON_FRONT)
	{
		return 1
	}
	else if(i == POLYGON_BACK)
	{
		return -1;
	}*/

	pa = bsp_ClassifyPoint(polygon->vertices[0].position, point, normal);
	pb = bsp_ClassifyPoint(polygon->vertices[1].position, point, normal);




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

		pa = bsp_ClassifyPoint(polygon->vertices[i].position, point, normal);
		pb = bsp_ClassifyPoint(polygon->vertices[r].position, point, normal);

		//assert(pa != POINT_CONTAINED || pb != POINT_CONTAINED);

		/* this edge straddles this splitter, so clip it... */
		if((pa | pb) == (POINT_FRONT | POINT_BACK))
		{
			bsp_ClipEdge(polygon->vertices[i].position, polygon->vertices[r].position, point, normal, &clip_vertex.position, &time);


			clip_vertex.normal.x = (1.0 - time) * polygon->vertices[i].normal.x + time * polygon->vertices[r].normal.x;
			clip_vertex.normal.y = (1.0 - time) * polygon->vertices[i].normal.y + time * polygon->vertices[r].normal.y;
			clip_vertex.normal.z = (1.0 - time) * polygon->vertices[i].normal.z + time * polygon->vertices[r].normal.z;

			clip_vertex.tangent.x = (1.0 - time) * polygon->vertices[i].tangent.x + time * polygon->vertices[r].tangent.x;
			clip_vertex.tangent.y = (1.0 - time) * polygon->vertices[i].tangent.y + time * polygon->vertices[r].tangent.y;
			clip_vertex.tangent.z = (1.0 - time) * polygon->vertices[i].tangent.z + time * polygon->vertices[r].tangent.z;

			clip_vertex.tex_coord.x = (1.0 - time) * polygon->vertices[i].tex_coord.x + time * polygon->vertices[r].tex_coord.x;
			clip_vertex.tex_coord.y = (1.0 - time) * polygon->vertices[i].tex_coord.y + time * polygon->vertices[r].tex_coord.y;


			if(cur_side == POINT_FRONT)
			{
				cur_side = POINT_BACK;
				front_vertexes[front_vertex_count++] = polygon->vertices[i];
				front_vertexes[front_vertex_count++] = clip_vertex;
			}
			else
			{
				cur_side = POINT_FRONT;
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
					cur_side = pb;
				}
			}
			else
			{
				if(pa == POINT_CONTAINED)
				{
					front_vertexes[front_vertex_count++] = polygon->vertices[i];
					cur_side = pb;
				}
			}

		}
	}

	if(front_vertex_count > polygon->vert_count)
	{
		#ifndef USE_MEMORY_MALLOC
		free(polygon->vertices);
		polygon->vertices = malloc(sizeof(vertex_t) * front_vertex_count);
		#else
		memory_Free(polygon->vertices);
		polygon->vertices = memory_Malloc(sizeof(vertex_t) * front_vertex_count);
		#endif
		polygon->vert_count = front_vertex_count;
	}

	for(i = 0; i < front_vertex_count; i++)
	{
		polygon->vertices[i] = front_vertexes[i];
	}

	return 0;

	#endif

}

int bsp_ClipCoplanarPolygonToPolygonEdges(bsp_polygon_t *polygon, bsp_polygon_t *splitter, bsp_polygon_t **clips)
{
	#if 0
	vec3_t edge_plane_normal;
	vec3_t edge_plane_point;


	vec3_t r;
	vec3_t s;


	bsp_polygon_t *clipped = NULL;
	bsp_polygon_t *front = NULL;
	bsp_polygon_t *back = NULL;
	bsp_polygon_t *q = NULL;
	bsp_polygon_t *t = NULL;
	bsp_polygon_t *h = NULL;

	int i;
	int c;

	c = splitter->vert_count;
	q = polygon;
	for(i = 0; i < splitter->vert_count; i++)
	{
		edge_plane_point = splitter->vertices[i].position;
		r = splitter->vertices[(i + 1) % c].position;

		s.x = r.x - edge_plane_point.x;
		s.y = r.y - edge_plane_point.y;
		s.z = r.z - edge_plane_point.z;

		edge_plane_normal = normalize3(cross(s, splitter->normal));

		while(q)
		{
			if(bsp_ClassifyPolygon(q, edge_plane_point, edge_plane_normal) == POLYGON_STRADDLING)
			{
				bsp_SplitPolygon(q, edge_plane_point, edge_plane_normal, &front, &back);
				h = NULL;

				if(front)
				{
					t = front;
					while(t->next) t = t->next;
					t->next = back;
					h = front;
				}

				if(back)
				{
					t = back;
					while(t->next) t = t->next;
					if(!h) h = back;
				}

				if(t)
				{
					t->next = clipped;
					clipped = h;
					break;
				}
			}

			q = q->next;
		}

		if(!clipped)
			q = polygon;
		else
			q = clipped;
	}

	if(!clipped)
	{
		return 0;
		//*clips = bsp_DeepCopyPolygon(polygon);
	}

	return 1;

	#endif
}



/*
==============
bsp_FindSplitter
==============
*/
bsp_polygon_t *bsp_FindSplitter(bsp_polygon_t **polygons, int ignore_used, int ignore_coplanar)
{
	bsp_polygon_t *cur_splitter = *polygons;
	bsp_polygon_t *last_splitter = NULL;
	bsp_polygon_t *prev_splitter = NULL;
	bsp_polygon_t *r;

	bsp_polygon_t *min_splitter = NULL;
	bsp_polygon_t *prev_min_splitter = NULL;
	unsigned int min_split_count = 0xffffffff;
	unsigned int split_count = 0;
	int i;

	float f;

	while(cur_splitter)
	{
		split_count = 0;

		/* skip polygons that have already been used as a splitter... */
		if(ignore_used)
		{
			if(cur_splitter->b_used)
			{
				cur_splitter = cur_splitter->next;
				prev_splitter = NULL;
				continue;
			}
		}

		r = *polygons;

		while(r)
		{
			if(r == cur_splitter)
			{
				r = r->next;
				continue;
			}

			if(bsp_ClassifyPolygon(r, cur_splitter->vertices[0].position, cur_splitter->normal) == POLYGON_STRADDLING) split_count++;
			r = r->next;
		}

		if(split_count < min_split_count)
		{
			min_split_count = split_count;
			min_splitter = cur_splitter;
			prev_min_splitter = prev_splitter;
		}

		prev_splitter = cur_splitter;
		cur_splitter = cur_splitter->next;
	}

	if(ignore_used)
	{
		if(min_splitter)
		{
			min_splitter->b_used = 1;

			/* mark coplanar polygons as used to
			avoid adding redundant nodes to the
			bsp... */

			/* NOTE: this might cause problems
			with polygons that are not *quite*
			on the plane, but slightly twisted
			against. FUZZY_ZERO macro defines
			how contained a polygon is before
			it's considered to be straddling... */

			if(ignore_coplanar)
			{
				cur_splitter = *polygons;
				while(cur_splitter)
				{
					if(cur_splitter == min_splitter)
					{
						cur_splitter = cur_splitter->next;
						continue;
					}

					i = bsp_ClassifyPolygon(cur_splitter, min_splitter->vertices[0].position, min_splitter->normal);

					if(i == POLYGON_CONTAINED_FRONT || i == POLYGON_CONTAINED_BACK)
					{
						cur_splitter->b_used = 1;
					}


					cur_splitter = cur_splitter->next;
				}
			}


		}

	}

	return min_splitter;
}


/*
==============
bsp_OpPolygonToBsp
==============
*/

bsp_polygon_t *bsp_OpPolygonToBsp(int op, bsp_node_t *bsp, bsp_polygon_t *polygon, int copy)
{
	bsp_polygon_t *frags = NULL;
	bsp_polygon_t *f = NULL;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *n;
	bsp_polygon_t *s;
	bsp_polygon_t *front = NULL;
	bsp_polygon_t *back = NULL;

	vec3_t point;
	vec3_t normal;

	vertex_t v;

	int i;
	int c;

	/* A (subtraction) B == A (intersection) (~B) */
	if(op == CSG_OP_SUBTRACTION)
	{
		c = polygon->vert_count;

		/* ~B... */
		polygon->normal.x = -polygon->normal.x;
		polygon->normal.y = -polygon->normal.y;
		polygon->normal.z = -polygon->normal.z;

		for(i = 0; i < c >> 1; i++)
		{
			v = polygon->vertices[i];

			v.normal.x = -v.normal.x;
			v.normal.y = -v.normal.y;
			v.normal.z = -v.normal.z;

			v.tangent.x = -v.tangent.x;
			v.tangent.y = -v.tangent.y;
			v.tangent.z = -v.tangent.z;

			polygon->vertices[i] = polygon->vertices[c - i - 1];
			polygon->vertices[c - i - 1] = v;

			v = polygon->vertices[i];

			v.normal.x = -v.normal.x;
			v.normal.y = -v.normal.y;
			v.normal.z = -v.normal.z;

			v.tangent.x = -v.tangent.x;
			v.tangent.y = -v.tangent.y;
			v.tangent.z = -v.tangent.z;

			polygon->vertices[i] = v;
		}

		op = CSG_OP_INTERSECTION;
	}



	if(bsp->type == BSP_LEAF)
	{
		switch(op)
		{
			case CSG_OP_UNION:
				if(bsp->bm_flags & BSP_SOLID)
				{
					/* (!copy) means that it's not necessary to copy this
					polygon, meaning it doesn't belong to any bsp tree,
					so it's safe to get rid of it here... */
					if(!copy)
					{
						#ifndef USE_MEMORY_MALLOC
						free(polygon->vertices);
						free(polygon);
						#else
						memory_Free(polygon->vertices);
						memory_Free(polygon);
						#endif
					}
					return NULL;
				}


				/* return a new copy of this polygon if this polygon
				didn't come from a split... */
				if(copy)
				{
					front = bsp_DeepCopyPolygon(polygon);
					return front;
				}
				else
				{
					polygon->next = NULL;
					return polygon;
				}
			break;

			case CSG_OP_INTERSECTION:
				if(bsp->bm_flags & BSP_SOLID)
				{
					/* return a new copy of this polygon if this polygon
					didn't come from a split... */
					if(copy)
					{
						front = bsp_DeepCopyPolygon(polygon);
						return front;
					}
					else
					{
						polygon->next = NULL;
						return polygon;
					}

				}

				/* (!copy) means that it's not necessary to copy this
				polygon, meaning it doesn't belong to any bsp tree,
				so it's safe to get rid of it here... */
				if(!copy)
				{
					#ifndef USE_MEMORY_MALLOC
					free(polygon->vertices);
					free(polygon);
					#else
					memory_Free(polygon->vertices);
					memory_Free(polygon);
					#endif
				}
				return NULL;
			break;
		}


	}
	else
	{

		if(bsp->splitter)
		{
			point = bsp->splitter->vertices[0].position;
			normal = bsp->splitter->normal;
		}
		else
		{
			point = bsp->point;
			normal = bsp->normal;
		}


		i = bsp_ClassifyPolygon(polygon, point, normal);

		switch(i)
		{
			case POLYGON_CONTAINED_FRONT:
				/* HACK: this is here to avoid the case where the polygon of a subtractive brush
				ends up being thrown away when it should instead be the inside polygon of the other
				brush. This happens when the subtractive brush's face is coplanar with the face
				of the other brush. Since the subtractive brush's face gets flipped, it's considered
				to be contained and facing the same direction of the other brush, which makes it end
				up inside empty space. Given that the intersection operation throws away faces in
				empty space and keeps faces in solid space, every-time we do an intersection operation
				and find a face being coplanar and facing the same direction as the splitting plane,
				we'll send it to the opposite subtree instead. This guarantees that there won't be no
				missing faces when two brushes are touching ...  */
				if(op == CSG_OP_INTERSECTION) return bsp_OpPolygonToBsp(op, bsp->back, polygon, copy);
			case POLYGON_FRONT:
				return bsp_OpPolygonToBsp(op, bsp->front, polygon, copy);
			break;

			case POLYGON_CONTAINED_BACK:
				if(op == CSG_OP_INTERSECTION) return bsp_OpPolygonToBsp(op, bsp->front, polygon, copy);
			case POLYGON_BACK:
				return bsp_OpPolygonToBsp(op, bsp->back, polygon, copy);
			break;


			case POLYGON_STRADDLING:

				/* bsp_SplitPolygon allocates two new polygons... */
				bsp_SplitPolygon(polygon, point, normal, &front, &back);


				if(!copy)
				{
					#ifndef USE_MEMORY_MALLOC
					free(polygon->vertices);
					free(polygon);
					#else
					memory_Free(polygon->vertices);
					memory_Free(polygon);
					#endif
				}

				/*... so tell any recursive call that it isn't necessary
				to make a copy before returning the polygon when it falls
				within an out leaf (or freeing it when it falls in solid
				space)... */
				front = bsp_OpPolygonToBsp(op, bsp->front, front, 0);
				back = bsp_OpPolygonToBsp(op, bsp->back, back, 0);

				if(front)
				{
					frags = front;
					r = front;
					while(r->next)
					{
						r = r->next;
					}
					r->next = back;
				}
				else
				{
					frags = back;
				}

				return frags;

				//assert(front);
				/* link those splits (and any other polygons from recursive calls)... */

			break;

			default:
				assert("holy" == "shit...");
			break;
		}
	}

	return NULL;
}


/*
==============
bsp_ClipPolygonToBsp
==============
*/
bsp_polygon_t *bsp_ClipPolygonToBsp(bsp_node_t *bsp, bsp_polygon_t *polygon, int copy)
{
	return bsp_OpPolygonToBsp(CSG_OP_UNION, bsp, polygon, copy);
}



bsp_polygon_t *bsp_ClipPolygonsToBsp(bsp_node_t *bsp, bsp_polygon_t *polygons, int do_copy)
{
	bsp_polygon_t *p; // = bsp_DeepCopyPolygons(polygons_a);
	bsp_polygon_t *r;
	bsp_polygon_t *s;
	bsp_polygon_t *clipped;
	//bsp_BuildSolid(&bsp_a, polygons_a, 0, 0);

	//r = polygons_b;

	if(do_copy)
	{
		r = bsp_DeepCopyPolygons(polygons);
	}
	else
	{
		r = polygons;
	}

	clipped = NULL;
	//polygons_c = NULL;
	//polygons_b = NULL;
	while(r)
	{
		p = r->next;
		r = bsp_ClipPolygonToBsp(bsp, r, 0);

		if(r)
		{
			s = r;
			while(s->next) s = s->next;

			s->next = clipped;
			clipped = r;
		}

		r = p;
	}

	return clipped;

}



/*
==============
bsp_ClipBspToBsp
==============
*/
bsp_polygon_t *bsp_ClipBspToBsp(bsp_node_t *bsp, bsp_node_t *input)
{
	bsp_polygon_t *polygon = NULL;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *s = NULL;

	int paths = 0;

	/* If this node is a leaf, it doesn't contain a
	polygon... */
	if(input->type == BSP_LEAF) return NULL;

	r = bsp_ClipBspToBsp(bsp, input->back);
	if(r)
	{
		s = r;
		/* necessary for clipping can result in several
		polygons, so those calls can end up returning a
		linked list... */
		while(s->next)
		{
			s = s->next;
		}
		s->next = polygon;
		polygon = r;
	}

	/* this will return null in case this polygon falls
	entirelly inside solid space... */
	r = bsp_ClipPolygonToBsp(bsp, input->splitter, 1);
	if(r)
	{
		s = r;
		while(s->next)
		{
			s = s->next;
		}
		s->next = polygon;
		polygon = r;
	}


	r = bsp_ClipBspToBsp(bsp, input->front);
	if(r)
	{
		s = r;
		while(s->next)
		{
			s = s->next;
		}
		s->next = polygon;
		polygon = r;
	}

	return polygon;
}








/*
==============
bsp_BuildSolid
==============
*/
void bsp_BuildSolid(bsp_node_t **root, bsp_polygon_t *polygons, int ignore_used, int ignore_coplanar)
{

	bsp_polygon_t *front_list = NULL;
	bsp_polygon_t *back_list = NULL;

	bsp_triangle_t *f = NULL;
	bsp_triangle_t *b = NULL;
	bsp_triangle_t *t;
	bsp_triangle_t *n;

	bsp_leaf_t *leaf;
	bsp_node_t *node;
	bsp_node_t *p;

	vec3_t point;
	vec3_t normal;

	bsp_polygon_t *cur_polygon;
	bsp_polygon_t *r;
	bsp_polygon_t *front_split;
	bsp_polygon_t *back_split;

	bsp_polygon_t *splitter;

	assert(root);
	assert(polygons);

	if(!root)
	{
		return;
	}

	#ifndef USE_MEMORY_MALLOC
	*root = malloc(sizeof(bsp_node_t));
	#else
	*root = memory_Malloc(sizeof(bsp_node_t));
	#endif

	p = *root;
	p->front = NULL;
	p->back = NULL;
	p->bm_flags = 0;
	p->type = BSP_NODE;

	p->splitter = bsp_FindSplitter(&polygons, ignore_used, ignore_coplanar);
	p->normal = p->splitter->normal;
	p->point = p->splitter->vertices[0].position;

	cur_polygon = polygons;

	while(cur_polygon)
	{

		r = cur_polygon->next;

		if(cur_polygon == p->splitter)
		{
			cur_polygon = cur_polygon->next;
			continue;
		}

		switch(bsp_ClassifyPolygon(cur_polygon, p->point, p->normal))
		{
			case POLYGON_FRONT:
			case POLYGON_CONTAINED_FRONT:
			case POLYGON_CONTAINED_BACK:
				cur_polygon->next = front_list;
				front_list = cur_polygon;

			break;

			case POLYGON_BACK:
				cur_polygon->next = back_list;
				back_list = cur_polygon;
			break;

			case POLYGON_STRADDLING:

				bsp_SplitPolygon(cur_polygon, p->point, p->normal, &front_split, &back_split);

				#ifndef USE_MEMORY_MALLOC
				free(cur_polygon->vertices);
				free(cur_polygon);
				#else
				memory_Free(cur_polygon->vertices);
				memory_Free(cur_polygon);
				#endif

				front_split->next = front_list;
				front_list = front_split;

				back_split->next = back_list;
				back_list = back_split;

			break;
		}
		cur_polygon = r;
	}

	if(!front_list)
	{
		#ifndef USE_MEMORY_MALLOC
		p->front = malloc(sizeof(bsp_leaf_t));
		#else
		p->front = memory_Malloc(sizeof(bsp_leaf_t));
		#endif

		leaf = (bsp_leaf_t *)p->front;

		assert(leaf);

		leaf->bm_flags = 0;
		leaf->type = BSP_LEAF;
		leaf->polygons = NULL;
		leaf->triangles = NULL;
		//leaf->stack = NULL;
	}
	else
	{
		bsp_BuildSolid(&p->front, front_list, ignore_used, ignore_coplanar);
	}

	if(!back_list)
	{
		#ifndef USE_MEMORY_MALLOC
		p->back = malloc(sizeof(bsp_leaf_t));
		#else
		p->back = memory_Malloc(sizeof(bsp_leaf_t));
		#endif

		leaf = (bsp_leaf_t *)p->back;

		assert(leaf);

		leaf->bm_flags = BSP_SOLID;
		leaf->type = BSP_LEAF;
		leaf->polygons = NULL;
		leaf->triangles = NULL;
		//leaf->stack = NULL;
	}
	else
	{
		bsp_BuildSolid(&p->back, back_list, ignore_used, ignore_coplanar);
	}

}


/*
==============
bsp_BuildSolidLeaf
==============
*/
void bsp_BuildSolidLeaf(bsp_node_t **root, bsp_polygon_t *polygons)
{

	bsp_polygon_t *front_list = NULL;
	bsp_polygon_t *back_list = NULL;

	bsp_triangle_t *f = NULL;
	bsp_triangle_t *b = NULL;
	bsp_triangle_t *t = NULL;
	bsp_triangle_t *n;

	bsp_leaf_t *leaf;
	bsp_node_t *node;

	bsp_polygon_t *cur_polygon;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *front_split = NULL;
	bsp_polygon_t *back_split = NULL;

	vec3_t v;

	vec3_t center;

	int i;
	int c;

	float x_max;
	float y_max;
	float z_max;

	float x_min;
	float y_min;
	float z_min;

	bsp_polygon_t *splitter = NULL;

	if(!root)
	{
		return;
	}
	splitter = bsp_FindSplitter(&polygons, 1, 1);




	if(splitter)
	{

		#ifndef USE_MEMORY_MALLOC
		*root = malloc(sizeof(bsp_node_t));
		#else
		*root = memory_Malloc(sizeof(bsp_node_t));
		#endif
		node = *root;

		node->type = BSP_NODE;
		node->bm_flags = 0;
		node->front = NULL;
		node->back = NULL;

		//splitter->b_used = 1;

		node->point = splitter->vertices[0].position;
		node->normal = splitter->normal;

		v.x = splitter->vertices[1].position.x - node->point.x;
		v.y = splitter->vertices[1].position.y - node->point.y;
		v.z = splitter->vertices[1].position.z - node->point.z;

		v = normalize3(v);

		node->tangent = cross(v, node->normal);


		cur_polygon = polygons;

		while(cur_polygon)
		{

			r = cur_polygon->next;

			if(cur_polygon == splitter)
			{
				cur_polygon->next = front_list;
				front_list = cur_polygon;
				cur_polygon = r;
				continue;
			}

			switch(bsp_ClassifyPolygon(cur_polygon, node->point, node->normal))
			{
				case POLYGON_FRONT:
				case POLYGON_CONTAINED_FRONT:
					cur_polygon->next = front_list;
					front_list = cur_polygon;

				break;

				case POLYGON_BACK:
				case POLYGON_CONTAINED_BACK:
					cur_polygon->next = back_list;
					back_list = cur_polygon;

				break;

				case POLYGON_STRADDLING:

					bsp_SplitPolygon(cur_polygon, node->point, node->normal, &front_split, &back_split);

					#ifndef USE_MEMORY_MALLOC
					free(cur_polygon->vertices);
					free(cur_polygon);
					#else
					memory_Free(cur_polygon->vertices);
					memory_Free(cur_polygon);
					#endif

					front_split->next = front_list;
					front_list = front_split;

					back_split->next = back_list;
					back_list = back_split;

				break;

				default:
					assert("oh" == "shit");
				break;
			}
			cur_polygon = r;
		}



		bsp_BuildSolidLeaf(&node->front, front_list);
		bsp_BuildSolidLeaf(&node->back, back_list);

	}
	else
	{
		#ifndef USE_MEMORY_MALLOC
		*root = malloc(sizeof(bsp_leaf_t));
		#else
		*root = memory_Calloc(1, sizeof(bsp_leaf_t));
		#endif
		leaf = (bsp_leaf_t *)*root;

		leaf->type = BSP_LEAF;
		leaf->bm_flags = 0;
		leaf->polygons = polygons;
		leaf->triangles = NULL;
		leaf->triangle_count = 0;
		leaf->portal_count = 0;
		leaf->leaf_index = 0;
		leaf->pvs = NULL;
		//leaf->stack = NULL;

		if(!polygons)
		{
			leaf->bm_flags |= BSP_SOLID;
		}
		else
		{

			x_max = -999999999.9;
			y_max = -999999999.9;
			z_max = -999999999.9;

			x_min = 999999999.9;
			y_min = 999999999.9;
			z_min = 999999999.9;


			center.x = 0.0;
			center.y = 0.0;
			center.z = 0.0;

			c = 0;
			while(polygons)
			{
				for(i = 0; i < polygons->vert_count; i++)
				{

					if(polygons->vertices[i].position.x > x_max) x_max = polygons->vertices[i].position.x;
					if(polygons->vertices[i].position.y > y_max) y_max = polygons->vertices[i].position.y;
					if(polygons->vertices[i].position.z > z_max) z_max = polygons->vertices[i].position.z;

					if(polygons->vertices[i].position.x < x_min) x_min = polygons->vertices[i].position.x;
					if(polygons->vertices[i].position.y < y_min) y_min = polygons->vertices[i].position.y;
					if(polygons->vertices[i].position.z < z_min) z_min = polygons->vertices[i].position.z;
				}

				polygons = polygons->next;
			}

			center.x = (x_max + x_min) / 2.0;
			center.y = (y_max + y_min) / 2.0;
			center.z = (z_max + z_min) / 2.0;

			leaf->center = center;
		}

		//root->back = NULL;
		//root->front = NULL;
	}

}






/*
==============
bsp_SolidBsp
==============
*/
bsp_node_t *bsp_SolidBsp(bsp_polygon_t *polygons)
{
	bsp_node_t *root = NULL;
	bsp_BuildSolid(&root, polygons, 0, 0);

	return root;
}



void bsp_DeleteSolidBsp(bsp_node_t *bsp, int free_splitters)
{
	bsp_leaf_t *leaf;

	if(!bsp)
		return;

	if(bsp->type == BSP_NODE)
	{
		if(free_splitters)
		{
			#ifndef USE_MEMORY_MALLOC
			free(bsp->splitter->vertices);
			free(bsp->splitter);
			#else
			memory_Free(bsp->splitter->vertices);
			memory_Free(bsp->splitter);
			#endif
		}

		bsp_DeleteSolidBsp(bsp->front, free_splitters);
		bsp_DeleteSolidBsp(bsp->back, free_splitters);
	}

	#ifndef USE_MEMORY_MALLOC
	free(bsp);
	#else
	memory_Free(bsp);
	#endif
}



/*
==============
bsp_SolidLeafBsp
==============
*/
bsp_node_t *bsp_SolidLeafBsp(bsp_polygon_t *polygons)
{
	bsp_node_t *root = NULL;
	bsp_BuildSolidLeaf(&root, polygons);

	return root;
}




void bsp_DeleteSolidLeafBsp(bsp_node_t *bsp)
{
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	bsp_portal_t *portal;
	bsp_portal_t *next_portal;
	bsp_polygon_t *next;

	bsp_triangle_t *triangle;
	bsp_triangle_t *next_triangle;

	if(!bsp)
		return;

	if(bsp->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)bsp;
		polygon = leaf->polygons;

		while(polygon)
		{
			next = polygon->next;
			#ifndef USE_MEMORY_MALLOC
			free(polygon->vertices);
			free(polygon);
			#else
			memory_Free(polygon->vertices);
			memory_Free(polygon);
			#endif
			polygon = next;
		}
	}
	else
	{
		bsp_DeleteSolidLeafBsp(bsp->front);
		bsp_DeleteSolidLeafBsp(bsp->back);
	}

	#ifndef USE_MEMORY_MALLOC
	free(bsp);
	#else
	memory_Free(bsp);
	#endif
}




void bsp_NegateBsp(bsp_node_t *bsp)
{
	int i;
	int vert_count;
	bsp_polygon_t *polygon;
	vertex_t vertex;

	if(bsp->type == BSP_LEAF)
		return;

	bsp_NegateBsp(bsp->front);
	bsp_NegateBsp(bsp->back);

	polygon = bsp->splitter;
	vert_count = polygon->vert_count;

	for(i = 0; i < vert_count >> 1; i++)
	{
		vertex = polygon->vertices[i];

		vertex.normal.x = -vertex.normal.x;
		vertex.normal.y = -vertex.normal.y;
		vertex.normal.z = -vertex.normal.z;

		vertex.tangent.x = -vertex.tangent.x;
		vertex.tangent.y = -vertex.tangent.y;
		vertex.tangent.z = -vertex.tangent.z;

		vertex.tex_coord.x = -vertex.tex_coord.x;
		vertex.tex_coord.y = -vertex.tex_coord.y;

		polygon->vertices[i] = polygon->vertices[vert_count - i - 1];
		polygon->vertices[vert_count - i - 1] = vertex;


		polygon->vertices[i].normal.x = -polygon->vertices[i].normal.x;
		polygon->vertices[i].normal.y = -polygon->vertices[i].normal.y;
		polygon->vertices[i].normal.z = -polygon->vertices[i].normal.z;


		polygon->vertices[i].tangent.x = -polygon->vertices[i].tangent.x;
		polygon->vertices[i].tangent.y = -polygon->vertices[i].tangent.y;
		polygon->vertices[i].tangent.z = -polygon->vertices[i].tangent.z;


		polygon->vertices[i].tex_coord.x = -polygon->vertices[i].tex_coord.x;
		polygon->vertices[i].tex_coord.y = -polygon->vertices[i].tex_coord.y;
	}

	polygon->normal.x = -polygon->normal.x;
	polygon->normal.y = -polygon->normal.y;
	polygon->normal.z = -polygon->normal.z;
}



/*
==============
bsp_CountNodesAndLeaves
==============
*/
void bsp_CountNodesAndLeaves(bsp_node_t *bsp, int *leaves, int *nodes)
{
	static int level = -1;

	bsp_leaf_t *leaf;

	if(!bsp)
	{
		return;
	}


	level++;

	if(!level)
	{
		*leaves = 0;
		*nodes = 0;
	}

	(*nodes)++;			/* nodes are necessary to signal when a leaf has been reached,
						   so add a node even if this is a leaf... */

	if(bsp->type == BSP_LEAF)
	{
		/* count only empty leaves... */
		if(!(bsp->bm_flags & BSP_SOLID))
		{
			leaf = (bsp_leaf_t *)bsp;
			leaf->leaf_index = *leaves;
			(*leaves)++;
		}
	}
	else
	{
		bsp_CountNodesAndLeaves(bsp->front, leaves, nodes);
		bsp_CountNodesAndLeaves(bsp->back, leaves, nodes);
	}

	level--;
}



#define NUDGE_AMOUNT 0.005

/*
=============
bsp_IntersectBsp
=============
*/
int bsp_IntersectBsp(bsp_node_t *node, vec3_t start, vec3_t end)
{
	bsp_leaf_t *leaf;
	float d0;
	float d1;
	float frac;
	vec3_t mid;
	vec3_t vec;

	vec3_t point;
	vec3_t normal;

	if(!node)
		return 0;


	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;

		if(leaf->bm_flags & BSP_SOLID)
		{
			return 1;
		}
		else
		{
			return 0;
		}

	}
	else
	{

		if(node->splitter)
		{
			point = node->splitter->vertices[0].position;
			normal = node->splitter->normal;
		}
		else
		{
			point = node->point;
			normal = node->normal;
		}

		/*vec.x = start.x - node->splitter->vertices[0].position.x + node->splitter->normal.x * NUDGE_AMOUNT;
		vec.y = start.y - node->splitter->vertices[0].position.y + node->splitter->normal.y * NUDGE_AMOUNT;
		vec.z = start.z - node->splitter->vertices[0].position.z + node->splitter->normal.z * NUDGE_AMOUNT;*/

		vec.x = start.x - point.x + normal.x * NUDGE_AMOUNT;
		vec.y = start.y - point.y + normal.y * NUDGE_AMOUNT;
		vec.z = start.z - point.z + normal.z * NUDGE_AMOUNT;

		d0 = dot3(vec, normal);


		vec.x = end.x - point.x - normal.x * NUDGE_AMOUNT;
		vec.y = end.y - point.y - normal.y * NUDGE_AMOUNT;
		vec.z = end.z - point.z - normal.z * NUDGE_AMOUNT;

		d1 = dot3(vec, normal);



		if(d0 >= 0.0 && d1 >= 0.0)
		{
			return bsp_IntersectBsp(node->front, start, end);
		}
		else if(d0 < 0.0 && d1 < 0.0)
		{
			return bsp_IntersectBsp(node->back, start, end);
		}

		frac = d0 / (d0 - d1);

		//printf("%f\n", frac);

		if(frac > 1.0) frac = 1.0;
		else if(frac < 0.0) frac = 0.0;


		mid.x = start.x + (end.x - start.x) * frac;
		mid.y = start.y + (end.y - start.y) * frac;
		mid.z = start.z + (end.z - start.z) * frac;

		if(d0 < 0.0)
		{
			return (bsp_IntersectBsp(node->back, start, mid) | bsp_IntersectBsp(node->front, mid, end));
		}
		else
		{
			return (bsp_IntersectBsp(node->front, start, mid) | bsp_IntersectBsp(node->back, mid, end));
		}

		//return (bsp_IntersectBsp(node->front, start, mid) | bsp_IntersectBsp(node->back, mid, end));
	}

}


bsp_leaf_t *bsp_PointContents(bsp_node_t *bsp, vec3_t point)
{

    bsp_leaf_t *leaf;


    if(!bsp)
    {
        return NULL;
    }


    if(bsp->type == BSP_LEAF)
    {
        leaf = (bsp_leaf_t *)bsp;

        if(!(leaf->bm_flags & BSP_SOLID))
        {
            return leaf;
        }
    }
    else
    {
        switch(bsp_ClassifyPoint(point, bsp->splitter->vertices[0].position, bsp->splitter->normal))
        {
            case POINT_FRONT:
                return bsp_PointContents(bsp->front, point);
            break;

            case POINT_BACK:
                return bsp_PointContents(bsp->back, point);
            break;

            case POINT_CONTAINED:
                leaf = bsp_PointContents(bsp->front, point);

                if(!leaf)
                {
                    leaf = bsp_PointContents(bsp->back, point);
                }

                return leaf;
            break;
        }
    }

    return NULL;
}




/*
===============================================================================
===============================================================================
===============================================================================
*/


#ifndef REMOVE_ALL_DEPENDENCIES


bsp_polygon_t *bsp_DeepCopyPolygonsContiguous(bsp_polygon_t *src)
{
	int polygon_count = 0;
	int vert_count = 0;
	int i;
	int j;


	bsp_polygon_t *r = src;
	bsp_polygon_t *out;
	vertex_t *vertices;

	while(r)
	{
		polygon_count++;
		vert_count += r->vert_count;
		r = r->next;
	}

	#ifndef USE_MEMORY_MALLOC
	out = malloc(sizeof(bsp_polygon_t ) * polygon_count);
	vertices = malloc(sizeof(vertex_t) * vert_count );
	#else
	out = memory_Malloc(sizeof(bsp_polygon_t ) * polygon_count);
	vertices = memory_Malloc(sizeof(vertex_t) * vert_count);
	#endif

	i = 0;

	r = src;
	while(r)
	{
		out[i].b_used = r->b_used;
		out[i].material_index = r->material_index;
		out[i].normal = r->normal;
		out[i].tiling = r->tiling;
		out[i].vert_count = r->vert_count;
		out[i].triangle_group = r->triangle_group;

		if(i)
		{
			out[i].vertices = out[i - 1].vertices + out[i - 1].vert_count;
		}
		else
		{
			out[i].vertices = vertices;
		}

		for(j = 0; j < out[i].vert_count; j++)
		{
			out[i].vertices[j] = r->vertices[j];
		}

		out[i].next = out + i + 1;

		i++;
		r = r->next;
	}

	/*for(i = 0; i < polygon_count; i++)
	{
		out[i].next = out + i + 1;
	}*/

	out[i - 1].next = NULL;

	return out;
}



void bsp_DeletePolygonsContiguous(bsp_polygon_t *polygons)
{
	if(polygons)
	{
		#ifndef USE_MEMORY_MALLOC
		free(polygons->vertices);
		free(polygons);
		#else
		memory_Free(polygons->vertices);
		memory_Free(polygons);
		#endif
	}

}

/* works well only on convex and non-degenerate faces... */
int bsp_RecursiveTriangulate(vertex_t *in_verts, int in_vert_count, vertex_t *out_verts, int *out_vert_count, int *in_indexes, int *out_indexes)
{
	int i;
	int c = in_vert_count;
	int j;
	int k = c;

	int ai = -1;
	int bi = -1;

	vec3_t tri[3];


	vertex_t *vin;
	vertex_t *vout;

	int *iin;
	int *iout;
	//float *vout1;

	vec3_t a;
	vec3_t b;
	vec3_t face_normal;
	vec3_t h;

	vec3_t s;
	vec3_t t;

	int p = 0;
	int best = 999999999;

	if(in_vert_count < 2)
	{
		return 0;
	}
	else if(in_vert_count == 3)
	{
		out_verts[(*out_vert_count)	   ] = in_verts[0];
		out_verts[(*out_vert_count) + 1] = in_verts[1];
		out_verts[(*out_vert_count) + 2] = in_verts[2];

		if(out_indexes)
		{
			out_indexes[(*out_vert_count)	 ] = in_indexes[0];
			out_indexes[(*out_vert_count) + 1] = in_indexes[1];
			out_indexes[(*out_vert_count) + 2] = in_indexes[2];
		}

		(*out_vert_count) += 3;

		return 1;
	}


	a = in_verts[0].position;
	b = in_verts[1].position;
	h = in_verts[2].position;

	s.x = b.x - a.x;
	s.y = b.y - a.y;
	s.z = b.z - a.z;

	t.x = h.x - a.x;
	t.y = h.y - a.y;
	t.z = h.z - a.z;

	face_normal = cross(s, t);

	for(i = 2; i < c; i++)
	{
		b = in_verts[i].position;

		/* for each vertex in the polygon
		face, build an egde... */
		s.x = b.x - a.x;
		s.y = b.y - a.y;
		s.z = b.z - a.z;
		p = 0;
		for(j = 1; j < c; j++)
		{
			if(j == i) continue;
			h = in_verts[j].position;

			t.x = h.x - a.x;
			t.y = h.y - a.y;
			t.z = h.z - a.z;

			h = cross(s, t);

			/* the dot product signal
			will depend in which side of the
			splitting edge the vertices are... */
			if(dot3(h, face_normal) < 0.0)
			{
				p--;
			}
			else
			{
				p++;
			}
		}
		p = abs(p);
		/* keep the edge that
		splits the polygon more
		evenly (and the index of
		it's first vertex)... */
		if(p < best)
		{
			best = p;
			bi = i;
		}

	}

	k = in_vert_count >> 1;

	#ifndef USE_MEMORY_MALLOC
	vin = malloc(sizeof(vertex_t) * in_vert_count);
	#else
	vin = memory_Malloc(sizeof(vertex_t) * in_vert_count);
	#endif

	if(out_indexes)
	{
		#ifndef USE_MEMORY_MALLOC
		iin = malloc(sizeof(int) * in_vert_count);
		#else
		iin = memory_Malloc(sizeof(int) * in_vert_count);
		#endif

		for(i = 0; i < bi + 1; i++)
		{
			iin[i] = in_indexes[i];
		}
	}

	for(i = 0; i < bi + 1; i++)
	{
		vin[i] = in_verts[i];
	}
	bsp_RecursiveTriangulate(vin, bi + 1, out_verts, out_vert_count, iin, out_indexes);


	if(out_indexes)
	{
		for(i = bi; i < in_vert_count + 1; i++)
		{
			iin[i - bi] = in_indexes[i % in_vert_count];
		}
	}

	for(i = bi; i < in_vert_count + 1; i++)
	{
		vin[i - bi] = in_verts[i % in_vert_count];
	}
	bsp_RecursiveTriangulate(vin, in_vert_count - bi + 1, out_verts, out_vert_count, iin, out_indexes);

	#ifndef USE_MEMORY_MALLOC
	free(vin);
	#else
	memory_Free(vin);
	#endif

	if(out_indexes)
	{
		#ifndef USE_MEMORY_MALLOC
		free(iin);
		#else
		memory_Free(iin);
		#endif
	}

	return 1;
}


void bsp_TriangulatePolygon(bsp_polygon_t *polygon, vertex_t **vertices, int *vertex_count)
{

	int count = 0;
	vertex_t *verts = NULL;

	if(polygon->vert_count >= 3)
	{
		#ifndef USE_MEMORY_MALLOC
		verts = malloc(sizeof(vertex_t ) * (polygon->vert_count - 2) * 3);
		#else
		verts = memory_Malloc(sizeof(vertex_t ) * (polygon->vert_count - 2) * 3);
		#endif

		if(polygon->vert_count == 3)
		{
			verts[0] = polygon->vertices[0];
			verts[1] = polygon->vertices[1];
			verts[2] = polygon->vertices[2];

			*vertices = verts;
			*vertex_count = 3;
			return;
		}

		bsp_RecursiveTriangulate(polygon->vertices, polygon->vert_count, verts, &count, NULL, NULL);
	}

	*vertex_count = count;
	*vertices = verts;
	return;
}


void bsp_TriangulatePolygonIndexes(bsp_polygon_t *polygon, int **indexes, int *index_count)
{
	int count = 0;
	int i;
	int c;
	vertex_t *verts = NULL;
	int *in_indexes = NULL;
	int *out_indexes = NULL;
	int out_index_count = 0;

	if(polygon->vert_count >= 3)
	{
		out_index_count = (polygon->vert_count - 2) * 3;

		#ifndef USE_MEMORY_MALLOC
		out_indexes = malloc(sizeof(int ) * out_index_count);
		#else
		out_indexes = memory_Malloc(sizeof(int ) * out_index_count);
		#endif

		/* avoid the whole thing if the polygon is a triangle... */
		if(polygon->vert_count == 3)
		{
			for(i = 0; i < 3; i++)
			{
				out_indexes[i] = i;
			}

			*indexes = out_indexes;
			*index_count = 3;

			return;
		}

		/* not a triangle... */

		#ifndef USE_MEMORY_MALLOC
		verts = malloc(sizeof(vertex_t ) * (polygon->vert_count - 2) * 3);
		in_indexes = malloc(sizeof(int) * polygon->vert_count);
		#else
		verts = memory_Malloc(sizeof(vertex_t ) * (polygon->vert_count - 2) * 3);
		in_indexes = memory_Malloc(sizeof(int) * polygon->vert_count);
		#endif

		c = polygon->vert_count;

		for(i = 0; i < c; i++)
		{
			in_indexes[i] = i;
		}

		bsp_RecursiveTriangulate(polygon->vertices, polygon->vert_count, verts, &count, in_indexes, out_indexes);

		/* welp, fuck this... */

		#ifndef USE_MEMORY_MALLOC
		free(verts);
		free(in_indexes);
		#else
		memory_Free(verts);
		memory_Free(in_indexes);
		#endif
	}

	*indexes = out_indexes;
	*index_count = out_index_count;

	return;
}

void bsp_TriangulatePolygonsIndexes(bsp_polygon_t *polygons, int **indexes, int *index_count)
{
	int count = 0;
	int max_verts = 0;
	int i;
	int c;
	int r;
	int cursor = 0;
	vertex_t *verts = NULL;
	int *in_indexes = NULL;
	int *out_indexes = NULL;
	bsp_polygon_t *polygon;
	int out_index_count = 0;
	int in_index_count = 0;
	int prev_last_index = 0;
	int vert_count;



	polygon = polygons;

	while(polygon)
	{
		assert(polygon->vert_count >= 3);
		count += (polygon->vert_count - 2) * 3;

		if(polygon->vert_count > max_verts)
		{
			max_verts = polygon->vert_count;
		}

		polygon = polygon->next;
	}


	if(!(*indexes))
	{
		#ifndef USE_MEMORY_MALLOC
		out_indexes = malloc(sizeof(int) * count * 2);
		#else
		out_indexes = memory_Malloc(sizeof(int) * count * 10);
		#endif
	}
	else
	{
		out_indexes = *indexes;
	}

	#ifndef USE_MEMORY_MALLOC
	in_indexes = malloc(sizeof(int) * max_verts * 3);
	verts = malloc(sizeof(vertex_t) * (max_verts - 2) * 3);
	#else
	in_indexes = memory_Malloc(sizeof(int) * max_verts * 3);
	verts = memory_Malloc(sizeof(vertex_t) * (max_verts - 2) * 3);
	#endif
	polygon = polygons;

	count = 0;
	out_index_count = 0;
	r = 0;

	while(polygon)
	{
		/* avoid the whole thing if the polygon is a triangle... */
		if(polygon->vert_count == 3)
		{
			for(i = 0; i < 3; i++)
			{
				out_indexes[i + cursor] = i + prev_last_index;
			}

			prev_last_index += 3;
			cursor += 3;
		}
		else
		{
			//c = polygon->vert_count;

			vert_count = polygon->vert_count;

			for(i = 0; i < vert_count; i++)
			{
				in_indexes[i] = i;
			}

			out_index_count = 0;

			bsp_RecursiveTriangulate(polygon->vertices, polygon->vert_count, verts, &out_index_count, in_indexes, out_indexes + cursor);

			for(i = 0; i < out_index_count; i++)
			{
				out_indexes[i + cursor] += prev_last_index;
			}
			prev_last_index += vert_count;
			cursor += out_index_count;
		}

		polygon = polygon->next;
	}

	#ifndef USE_MEMORY_MALLOC
	free(verts);
	free(in_indexes);
	#else
	memory_Free(verts);
	memory_Free(in_indexes);
	#endif

	*indexes = out_indexes;
	*index_count = cursor;

	return;
}


void bsp_RecursiveQuickHull(vec2_t *in_verts, int in_vert_count, vec2_t *hull_verts, int *hull_vert_count, vec2_t edge_a, vec2_t edge_b, vec2_t edge_dir)
{

	int i;

	int max_index = -1;
	float max_d = -999999.9;
	float d;

	vec2_t out_a;
	vec2_t out_b;
	vec2_t out_dir;

	int out_count = 0;
	vec2_t *out;

	vec2_t v;


	if(in_vert_count < 2)
	{
		if(in_vert_count)
		{
			hull_verts[*hull_vert_count] = in_verts[0];
			(*hull_vert_count)++;
		}

		return;
	}

	#ifndef USE_MEMORY_MALLOC
	out = malloc(sizeof(vec2_t ) * in_vert_count);
	#else
	out = memory_Malloc(sizeof(vec2_t ) * in_vert_count);
	#endif


	/* get farthest vertex from the current edge... */
	for(i = 0; i < in_vert_count; i++)
	{
		v.x = in_verts[i].x - edge_a.x;
		v.y = in_verts[i].y - edge_a.y;

		d = dot2(v, edge_dir);

		if(d > max_d)
		{
			if(in_verts[i].x != edge_a.x || in_verts[i].y != edge_a.y)
			{
				max_d = d;
				max_index = i;
		    }
		}
	}

	assert(max_index != -1);

	out_a = edge_a;
	out_b = in_verts[max_index];

	out_dir.x = out_b.x - out_a.x;
	out_dir.y = out_b.y - out_a.y;


	d = out_dir.x;
	out_dir.x = out_dir.y;
	out_dir.y = -d;

	out_dir = normalize2(out_dir);

	out_count = 0;

	/* edge_a, in_verts[max_index] and edge_b
	define two edges. Get the vertices that are
	in front of the first edge... */
	for(i = 0; i < in_vert_count; i++)
	{
		v.x = in_verts[i].x - out_a.x;
		v.y = in_verts[i].y - out_a.y;

		d = dot2(v, out_dir);

		if(d > FUZZY_ZERO)
		{
			out[out_count++] = in_verts[i];
		}
	}

	bsp_RecursiveQuickHull(out, out_count, hull_verts, hull_vert_count, out_a, out_b, out_dir);

	hull_verts[*hull_vert_count] = out_b;

	(*hull_vert_count)++;

	out_count = 0;
	out_a = in_verts[max_index];
	out_b = edge_b;

	out_dir.x = out_b.x - out_a.x;
	out_dir.y = out_b.y - out_a.y;


	d = out_dir.x;
	out_dir.x = out_dir.y;
	out_dir.y = -d;

	out_dir = normalize2(out_dir);

	for(i = 0; i < in_vert_count; i++)
	{
		v.x = in_verts[i].x - out_a.x;
		v.y = in_verts[i].y - out_a.y;

		d = dot2(v, out_dir);

		if(d > 0.0)
		{
			out[out_count++] = in_verts[i];
		}

	}

	bsp_RecursiveQuickHull(out, out_count, hull_verts, hull_vert_count, out_a, out_b, out_dir);

	#ifndef USE_MEMORY_MALLOC
	free(out);
	#else
	memory_Free(out);
	#endif

}


void bsp_QuickHull(vec2_t *in_verts, int in_vert_count, vec2_t **hull_verts, int *hull_vert_count)
{
	float x_max = -99999999.9;
	float x_min = 99999999.9;

	int x_max_index;
	int x_min_index;

	int i;
	//int c = *vert_count;

	//int pos_index;
	//float pos_d = -99999999.9;
	//int neg_index;
	//float neg_d = 99999999.9;
	float d;
	float p;

	vec2_t a;
	vec2_t b;
	vec2_t ab;

	int out_count = 0;
	vec2_t *out;
	int hull_count = 0;
	vec2_t *hull;
	vec2_t *t;

	vec2_t v;


	if(in_vert_count == 3)
	{
		#ifndef USE_MEMORY_MALLOC
		*hull_verts = malloc(sizeof(vec2_t) * 3);
		#else
		*hull_verts = memory_Malloc(sizeof(vec2_t) * 3);
		#endif
		(*hull_verts)[0] = in_verts[0];
		(*hull_verts)[1] = in_verts[1];
		(*hull_verts)[2] = in_verts[2];

		*hull_vert_count = 3;

		return;
	}
	else if(in_vert_count < 3)
	{
		*hull_verts = NULL;
		*hull_vert_count = 0;
		return;
	}


	#ifndef USE_MEMORY_MALLOC
	out = malloc(sizeof(vec2_t ) * in_vert_count);
	hull = malloc(sizeof(vec2_t ) * in_vert_count * 2);
	#else
	out = memory_Malloc(sizeof(vec2_t ) * in_vert_count);
	hull = memory_Malloc(sizeof(vec2_t ) * in_vert_count * 2);
	#endif

	for(i = 0; i < in_vert_count; i++)
	{
		if(in_verts[i].x > x_max)
		{
			x_max = in_verts[i].x;
			x_max_index = i;
		}
		if(in_verts[i].x < x_min)
		{
			x_min = in_verts[i].x;
			x_min_index = i;
		}
	}

	/* first edge... */
	a = in_verts[x_min_index];
	b = in_verts[x_max_index];


	ab.x = b.x - a.x;
	ab.y = b.y - a.y;

	d = ab.x;
	ab.x = ab.y;
	ab.y = -d;

	ab = normalize2(ab);

	/* first hull index... */
	hull[hull_count++] = a;

	for(i = 0; i < in_vert_count; i++)
	{
		if(i == x_min_index)
			continue;

		if(i == x_max_index)
			continue;

		v.x = in_verts[i].x - a.x;
		v.y = in_verts[i].y - a.y;

		d = dot2(v, ab);


		if(d > FUZZY_ZERO)
		{
			if(in_verts[i].x != a.x || in_verts[i].y != a.y)
			{
				if(in_verts[i].x != b.x || in_verts[i].y != b.y)
				{
					out[out_count++] = in_verts[i];
				}
			}

		}

	}

	/* first half of the hull... */
	bsp_RecursiveQuickHull(out, out_count, hull, &hull_count, a, b, ab);

	hull[hull_count++] = b;

	out_count = 0;

	ab.x = -ab.x;
	ab.y = -ab.y;

	for(i = 0; i < in_vert_count; i++)
	{
		if(i == x_min_index)
			continue;

		if(i == x_max_index)
			continue;

		v.x = in_verts[i].x - a.x;
		v.y = in_verts[i].y - a.y;

		d = dot2(v, ab);


		if(d >= 0.0)
		{
			if(in_verts[i].x != a.x || in_verts[i].y != a.y)
			{
				if(in_verts[i].x != b.x || in_verts[i].y != b.y)
				{
					out[out_count++] = in_verts[i];
				}
			}
		}
	}

	bsp_RecursiveQuickHull(out, out_count, hull, &hull_count, b, a, ab);


	#ifndef USE_MEMORY_MALLOC
	*hull_verts = malloc(sizeof(vec2_t ) * hull_count);
	#else
	*hull_verts = memory_Malloc(sizeof(vec2_t ) * hull_count);
	#endif
	*hull_vert_count = hull_count;

	t = *hull_verts;

	for(i = 0; i < hull_count; i++)
	{
		t[i] = hull[i];
	}


	#ifndef USE_MEMORY_MALLOC
	free(hull);
	free(out);
	#else
	memory_Free(hull);
	memory_Free(out);
	#endif





}


bsp_polygon_t *bsp_ClipContiguousPolygonsToBsp(int op, bsp_node_t *bsp0, bsp_node_t *bsp1, bsp_polygon_t *polygons0, bsp_polygon_t *polygons1, int do_copy)
{
	bsp_polygon_t *p = NULL;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *s = NULL;
	bsp_polygon_t *clipped = NULL;
	bsp_node_t *bsp2;


	/* this convert the polygons from a contiguous list to a linked list... */
	r = bsp_DeepCopyPolygons(polygons0);
	while(r)
	{
		p = r->next;
		r->next = NULL;

		/* first clip the current brush polygons against the
		other brush. This essentially carves out the part
		that's intersecting the subtractive brush... */
		r = bsp_OpPolygonToBsp(CSG_OP_UNION, bsp0, r, 0);

		if(r)
		{
			s = r;
			while(s->next) s = s->next;

			s->next = clipped;
			clipped = r;
		}

		r = p;
	}

	if(op == CSG_OP_SUBTRACTION)
	{
		/* copy the subtractive brush's polygons... */
		r = bsp_DeepCopyPolygons(polygons1);

		/* HACK: necessary so multiple subtractive brushes can properly act on normal brushes. The
		problem happens because when clipping the subtractive brush against the original brush
		(in order to obtain the inside polygons), the original brush's bsp is used (which is built
		with the base polygons). This bsp does not reflect the result of past subtraction operations,
		and so the inside polygons gets generated without taking into account the result of previous
		subtractive clips. To solve this, we build a temporary bsp using the clipped polygons from
		the brush. This way, this subtraction operation will see previous subtractions... */
		s = bsp_DeepCopyPolygons(polygons0);
		bsp2 = bsp_SolidBsp(s);

		while(r)
		{
			p = r->next;
			r->next = NULL;
			/* if we're doing a subtraction, take the original polygons
			of the subtractive brush and do a subtraction against the first
			brush bsp... */
			r = bsp_OpPolygonToBsp(CSG_OP_SUBTRACTION, bsp2, r, 0);

			if(r)
			{
				s = r;
				while(s->next) s = s->next;

				s->next = clipped;
				clipped = r;
			}

			r = p;
		}

		bsp_DeleteSolidBsp(bsp2, 1);


		r = clipped;
		clipped = NULL;

		/* HACK: whenever two normal brushes touch each other face to face,
		and those faces have the same size, both faces will be clipped,
		and a "hole" will be formed.

		This "hole" would only exist if their clipped polygons were to be
		used to build a new bsp. Which is exactly what happens here.

		In order to generate the inside polygons, the subtractive brush's
		polygons get clipped against a bsp formed by the clipped polygons
		from the other brush. This bsp will have a "hole" in it, making
		the clipping code sometimes incorrectly consider the whole subtractive
		brush to be in solid space, given that there's no polygon in this
		"hole" to close the mesh...

		To fix the whole brush being in solid space we also clip the inside
		polygons against the original brush's bsp. This bsp is guaranteed
		to have no holes... */

		while(r)
		{
            p = r->next;
			r->next = NULL;

			r = bsp_OpPolygonToBsp(CSG_OP_INTERSECTION, bsp1, r, 0);

			if(r)
			{
				s = r;
				while(s->next) s = s->next;

				s->next = clipped;
				clipped = r;
			}

			r = p;
		}

	}

	if(clipped)
	{
		bsp_DeletePolygonsContiguous(polygons0);
		r = bsp_DeepCopyPolygonsContiguous(clipped);
		bsp_DeletePolygons(clipped);
	}
	return r;
}


/*
==============
bsp_ClipBrushes
==============
*/
bsp_polygon_t *bsp_ClipBrushes(brush_t *brush_list, int brush_list_count)
{

	int i;
	int c;
	bsp_node_t *bsp_a;
	bsp_node_t *bsp_b;
	bsp_polygon_t *p;
	bsp_polygon_t *polygons_a = NULL;
	bsp_polygon_t *polygons_b = NULL;
	brush_t *b;

	bsp_polygon_t *set_a = NULL;
	bsp_polygon_t *set_b = NULL;
	bsp_polygon_t *r;
	bsp_polygon_t *s;

	c = brush_list_count;

	SDL_sem *sem0;
	SDL_sem *sem1;


	//sem0 = SDL_CreateSemaphore(0);
	//sem1 = SDL_CreateSemaphore(0);

	float start;
	float end;

	/* to do the union operation between two brushes, it's necessary
	to "push" brush A's polygons through brush B's bsp and vice versa,
	retaining just those polygons that are on the outside. */

	/* merging bsps direcly instead of incremental set-op
	might work better...  */

	start = engine_GetDeltaTime();

	if(c)
	{

		i = 0;

		do
		{
			/* build the polygon list for the first brush in the list
			(brush A in the first iteration of the loop below)... */

			if(brush_list[i].type == BRUSH_INVALID)
				continue;

			polygons_a = bsp_DeepCopyPolygons(brush_list[i].base_polygons);

			i++;

		}while(!polygons_a && i < c);

		brush_list[i - 1].bm_flags &= ~BRUSH_MOVED;

		for(; i < c; i++)
		{
			//printf("%d/%d\n", i, c);
			if(brush_list[i].type == BRUSH_INVALID)
				continue;

			polygons_b = bsp_DeepCopyPolygons(brush_list[i].base_polygons);

			if(!polygons_b)
				continue;


			bsp_a = NULL;
			/* build the bsp for the polygons of brush A... */
			bsp_BuildSolid(&bsp_a, polygons_a, 0, 0);


			bsp_b = NULL;
			/* build the bsp for the polygons of brush B... */
			bsp_BuildSolid(&bsp_b, polygons_b, 0, 0);

			/* clip the polygons that belong to bsp_a against
			bsp_b... */
			polygons_a = bsp_ClipBspToBsp(bsp_b, bsp_a);


			/* clip the polygons that belong to bsp_b against
			bsp_a... */
			polygons_b = bsp_ClipBspToBsp(bsp_a, bsp_b);



			/* get rid of those bsp's, they're
			useless now. Such useless motherfuckers... */
			/* NOTE: this has become a HUGE bottleneck... */
			bsp_DeleteSolidBsp(bsp_a, 1);
			bsp_DeleteSolidBsp(bsp_b, 1);

			p = polygons_a;

			/* link the two lists of polygons... */
			while(p->next)
			{
				p = p->next;
			}
			p->next = polygons_b;

			brush_list[i].bm_flags &= ~BRUSH_MOVED;


		}

		polygons_b = polygons_a;

		while(polygons_b)
		{
			polygons_b->b_used = 0;
			polygons_b = polygons_b->next;
		}

	}

	end = engine_GetDeltaTime();

	//printf("clipping took %f\n", end - start);

	//SDL_DestroySemaphore(sem0);
	//SDL_DestroySemaphore(sem1);



	return polygons_a;
}

bsp_polygon_t *bsp_ClipBrushes2(brush_t *brush_list, int brush_list_count)
{
	int i;
	int c;
	bsp_node_t *bsp_a;
	bsp_node_t *bsp_b;
	bsp_polygon_t *p;
	bsp_polygon_t *polygons_a = NULL;
	bsp_polygon_t *polygons_b = NULL;
	bsp_polygon_t *polygons_c = NULL;
	brush_t *b;

	bsp_polygon_t *set_a = NULL;
	bsp_polygon_t *set_b = NULL;
	bsp_polygon_t *r;
	bsp_polygon_t *s;

	c = brush_list_count;

	SDL_sem *sem0;
	SDL_sem *sem1;


	//sem0 = SDL_CreateSemaphore(0);
	//sem1 = SDL_CreateSemaphore(0);

	float start;
	float end;

	/* to do the union operation between two brushes, it's necessary
	to "push" brush A's polygons through brush B's bsp and vice versa,
	retaining just those polygons that are on the outside. */

	/* merging bsps direcly instead of incremental set-op
	might work better...  */

	start = engine_GetDeltaTime();

	if(c)
	{

		for(i = 0; i < c; i++)
		{
			if(brush_list[i].type == BRUSH_INVALID)
				continue;

			p = bsp_DeepCopyPolygons(brush_list[i].base_polygons);
			brush_list[i].bm_flags &= ~BRUSH_MOVED;

			if(p)
			{
				r = p;

				while(r->next) r = r->next;
				r->next = polygons_a;
				polygons_a = p;
			}

		}

		polygons_b = bsp_DeepCopyPolygons(polygons_a);
		bsp_BuildSolid(&bsp_a, polygons_a, 0, 0);

		r = polygons_b;
		//polygons_c = NULL;
		polygons_b = NULL;
		while(r)
		{
			p = r->next;
			r = bsp_ClipPolygonToBsp(bsp_a, r, 0);

			if(r)
			{
				s = r;
				while(s->next) s = s->next;

				s->next = polygons_b;
				polygons_b = r;
			}

			r = p;
		}

		polygons_a = polygons_b;

		while(polygons_b)
		{
			polygons_b->b_used = 0;
			polygons_b = polygons_b->next;
		}

		bsp_DeleteSolidBsp(bsp_a, 1);
	}

	end = engine_GetDeltaTime();

	//printf("clipping took %f\n", end - start);

	//SDL_DestroySemaphore(sem0);
	//SDL_DestroySemaphore(sem1);



	return polygons_a;
}


bsp_polygon_t *bsp_ClipCoplanarPolygons(bsp_polygon_t *clip_to, bsp_polygon_t *to_clip)
{
    bsp_node_t *clip_to_bsp;
    bsp_node_t *clip_to_node;
    bsp_leaf_t *leaf;

    bsp_polygon_t *clipped_polygons = NULL;

	int i;
	int j;

	int v0;
	int v1;

	vec3_t edge_vec;
	vec3_t edge_normal;

	int safe_size;

	safe_size = sizeof(bsp_node_t) + sizeof(bsp_leaf_t);

    if(clip_to && to_clip)
	{
		clip_to_bsp = memory_Calloc(1, sizeof(bsp_node_t));
		clip_to_node = clip_to_bsp;

		for(i = 0; i < clip_to->vert_count; i++)
		{
			v0 = i;
			v1 = (i + 1) % clip_to->vert_count;

			edge_vec.x = clip_to->vertices[v1].position.x - clip_to->vertices[v0].position.x;
			edge_vec.y = clip_to->vertices[v1].position.y - clip_to->vertices[v0].position.y;
			edge_vec.z = clip_to->vertices[v1].position.z - clip_to->vertices[v0].position.z;

			edge_vec = normalize3(edge_vec);

			edge_normal = cross(edge_vec, clip_to->normal);

			clip_to_node->type = BSP_NODE;
			clip_to_node->normal = edge_normal;
			clip_to_node->point = clip_to->vertices[v0].position;

			leaf = memory_Calloc(1, sizeof(bsp_leaf_t));
			leaf->type = BSP_LEAF;

			clip_to_node->front = (bsp_node_t *)leaf;
			clip_to_node->back = memory_Calloc(1, safe_size);

			clip_to_node = clip_to_node->back;
		}

		leaf = (bsp_leaf_t *)clip_to_node;

		leaf->type = BSP_LEAF;
		leaf->bm_flags |= BSP_SOLID;

		clipped_polygons = bsp_OpPolygonToBsp(CSG_OP_UNION, clip_to_bsp, to_clip, 0);

		bsp_DeleteSolidBsp(clip_to_bsp, 0);
	}

	return clipped_polygons;
}


void bsp_ClipCoplanarLeafPolygons(bsp_node_t *bsp)
{
	bsp_node_t *node;
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;

	bsp_polygon_t *other;
	bsp_polygon_t *next_other;
	bsp_polygon_t *prev;
	bsp_polygon_t *clipped;

	bsp_polygon_t *clip_list = NULL;

	bsp_edge_t *edges = NULL;
	bsp_edge_t *edge = NULL;
	bsp_edge_t *next_edge = NULL;

	vec3_t edge_vec;
	vec3_t edge_normal;

	bsp_node_t *polygon_bsp = NULL;
	bsp_node_t *polygon_bsp_node = NULL;
	bsp_leaf_t *polygon_bsp_leaf = NULL;

	int v0;
	int v1;

	vertex_t *p0;
	vertex_t *p1;

	vec3_t edge0;
	vec3_t edge1;
	vec3_t edge_pvec;

	vertex_t *r0;
	vertex_t *r1;

	int i;
	int j;
;

	node = bsp;

	if(!node)
	{
		return;
	}

	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;

        if(!(leaf->bm_flags & BSP_SOLID))
		{
			polygon = leaf->polygons;

			while(polygon)
			{
				polygon_bsp = memory_Calloc(1, sizeof(bsp_node_t));
				polygon_bsp_node = polygon_bsp;

				/* first, build a small bsp tree using the
				polygon's edges as "planes"... */

				for(i = 0; i < polygon->vert_count; i++)
				{
					v0 = i;
					v1 = (i + 1) % polygon->vert_count;

					edge_vec.x = polygon->vertices[v1].position.x - polygon->vertices[v0].position.x;
					edge_vec.y = polygon->vertices[v1].position.y - polygon->vertices[v0].position.y;
					edge_vec.z = polygon->vertices[v1].position.z - polygon->vertices[v0].position.z;

					edge_vec = normalize3(edge_vec);

					edge_normal = cross(edge_vec, polygon->normal);

					polygon_bsp_node->type = BSP_NODE;
					polygon_bsp_node->normal = edge_normal;
					polygon_bsp_node->point = polygon->vertices[v0].position;

					polygon_bsp_leaf = memory_Calloc(1, sizeof(bsp_leaf_t));
					polygon_bsp_leaf->type = BSP_LEAF;

					polygon_bsp_node->front = (bsp_node_t *)polygon_bsp_leaf;
					polygon_bsp_node->back = memory_Calloc(1, sizeof(bsp_leaf_t) + sizeof(bsp_node_t));

					polygon_bsp_node = polygon_bsp_node->back;
				}

				polygon_bsp_leaf = (bsp_leaf_t *)polygon_bsp_node;

				polygon_bsp_leaf->type = BSP_LEAF;
				polygon_bsp_leaf->bm_flags |= BSP_SOLID;


				prev = polygon;

				clip_list = NULL;

				other = polygon->next;
				/* find out which polygons intersect this bsp... */
				while(other)
				{
					next_other = other->next;

					if(dot3(polygon->normal, other->normal) == 1.0)
					{
						for(i = 0; i < other->vert_count; i++)
						{
							v0 = i;
							v1 = (i + 1) % other->vert_count;

							if(bsp_IntersectBsp(polygon_bsp, other->vertices[v0].position, other->vertices[v1].position))
							{
								/* if this polygon intersects the bsp, link it to a list
								to be clipped afterwards... */

								prev->next = next_other;
                                other->next = clip_list;
                                clip_list = other;

                                other = prev;

								break;
							}
						}
					}

					prev = other;

					other = next_other;
				}

				clip_list = bsp_ClipPolygonsToBsp(polygon_bsp, clip_list, 0);

				if(clip_list)
				{
					/* link whatever survived the clipping
					back into the leaf's polygon list... */
					next_other = polygon->next;

					polygon->next = clip_list;

					while(clip_list->next)
					{
						clip_list = clip_list->next;
					}

					clip_list->next = next_other;
				}

				bsp_DeleteSolidBsp(polygon_bsp, 0);

				polygon = polygon->next;
			}
		}

	}
	else
	{
        bsp_ClipCoplanarLeafPolygons(node->front);
		bsp_ClipCoplanarLeafPolygons(node->back);
	}
}

void bsp_MergeCoplanarLeafTriangles(bsp_node_t *bsp, int *triangle_count)
{
	#if 1
	bsp_node_t *node;
	bsp_leaf_t *leaf;

	bsp_triangle_t *triangle;
	bsp_triangle_t *prev;
	vec3_t triangle_normal;

	bsp_triangle_t *other;
	vec3_t other_normal;

	vec3_t edge0;
	vec3_t edge1;

	int v0t0i;
	int v1t0i;
	vertex_t *v0t0;
	vertex_t *v1t0;


	int v0t1i;
	int v1t1i;
	vertex_t *v0t1;
	vertex_t *v1t1;

	bsp_triangle_t temp_triangle;

	int v0;
	int v1;

	float dist;

	int removed_triangles = 0;

	float angle;

	int i;
	int j;

	static int depth = -1;


	depth++;


	if(!depth)
	{
		*triangle_count = 0;
	}

	node = bsp;

	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;

		if(!(leaf->bm_flags & BSP_SOLID))
		{
			triangle = leaf->triangles;

			while(triangle)
			{
				//dist = dot3(triangle->normal, triangle->vertices[0].position);

				do
				{
					removed_triangles = 0;

					prev = triangle;
					other = triangle->next;

					while(other)
					{
						/* comparing floats for equality is unsafe in general,
						but here it's safe since all the triangles generated from
						the same polygon inherited the same normal from
						said polygon... */
						if(triangle->normal.x == other->normal.x &&
						   triangle->normal.y == other->normal.y &&
						   triangle->normal.z == other->normal.z)
						{
							for(i = 0; i < 3; i++)
							{
								v0t0 = &triangle->vertices[i];
								v1t0 = &triangle->vertices[(i + 1) % 3];

								for(j = 0; j < 3; j++)
								{
									v0t1 = &other->vertices[j];
									v1t1 = &other->vertices[(j + 1) % 3];

									if(v0t0->position.x == v1t1->position.x &&
									   v0t0->position.y == v1t1->position.y &&
									   v0t0->position.z == v1t1->position.z)
									{
										if(v1t0->position.x == v0t1->position.x &&
										   v1t0->position.y == v0t1->position.y &&
										   v1t0->position.z == v0t1->position.z)
										{
											/* those triangles share an edge, so now test
											the other two edges. If a pair of edges that
											contain the same vertex as the shared edge are
											parallel, it means the triangle can be merged... */


											v0 = i - 1;

											if(v0 < 0)
											{
												v0 = 2;
											}

											v1 = i;

											/* the edge before the shared edge on triangle0... */
											edge0.x = v0t0->position.x - triangle->vertices[v0].position.x;
											edge0.y = v0t0->position.y - triangle->vertices[v0].position.y;
											edge0.z = v0t0->position.z - triangle->vertices[v0].position.z;


											v0 = (j + 1) % 3;
											v1 = (j + 2) % 3;

											/* the edge after the shared edge on triangle1... */
											edge1.x = other->vertices[v1].position.x - v1t1->position.x;
											edge1.y = other->vertices[v1].position.y - v1t1->position.y;
											edge1.z = other->vertices[v1].position.z - v1t1->position.z;

											edge0 = normalize3(edge0);
											edge1 = normalize3(edge1);

											//angle = dot3(normalize3(edge0), normalize3(edge1));

											//if((angle <= 1.0 + FLT_EPSILON) && (angle >= 1.0 - FLT_EPSILON))
											//if(angle >= 0.9999)


											if((edge0.x <= edge1.x + FLT_EPSILON * 2.0) && (edge0.x >= edge1.x - FLT_EPSILON * 2.0))
											{
                                                if((edge0.y <= edge1.y + FLT_EPSILON * 2.0) && (edge0.y >= edge1.y - FLT_EPSILON * 2.0))
												{
													if((edge0.z <= edge1.z + FLT_EPSILON * 2.0) && (edge0.z >= edge1.z - FLT_EPSILON * 2.0))
													{
														prev = other->prev;
														prev->next = other->next;

														if(other->next)
														{
															other->next->prev = prev;
														}


														/* make the first vertex of the shared edge on triangle 0
														be the only vertex not contained in the shared edge on
														triangle 1... */
														//triangle->vertices[i] = other->vertices[(j + 2) % 3];

														*v0t0 = other->vertices[(j + 2) % 3];

														memory_Free(other);

														other = prev;

														removed_triangles++;

														leaf->triangle_count--;

														goto _next_triangle;
													}
												}
											}


											v0 = (i + 1) % 3;
											v1 = (i + 2) % 3;

											/* the edge after the shared edge on triangle0... */
											edge0.x = triangle->vertices[v1].position.x - v1t0->position.x;
											edge0.y = triangle->vertices[v1].position.y - v1t0->position.y;
											edge0.z = triangle->vertices[v1].position.z - v1t0->position.z;


											v0 = j - 1;

											if(v0 < 0)
											{
												v0 = 2;
											}

											v1 = j;

											/* the edge before the shared edge on triangle1... */
											edge1.x = v0t1->position.x - other->vertices[v0].position.x;
											edge1.y = v0t1->position.y - other->vertices[v0].position.y;
											edge1.z = v0t1->position.z - other->vertices[v0].position.z;

											angle = -dot3(normalize3(edge0), normalize3(edge1));

											//if((angle <= 1.0 + FLT_EPSILON) && (angle >= 1.0 - FLT_EPSILON))
											//if(angle > 1.0 - FLT_EPSILON)
											//if(angle >= 0.9999)

											edge0 = normalize3(edge0);
											edge1 = normalize3(edge1);

											if((edge0.x <= edge1.x + FLT_EPSILON * 2.0) && (edge0.x >= edge1.x - FLT_EPSILON * 2.0))
											{
                                                if((edge0.y <= edge1.y + FLT_EPSILON * 2.0) && (edge0.y >= edge1.y - FLT_EPSILON * 2.0))
												{
													if((edge0.z <= edge1.z + FLT_EPSILON * 2.0) && (edge0.z >= edge1.z - FLT_EPSILON * 2.0))
													{
											//{
														prev = other->prev;
														other->prev->next = other->next;

														if(other->next)
														{
															other->next->prev = other->prev;
														}

														//triangle->vertices[(i + 1) % 3] = other->vertices[(j + 2) % 3];

														*v1t0 = other->vertices[(j + 2) % 3];

														memory_Free(other);

														other = prev;

														removed_triangles++;

														leaf->triangle_count--;

														goto _next_triangle;
													}
												}
											}
											//}





										}
									}

								}
							}
						}

						_next_triangle:

						other = other->next;
					}

				}while(removed_triangles);

				triangle = triangle->next;
			}


			*triangle_count += leaf->triangle_count;


		}
	}
	else
	{
        bsp_MergeCoplanarLeafTriangles(bsp->front, triangle_count);
		bsp_MergeCoplanarLeafTriangles(bsp->back, triangle_count);
	}

	depth--;

	#endif


}


void bsp_TriangulateLeafPolygons(bsp_node_t *node, int *triangle_count)
{

	bsp_polygon_t *polygon = NULL;
	bsp_triangle_t *triangles = NULL;
	bsp_triangle_t *triangle = NULL;
	bsp_leaf_t *leaf;

	vertex_t *vertices = NULL;
	int vertice_count = 0;
	int tris_count = 0;

	int i;


	static int level = -1;

	level++;

	if(!level)
	{
		*triangle_count = 0;
	}



	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;

		if(!(leaf->bm_flags & BSP_SOLID))
		{

			polygon = leaf->polygons;
			triangles = NULL;


			while(polygon)
			{
				bsp_TriangulatePolygon(polygon, &vertices, &vertice_count);

				for(i = 0; i < vertice_count;)
				{
					#ifndef USE_MEMORY_MALLOC
					triangle = malloc(sizeof(bsp_triangle_t));
					#else
					triangle = memory_Calloc(1, sizeof(bsp_triangle_t));
					#endif

					triangle->vertices[0] = vertices[i];
					i++;
					triangle->vertices[1] = vertices[i];
					i++;
					triangle->vertices[2] = vertices[i];
					i++;

					triangle->material_index = polygon->material_index;

					triangle->normal = polygon->normal;

					if(triangles)
					{
						triangles->prev = triangle;
					}

					triangle->next = triangles;
					triangles = triangle;

					tris_count++;

				}
				polygon = polygon->next;
			}


			leaf->triangles = triangles;
			leaf->triangle_count = tris_count;

			*triangle_count += tris_count;


		}
	}
	else
	{
		bsp_TriangulateLeafPolygons(node->front, triangle_count);
		bsp_TriangulateLeafPolygons(node->back, triangle_count);
	}

	level--;


}


/*
==============
bsp_BuildTriangleGroups
==============
*/
void bsp_BuildTriangleGroups(bsp_node_t *root, struct batch_t **groups, int *count)
{
	struct batch_t *g = NULL;
	struct batch_t *n = NULL;
	bsp_triangle_t *triangle = NULL;

	bsp_leaf_t *leaf = NULL;
	int i;
	int c;
	int k;

	if(!root)
		return;

	if(root->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)root;

		if(!(leaf->bm_flags & BSP_SOLID))
		{
			c = *count;
			g = *groups;

			triangle = leaf->triangles;

			/* go over the triangles of this leaf... */
			//for(i = 0; i < leaf->triangle_count; i++)
			while(triangle)
			{
				/* test if the material of this triangle has been added to the list... */
				for(k = 0; k < c; k++)
				{
					//printf("%d %d\n", triangle->material_index, g[k].material_index);
					//if(leaf->triangles[i].material_index == g[k].material_index)
					if(triangle->material_index == g[k].material_index)
					{
						break;	/* this material has been added to the list... */
					}
				}

				/* this material hasn't been added to the list... */
				if(k >= c)
				{
					#ifndef USE_MEMORY_MALLOC
					n = malloc(sizeof(batch_t) * (c + 1));
					#else
					n = memory_Malloc(sizeof(struct batch_t) * (c + 1));
					#endif

					/* copy everything before it... */
					for(k = 0; k < c; k++)
					{
						n[k] = g[k];
					}

					/* set stuff... */
					//n[k].material_index = leaf->triangles[i].material_index;
					n[k].material_index = triangle->material_index;
					n[k].next = 3;

					if(k)
					{
						/* this triangle group starts where the previous one ends... */
						n[k].start = n[k - 1].start + n[k - 1].next;
					}
					else
					{
						n[k].start = 0;
					}

					c++;

					if(g)
					{
						#ifndef USE_MEMORY_MALLOC
						free(g);
						#else
						memory_Free(g);
						#endif
					}

					g = n;
				}
				else
				{
					/* advance the next value... */
					g[k].next += 3;
					//g[k].triangle_count++;

					/* ... and update any triangle group that comes after this one... */
					for(k++; k < c; k++)
					{
						g[k].start = g[k - 1].start + g[k - 1].next;
					}
				}

				triangle = triangle->next;
			}

			*count = c;
			*groups = g;
		}
	}
	else
	{
		bsp_BuildTriangleGroups(root->front, groups, count);
		bsp_BuildTriangleGroups(root->back, groups, count);
	}
}


void bsp_RemoveExterior(bsp_node_t *bsp)
{

}


/*
==============
bsp_RecursiveLinearizeBsp
==============
*/

//#define LEAK_FOR_FUN

void bsp_RecursiveLinearizeBsp(bsp_node_t *bsp, vertex_t *vertices, int *vertex_count, struct bsp_pnode_t *lnodes, int *lnode_count, struct bsp_dleaf_t *lleaves, int *lleaves_count, struct batch_t *groups, int tri_group_count, int create_leaves, int pvs_size, int *debug_vertex_count)
{
	bsp_leaf_t *leaf;
	struct bsp_pnode_t *pnode;
	struct bsp_dleaf_t *dleaf;

	bsp_triangle_t *triangle;
	bsp_triangle_t *next_triangle;
	bsp_polygon_t *polygon;
	bsp_polygon_t *next_polygon;

	int i = 0;
	int k = 0;
	int tris_index;

	int node_index;
	int leaf_index;


	vec3_t center;
	float x_max = -9999999999999999.9;
	float y_max = -9999999999999999.9;
	float z_max = -9999999999999999.9;


	float x_min = 9999999999999999.9;
	float y_min = 9999999999999999.9;
	float z_min = 9999999999999999.9;


	if(!bsp)
		return;

	if(bsp->type == BSP_LEAF)
	{
		/* the extra node used to
		point to a empty leaf... */
		node_index = *lnode_count;
		pnode = lnodes + node_index;
		(*lnode_count)++;

		leaf = (bsp_leaf_t *)bsp;

		if(!(leaf->bm_flags & BSP_SOLID))
		{
			/* both child == 0 means this node indexes a leaf in the leaf array... */
			pnode->child[0] = BSP_EMPTY_LEAF;
			pnode->child[1] = BSP_EMPTY_LEAF;

			if(!create_leaves)
				return;

			leaf_index = *lleaves_count;
			(*lleaves_count)++;

			/* ... and the leaf index is stored in dist... */
			*(int *)&(pnode->dist) = leaf_index;


			dleaf = lleaves + leaf_index;



			dleaf->tris_count = leaf->triangle_count;
			//dleaf->tris_count = 0;

			#ifndef USE_MEMORY_MALLOC
			dleaf->tris = malloc(sizeof(bsp_striangle_t ) * dleaf->tris_count);
			#else
			dleaf->tris = memory_Malloc(sizeof(struct bsp_striangle_t ) * dleaf->tris_count);
			#endif
			//dleaf->pvs = leaf->pvs;

			#ifndef USE_MEMORY_MALLOC
			leaf->pvs = calloc(pvs_size, 1);
			#else
			leaf->pvs = memory_Calloc(pvs_size, 1);
			#endif


			dleaf->pvs = leaf->pvs;

			//leaf->pvs = dleaf->pvs;

			/* this is causing a crash... find out why... */
			/*for(i = 0; i < pvs_size; i++)
			{
				dleaf->pvs[i] = 0x0;
			}*/

			//dleaf->leaf_index = leaf->leaf_index;

			//return;

			triangle = leaf->triangles;
			tris_index = 0;
			while(triangle)
			{


				/* find to which triangle group this triangle belongs to... */
				for(i = 0; i < tri_group_count; i++)
				{
					if(triangle->material_index == groups[i].material_index)
					{
						break;
					}
				}

				k = groups[i].start;

				/* index to the first vertex of this triangle... */
				dleaf->tris[tris_index].first_vertex = k + groups[i].next;
				/* ...and which batch it belongs to... */
				dleaf->tris[tris_index].batch = i;

				tris_index++;

				/* copy this triangle's vertices to the appropriate region inside
				the triangle array... */
				vertices[k + groups[i].next] = triangle->vertices[0];
				if(triangle->vertices[0].position.x > x_max) x_max = triangle->vertices[0].position.x;
				if(triangle->vertices[0].position.y > y_max) y_max = triangle->vertices[0].position.y;
				if(triangle->vertices[0].position.z > z_max) z_max = triangle->vertices[0].position.z;

				if(triangle->vertices[0].position.x < x_min) x_min = triangle->vertices[0].position.x;
				if(triangle->vertices[0].position.y < y_min) y_min = triangle->vertices[0].position.y;
				if(triangle->vertices[0].position.z < z_min) z_min = triangle->vertices[0].position.z;
				groups[i].next++;

				vertices[k + groups[i].next] = triangle->vertices[1];
				if(triangle->vertices[1].position.x > x_max) x_max = triangle->vertices[1].position.x;
				if(triangle->vertices[1].position.y > y_max) y_max = triangle->vertices[1].position.y;
				if(triangle->vertices[1].position.z > z_max) z_max = triangle->vertices[1].position.z;

				if(triangle->vertices[1].position.x < x_min) x_min = triangle->vertices[1].position.x;
				if(triangle->vertices[1].position.y < y_min) y_min = triangle->vertices[1].position.y;
				if(triangle->vertices[1].position.z < z_min) z_min = triangle->vertices[1].position.z;
				groups[i].next++;

				vertices[k + groups[i].next] = triangle->vertices[2];
				if(triangle->vertices[2].position.x > x_max) x_max = triangle->vertices[2].position.x;
				if(triangle->vertices[2].position.y > y_max) y_max = triangle->vertices[2].position.y;
				if(triangle->vertices[2].position.z > z_max) z_max = triangle->vertices[2].position.z;

				if(triangle->vertices[2].position.x < x_min) x_min = triangle->vertices[2].position.x;
				if(triangle->vertices[2].position.y < y_min) y_min = triangle->vertices[2].position.y;
				if(triangle->vertices[2].position.z < z_min) z_min = triangle->vertices[2].position.z;
				groups[i].next++;

				*(debug_vertex_count) += 3;

				next_triangle = triangle->next;
				#ifndef USE_MEMORY_MALLOC
				free(triangle);
				#else
				memory_Free(triangle);
				#endif
				triangle = next_triangle;
			}

			/* the center of this leaf... */
			dleaf->center.x = (x_max + x_min) / 2.0;
			dleaf->center.y = (y_max + y_min) / 2.0;
			dleaf->center.z = (z_max + z_min) / 2.0;

			/* ...and it's axis aligned extents... */
			dleaf->extents.x = x_max - dleaf->center.x;
			dleaf->extents.y = y_max - dleaf->center.y;
			dleaf->extents.z = z_max - dleaf->center.z;
		}
		else
		{

			/* this node points to a solid leaf... */
			pnode->child[0] = BSP_SOLID_LEAF;
			pnode->child[1] = BSP_SOLID_LEAF;
		}

		#ifndef LEAK_FOR_FUN
		//free(leaf);
		#endif

	}
	else
	{
		/* this node might point to two other nodes,
		or might point to either a solid or empty
		leaf. In case of an solid leaf, the node
		will have both it's children == 0xffff,
		and won't index into the leaf array... */
		node_index = *lnode_count;
		pnode = lnodes + node_index;
		pnode->normal = bsp->normal;
		//pnode->point = bsp->point;
		pnode->dist = dot3(bsp->point, bsp->normal);
		(*lnode_count)++;




		/* relative displacement... */
		pnode->child[0] = (*lnode_count) - node_index;
		bsp_RecursiveLinearizeBsp(bsp->front, vertices, vertex_count, lnodes, lnode_count, lleaves, lleaves_count, groups, tri_group_count, create_leaves, pvs_size, debug_vertex_count);

		pnode->child[1] = (*lnode_count) - node_index;
		bsp_RecursiveLinearizeBsp(bsp->back, vertices, vertex_count, lnodes, lnode_count, lleaves, lleaves_count, groups, tri_group_count, create_leaves, pvs_size, debug_vertex_count);

		#ifndef LEAK_FOR_FUN
		//free(bsp);
		#endif

	}
}



void bsp_RecursiveAllocPvsForLeaves(bsp_node_t *bsp, int pvs_size)
{
	bsp_leaf_t *leaf;
	if(bsp->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)bsp;
		if(!(leaf->bm_flags & BSP_SOLID))
		{
			#ifndef USE_MEMORY_MALLOC
			leaf->pvs = calloc(pvs_size, 1);
			#else
			leaf->pvs = memory_Calloc(pvs_size, 1);
			#endif
		}
	}
	else
	{
		bsp_RecursiveAllocPvsForLeaves(bsp->front, pvs_size);
		bsp_RecursiveAllocPvsForLeaves(bsp->back, pvs_size);
	}
}



void bsp_AllocPvsForLeaves(bsp_node_t *bsp)
{
	int leaves = 0;
	int nodes = 0;
	int pvs_size = 0;

	if(bsp)
	{
		bsp_CountNodesAndLeaves(bsp, &leaves, &nodes);
		pvs_size = 4 + (leaves >> 3);
		bsp_RecursiveAllocPvsForLeaves(bsp, pvs_size);
	}
}


/*
==============
bsp_LinearizeBsp
==============
*/
void bsp_LinearizeBsp(bsp_node_t *bsp, vertex_t **vertices, int *vertex_count, struct bsp_pnode_t **lnodes, int *lnode_count, struct bsp_dleaf_t **lleaves, int *lleaves_count, struct batch_t *groups, int tri_group_count, int create_leaves)
{

	int i;
	int c = 0;
	int n = 0;
	int l = 0;

	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *polygon = NULL;
	struct bsp_pnode_t *nodes = NULL;
	struct bsp_dleaf_t *leaves = NULL;
	vertex_t *v;
	int debug_vertex_count = 0;

	int pvs_size = 0;

	if(vertex_count)
	{
		*vertex_count = groups[tri_group_count - 1].start + groups[tri_group_count - 1].next;
		#ifndef USE_MEMORY_MALLOC
		v = malloc(sizeof(vertex_t) * (*vertex_count));
		#else
		v = memory_Malloc(sizeof(vertex_t) * (*vertex_count));
		#endif
	}




	bsp_CountNodesAndLeaves(bsp, &l, &n);

	#ifndef USE_MEMORY_MALLOC
	nodes = malloc(sizeof(bsp_pnode_t) * n);
	#else
	nodes = memory_Malloc(sizeof(struct bsp_pnode_t) * n);
	#endif

	if(lleaves)
	{
		#ifndef USE_MEMORY_MALLOC
		leaves = malloc(sizeof(bsp_dleaf_t) * l);
		#else
		leaves = memory_Malloc(sizeof(struct bsp_dleaf_t) * l);
		#endif
		pvs_size = 4 + (l >> 3);
	}


	i = 0;
	n = 0;
	l = 0;
	c = 0;

	for(i = 0; i < tri_group_count; i++)
	{
		groups[i].next = 0;
	}

	bsp_RecursiveLinearizeBsp(bsp, v, &i, nodes, &n, leaves, &l, groups, tri_group_count, create_leaves, pvs_size, &debug_vertex_count);



	//visited_leaves = malloc(sizeof(bsp_dleaf_t *) * l);

//	printf("%d %d\n", l, n);

	if(vertices)
		*vertices = v;

	*lnodes = nodes;
	*lnode_count = n;

	if(lleaves)
	{
		*lleaves = leaves;
		*lleaves_count = l;
	}

}


bsp_edge_t *bsp_BuildBevelEdges(bsp_polygon_t *brush_polygons)
{
	int i;
	int c;

	int j;
	int k;

	int edge_count = 0;

	vec3_t p0;
	vec3_t p1;

	vec3_t r0;
	vec3_t r1;

	bsp_polygon_t *polygon_list = brush_polygons;
	bsp_polygon_t *p;
	bsp_polygon_t *r;

	bsp_edge_t *edge_list = NULL;
	bsp_edge_t *edge;
	bsp_edge_t *check_edge;

	//polygon_list = bsp_BuildPolygonsFromBrush(brush);


	p = polygon_list;


	while(p)
	{

		c = p->vert_count;

		r = polygon_list;

		while(r)
		{
			if(r == p)
			{
				r = r->next;
				continue;
			}

			for(i = 0; i < c; i++)
			{
				p0 = p->vertices[i].position;
				p1 = p->vertices[(i + 1) % c].position;

				k = r->vert_count;

				for(j = 0; j < k; j++)
				{

					r0 = r->vertices[j].position;
					r1 = r->vertices[(j + 1) % k].position;

					if(((p0.x == r0.x && p0.y == r0.y && p0.z == r0.z) && (p1.x == r1.x && p1.y == r1.y && p1.z == r1.z)) ||
					   ((p0.x == r1.x && p0.y == r1.y && p0.z == r1.z) && (p1.x == r0.x && p1.y == r0.y && p1.z == r0.z)) )
					{

						check_edge = edge_list;

						while(check_edge)
						{

							if((check_edge->polygon0 == p && check_edge->polygon1 == r) ||
							   (check_edge->polygon1 == p && check_edge->polygon0 == r ))
							{
								/* There's already an edge that link those two polygons... */
								break;
							}
							check_edge = check_edge->next;
						}

						/* this isn't a duplicate edge, so
						add it to the list... */
						if(!check_edge)
						{

							#if 0
							if((fabs(p->normal.x) == 1.0 && p->normal.y == 0.0 && p->normal.z == 0.0) ||
							   (p->normal.x == 0.0 && fabs(p->normal.y) == 1.0 && p->normal.z == 0.0) ||
							   (p->normal.x == 0.0 && p->normal.y == 0.0 && fabs(p->normal.z) == 1.0))
							{
								/* this polygon is aligned to one of the axial planes... */

								if(dot3(p->normal, r->normal) >= 0.0)
									continue;	/* this polygon already acts as the beveling plane... */
							}

							else if((fabs(r->normal.x) == 1.0 && r->normal.y == 0.0 && r->normal.z == 0.0) ||
							   		(r->normal.x == 0.0 && fabs(r->normal.y) == 1.0 && r->normal.z == 0.0) ||
							   		(r->normal.x == 0.0 && r->normal.y == 0.0 && fabs(r->normal.z) == 1.0))
							{
								/* this polygon is aligned to one of the axial planes... */

								if(dot3(p->normal, r->normal) >= 0.0)
									continue;	/* this polygon already acts as the beveling plane... */
							}

							#endif

							#ifndef USE_MEMORY_MALLOC
							edge = malloc(sizeof(bsp_edge_t) );
							#else
							edge = memory_Malloc(sizeof(bsp_edge_t));
							#endif
							edge->v0 = p0;
							edge->v1 = p1;
							edge->v0_p0 = i;
							edge->v0_p1 = (i + 1) % c;
							edge->v1_p0 = j;
							edge->v1_p1 = (j + 1) % k;

							edge->polygon0 = p;
							edge->polygon1 = r;

							edge->dot = dot3(p->normal, r->normal);

							edge->next = edge_list;
							edge_list = edge;

							edge_count++;
						}

					}
				}
			}

			r = r->next;
		}

		p = p->next;
	}


	return edge_list;

}


void bsp_ExpandBrushes(vec3_t box_extents)
{
	#if 0
	int i;
	int j;
	int c;
	int vert_count;
	float d;
	bsp_edge_t *edges;
	bsp_edge_t *edge;
	bsp_edge_t *prev;
	bsp_polygon_t *polygons;
	bsp_polygon_t *polygon;
	bsp_polygon_t *adj;
	bsp_polygon_t *p0;
	bsp_polygon_t *p1;

	vec3_t polygon_normal;
	vec3_t v;
	vec3_t polygon_center;
	vec3_t r;

	short adj_v0;
	short adj_v1;


	if(expanded_brushes)
	{
		for(i = 0; i < expanded_brush_count; i++)
		{
			polygons = expanded_brushes[i].base_polygons;

			while(polygons)
			{
				polygon = polygons->next;
				#ifndef USE_MEMORY_MALLOC
				free(polygons->vertices);
				free(polygons);
				#else
				memory_Free(polygons->vertices);
				memory_Free(polygons);
				#endif
				polygons = polygon;
			}

		}

		#ifndef USE_MEMORY_MALLOC
		free(expanded_brushes);
		#else
		memory_Free(expanded_brushes);
		#endif

	}


	#ifndef USE_MEMORY_MALLOC
	expanded_brushes = malloc(sizeof(brush_t) * brush_list_size);
	#else
	expanded_brushes = memory_Malloc(sizeof(brush_t) * brush_list_size, "bsp_ExpandBrushes");
	#endif


	for(i = 0, c = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;


		polygons = bsp_DeepCopyPolygons(brushes[i].base_polygons);

		if(!polygons)
			continue;

		edges = bsp_BuildBevelEdges(polygons);

		expanded_brushes[c] = brushes[i];
		expanded_brushes[c].base_polygons = polygons;
		expanded_brushes[c].type = BRUSH_CUBE;

		polygon = polygons;

		while(polygon)
		{
			polygon->b_used = 0;
			vert_count = polygon->vert_count;
			polygon_normal = polygon->normal;

			v.x = fabs(polygon_normal.x);
			v.y = fabs(polygon_normal.y);
			v.z = fabs(polygon_normal.z);

			d = fabs(dot3(box_extents, v));

			for(j = 0; j < vert_count; j++)
			{
				polygon->vertices[j].position.x += polygon_normal.x * d;
				polygon->vertices[j].position.y += polygon_normal.y * d;
				polygon->vertices[j].position.z += polygon_normal.z * d;
			}

			edge = edges;

			/* go over the edges... */
			while(edge)
			{

				if(edge->polygon0 == polygon)
				{
					adj = edge->polygon1;
					adj_v0 = edge->v1_p0;
					adj_v1 = edge->v1_p1;
				}
				else if(edge->polygon1 == polygon)
				{
					adj = edge->polygon0;
					adj_v0 = edge->v0_p0;
					adj_v1 = edge->v0_p1;
				}
				else
				{
					edge = edge->next;
					continue;
				}

				/* ... and move the vertices of any polygon that share
				an edge with the polygon we just moved...*/
				adj->vertices[adj_v0].position.x += polygon_normal.x * d;
				adj->vertices[adj_v0].position.y += polygon_normal.y * d;
				adj->vertices[adj_v0].position.z += polygon_normal.z * d;

				adj->vertices[adj_v1].position.x += polygon_normal.x * d;
				adj->vertices[adj_v1].position.y += polygon_normal.y * d;
				adj->vertices[adj_v1].position.z += polygon_normal.z * d;

				edge = edge->next;
			}



			polygon = polygon->next;
		}

		#if 0

		edge = edges;
		prev = NULL;
		while(edge)
		{

			p0 = edge->polygon0;
			p1 = edge->polygon1;

			if((fabs(p0->normal.x) == 1.0 && p0->normal.y == 0.0 && p0->normal.z == 0.0) ||
			   (p0->normal.x == 0.0 && fabs(p0->normal.y) == 1.0 && p0->normal.z == 0.0) ||
			   (p0->normal.x == 0.0 && p0->normal.y == 0.0 && fabs(p0->normal.z) == 1.0))
			{
				/* this polygon is aligned to one of the axial planes... */

				if(dot3(p0->normal, p1->normal) >= 0.0)
				{
					if(prev)
					{
						prev->next = edge->next;
						free(edge);
						edge = prev->next;
						continue;
					}
					else
					{
						prev = edge->next;
						free(edge);
						edge = prev;
						edges = prev;
						prev = NULL;
						continue;
					}
				}

			}

			else if((fabs(p1->normal.x) == 1.0 && p1->normal.y == 0.0 && p1->normal.z == 0.0) ||
			   		(p1->normal.x == 0.0 && fabs(p1->normal.y) == 1.0 && p1->normal.z == 0.0) ||
			   		(p1->normal.x == 0.0 && p1->normal.y == 0.0 && fabs(p1->normal.z) == 1.0))
			{
				/* this polygon is aligned to one of the axial planes... */

				if(dot3(p0->normal, p1->normal) >= 0.0)
				{
					if(prev)
					{
						prev->next = edge->next;
						free(edge);
						edge = prev->next;
						continue;
					}
					else
					{
						prev = edge->next;
						free(edge);
						edge = prev;
						prev = NULL;
						continue;
					}
				}

			}

			prev = edge;
			edge = edge->next;
		}

		#endif


		/*#ifdef DRAW_BEVEL_EDGES

		if(prev)
		{
			prev->next = bevel_edges;
			bevel_edges = edges;
		}

		#else*/

		while(edges)
		{
			edge = edges->next;
			#ifndef USE_MEMORY_MALLOC
			free(edges);
			#else
			memory_Free(edges);
			#endif
			edges = edge;
		}

		//#endif

		c++;


	}

	expanded_brush_count = c;

	#endif

}


void bsp_LockBrushPolygons()
{
	if(!brush_polygons_lock)
	{
		brush_polygons_lock = SDL_CreateMutex();
	}

	SDL_LockMutex(brush_polygons_lock);
}

void bsp_UnlockBrushPolygons()
{
	SDL_UnlockMutex(brush_polygons_lock);
}


/*
==============
bsp_BuildBsp
==============
*/
void bsp_CompileBsp(int remove_outside)
{
	int i;
	int c;
	int j;
//	int k = brush_count;
	int triangle_group_count = 0;
	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *r;
	bsp_polygon_t *s;
	bsp_node_t *bsp;

	//_HEAPINFO heap_info;


	if(b_compiling || b_calculating_pvs)
	{
		printf("bsp_CompileBsp: busy!\n");
		return;
	}

	b_compiling = 1;

	//world_Clear(WORLD_CLEAR_FLAG_PHYSICS_MESH | WORLD_CLEAR_FLAG_BSP);

	if(world_bsp)
	{
		bsp_DeleteSolidLeafBsp(world_bsp);
		world_bsp = NULL;
	}

	//bsp_triangle_t *triangles;



	//bsp_BuildCollisionBsp();

	//bsp_LockBrushPolygons();

    //memory_CheckCorrupted();

	polygons = brush_BuildPolygonListFromBrushes();

	//memory_CheckCorrupted();

	//bsp_UnlockBrushPolygons();

	if(!polygons)
	{
		b_compiling = 0;
		printf("bsp_CompileBsp: !polygons\n");
		return;
	}


	printf("bsp_CompileBsp: bsp_SolidLeafBsp... ");
	world_bsp = bsp_SolidLeafBsp(polygons);
	printf("done\n");

	//memory_CheckCorrupted();

	//printf("bsp_CompileBsp: bsp_ClipCoplanarLeafPolygons... ");
	//bsp_ClipCoplanarLeafPolygons(world_bsp);
	//printf("done\n");

	//memory_CheckCorrupted();


	printf("bsp_CompileBsp: bsp_TriangulateLeafPolygons... ");
	bsp_TriangulateLeafPolygons(world_bsp, &w_world_triangle_count);
	printf("done\n");

    //memory_CheckCorrupted();

	//printf("bsp_CompileBsp: bsp_MergeCoplanarLeafTriangles... ");
	//bsp_MergeCoplanarLeafTriangles(world_bsp, &w_world_triangle_count);
	//printf("done\n");

	//memory_CheckCorrupted();


	printf("bsp_CompileBsp: bsp_BuildTriangleGroups... ");
	bsp_BuildTriangleGroups(world_bsp, &w_world_batches, &w_world_batch_count);
	printf("done\n");

	//memory_CheckCorrupted();


	printf("bsp_CompileBsp: bsp_LinearizeBsp... ");
	bsp_LinearizeBsp(world_bsp, &w_world_vertices, &w_world_vertices_count, &w_world_nodes, &w_world_nodes_count, &w_world_leaves, &w_world_leaves_count, w_world_batches, w_world_batch_count, 1);
	printf("done\n");

	//memory_CheckCorrupted();

/*
	struct bsp_vis_sample_t *samples;
	struct bsp_vis_sample_t *sample;
	struct trace_t result;

	struct bsp_dleaf_t *leaf;

	vec3_t max = vec3_t_c(0.0, 0.0, 0.0);
	vec3_t min = vec3_t_c(0.0, 0.0, 0.0);
	vec3_t range;
	vec3_t pos;

    int leaf_index;

	int x;
	int y;
	int z;

	int rx;
	int ry;

	int cx;
	int cy;
	int cz;


	vec3_t start;
	vec3_t end;

	vec3_t dir;

	//bsp_BspBounds(world_bsp, &max, &min);

	//samples = memory_Calloc(1000 * 1000 * 1000, sizeof(struct bsp_vis_sample_t));

	#define BSP_VIS_SAMPLE_DENSITY 2
    #define BSP_VIS_RESOLUTION 4

    printf("bsp_ClassifyViewSamples...");

    for(leaf_index = 0; leaf_index < w_world_leaves_count; leaf_index++)
    {
        leaf = w_world_leaves + leaf_index;

        range.x = leaf->extents.x * 2.0;
        range.y = leaf->extents.y * 2.0;
        range.z = leaf->extents.z * 2.0;

        pos.x = leaf->center.x - leaf->extents.x;
        pos.y = leaf->center.y - leaf->extents.y;
        pos.z = leaf->center.z - leaf->extents.z;

        cx = 1 + range.x * BSP_VIS_SAMPLE_DENSITY;
        cy = 1 + range.y * BSP_VIS_SAMPLE_DENSITY;
        cz = 1 + range.z * BSP_VIS_SAMPLE_DENSITY;

        samples = memory_Calloc(cx * cy * cz, sizeof(struct bsp_vis_sample_t));

        for(z = 0; z < cz; z++)
        {
            for(y = 0; y < cy; y++)
            {
                for(x = 0; x < cx; x++)
                {
                    sample = samples + (z * cy * cx) + (y * cx) + x;

                    sample->position.x = pos.x + range.x * ((float)x / (float)cx);
                    sample->position.y = pos.y + range.y * ((float)y / (float)cy);
                    sample->position.z = pos.z + range.z * ((float)z / (float)cz);

                    sample->valid = (bsp_GetCurrentLeaf(w_world_nodes, sample->position) == leaf);
                }
            }
        }

        for(z = 0; z < cz; z++)
        {
            for(y = 0; y < cy; y++)
            {
                for(x = 0; x < cx; x++)
                {
                    sample = samples + (z * cy * cx) + (y * cx) + x;

                    if(sample->valid)
                    {
                        for(ry = 0; ry < BSP_VIS_RESOLUTION; ry++)
                        {
                            for(rx = 0; rx < BSP_VIS_RESOLUTION; rx++)
                            {
                                dir.x = -1.0 + 2.0 * ((float)rx / (float)BSP_VIS_RESOLUTION);
                                dir.y = -1.0 + 2.0 * ((float)ry / (float)BSP_VIS_RESOLUTION);
                                dir.z = -1.0;

                                dir = normalize3(dir);

                                start = sample->position;

                                end.x = sample->position.x + dir.x * 1000.0;
                                end.y = sample->position.y + dir.y * 1000.0;
                                end.z = sample->position.z + dir.z * 1000.0;

                                bsp_FirstHit(w_world_nodes, start, end, &result);
                            }
                        }
                    }
                }
            }
        }

        printf("%.03f%%\n", 100.0 * ((float)leaf_index / (float)w_world_leaves_count));

        memory_Free(samples);
    }

    printf("done\n");
*/



	//bsp_AllocPvsForLeaves(world_bsp);

	//printf("before world_Update...\n");
	//log_LogMessage(LOG_MESSAGE_NOTIFY, "before world_Update...");
	//memory_CheckCorrupted();

	world_Update();

	//memory_CheckCorrupted();

	//printf("after world_Update...\n");
	//log_LogMessage(LOG_MESSAGE_NOTIFY, "after world_Update...");
	//memory_CheckCorrupted();

	//memory_CheckCorrupted();


/*	if(!step_semaphore)
	{
		step_semaphore = SDL_CreateSemaphore(0);
		polygon_copy_mutex = SDL_CreateMutex();
	}
	*/

	//bsp_build_thread = SDL_CreateThread(bsp_CalculatePvsAssync, "calculate pvs thread", world_bsp);
	//SDL_DetachThread(bsp_build_thread);

	b_compiling = 0;



	//bsp_DeleteSolidLeafBsp(bsp);




}

#endif
















