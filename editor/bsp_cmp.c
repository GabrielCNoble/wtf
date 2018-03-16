#include <stdio.h>
#include <stdlib.h>
#include "bsp.h"
#include "bsp_cmp.h"
#include "pvs.h"
#include "world.h"
#include "camera.h"
#include "player.h"
#include "engine.h"
 
#include <float.h>

#include <fenv.h>

#include "SDL2\SDL.h"
#include "GL\glew.h"



/* from brush.c */
extern int brush_list_size;
extern int brush_count;
extern brush_t *brushes;
extern int expanded_brush_count;
extern brush_t *expanded_brushes;

bsp_polygon_t *expanded_polygons = NULL;

/* from light.c */
extern int visible_light_count;

/* from brush.c */
extern vec3_t cube_bmodel_collision_verts[];
extern vec3_t cube_bmodel_collision_normals[];

/* from world.c */
bsp_node_t *world_bsp = NULL;
bsp_node_t *collision_bsp = NULL;
extern int world_vertices_count;
extern vertex_t *world_vertices;
extern int world_triangle_group_count;
extern triangle_group_t *world_triangle_groups;
extern int world_nodes_count;
extern bsp_pnode_t *world_nodes;
extern int collision_nodes_count;
extern bsp_pnode_t *collision_nodes;
//extern bsp_polygon_t *node_polygons;			/* necessary to quickly build portals... */
extern int world_leaves_count;
extern bsp_dleaf_t *world_leaves;
extern int visited_leaves_count;
extern bsp_dleaf_t *visited_leaves;

bsp_polygon_t *node_polygons = NULL;			/* necessary to quickly build portals... */

static SDL_Thread *bsp_build_thread;

SDL_mutex *polygon_copy_mutex = NULL;
SDL_sem *step_semaphore = NULL;
SDL_mutex *stop_flags_mutex = NULL;




bsp_polygon_t *beveled_polygons = NULL;
bsp_edge_t *bevel_edges = NULL;

bsp_polygon_t *world_polygons_debug = NULL;

int b_compiling = 0;
//int b_stop = 0;

extern int b_calculating_pvs;


#define BSP_POOL_SIZE 50000

int node_pool_cursor = 0;
bsp_node_t *node_pool = NULL;

int leaf_pool_cursor = 0;
bsp_leaf_t *leaf_pool = NULL;


#define DRAW_EXPANDED_BRUSHES
//#define DRAW_BEVEL_EDGES
#define DRAW_WORLD_POLYGONS


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
	//r = d;
	
	/**(int *)&r &= ~0x80000000;*/
	
	//printf("%f\n", d);
	if(d > FUZZY_ZERO)
	{
		return POINT_FRONT;
	}
	else if(d < -FUZZY_ZERO)
	{
		return POINT_BACK;
	}
	
	return POINT_CONTAINED;
	/*else if(r <= FUZZY_ZERO)		
	{
		return POINT_CONTAINED;
	}
	else
	{
		return POINT_BACK;
	}*/
}

/*
==============
bsp_ClassifyTriangle
==============
*/
int bsp_ClassifyTriangle(bsp_triangle_t *triangle, vec3_t point, vec3_t normal)
{
	int a;
	int b;
	int c;
	
	int r = 0;
	
	a = bsp_ClassifyPoint(triangle->a.position, point, normal);
	b = bsp_ClassifyPoint(triangle->b.position, point, normal);
	c = bsp_ClassifyPoint(triangle->c.position, point, normal);
	
	vec3_t n;
	
	switch(a | b | c)
	{
		case POINT_FRONT:
		case POINT_FRONT | POINT_CONTAINED:
			return TRIANGLE_FRONT;
		break;
		
		case POINT_BACK:
		case POINT_BACK | POINT_CONTAINED:
			return TRIANGLE_BACK;
		break;
		
		case POINT_FRONT | POINT_BACK:
		case POINT_FRONT | POINT_BACK | POINT_CONTAINED:
			return TRIANGLE_STRADDLING;
		break;
		
		case POINT_CONTAINED:
			
			/*v.x = triangle->a.position.x - point.x;
			v.y = triangle->a.position.y - point.y;
			v.z = triangle->a.position.z - point.z;*/
			
			n.x = (triangle->a.normal.x + triangle->b.normal.x + triangle->c.normal.x) / 3.0;
			n.y = (triangle->a.normal.y + triangle->b.normal.y + triangle->c.normal.y) / 3.0;
			n.z = (triangle->a.normal.z + triangle->b.normal.z + triangle->c.normal.z) / 3.0;
			
			
			//n = normalize3(n);
			
			if(dot3(n, normal) >= 0.0)
			{
				return TRIANGLE_CONTAINED_FRONT;
			}
			else
			{
				return TRIANGLE_CONTAINED_BACK;
			}
		break;
		
	}

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
bsp_SplitTriangle
==============
*/
int bsp_SplitTriangle(bsp_triangle_t *triangle, vec3_t point, vec3_t normal, bsp_triangle_t **front, bsp_triangle_t **back)
{
	
	//printf("start of bsp_SplitTriangle\n");
	
	int i;
	int r;
	
	vertex_t tri_verts[3];
	//vec3_t normal;
	//vec3_t tangent;
	//vec2_t tex_coord;
	
	vec3_t clip_normal;
	vec3_t clip_tangent;
	vec2_t clip_tex_coord;
	
	vec3_t splitter_point;
	vec3_t splitter_normal;
	
	int clip_vertex_count = 0;
	vec3_t clip_vertex;
	
	int front_vertex_count = 0;
	vertex_t front_vertexes[6];
	
	int back_vertex_count = 0;
	vertex_t back_vertexes[6];
	
	int cur_side;
	
	float time;
	
	bsp_triangle_t *f;
	bsp_triangle_t *b;
	bsp_triangle_t *t;
	
	
	tri_verts[0] = triangle->a;
	tri_verts[1] = triangle->b;
	tri_verts[2] = triangle->c;
	
	int pa;
	int pb;
	int pc;
	
	
	
	pa = bsp_ClassifyPoint(tri_verts[0].position, point, normal);
	pb = bsp_ClassifyPoint(tri_verts[1].position, point, normal);
	pc = bsp_ClassifyPoint(tri_verts[2].position, point, normal);
	
	//printf("pa: %d    pb: %d    pc: %d\n", pa, pb, pc);
	
	switch(pa | pb | pc)
	{
		case POINT_FRONT:
		case POINT_CONTAINED | POINT_FRONT:
			
			//printf("triangle on front\n");
			
			_return_front_triangle:
			
			f = malloc(sizeof(bsp_triangle_t));
			*f = *triangle;
			*front = f;
			*back = NULL;
			return;
		break;
		
		
		case POINT_BACK:
		case POINT_CONTAINED | POINT_BACK:
			
			//printf("triangle on back\n");
			
			_return_back_triangle:
				
			f = malloc(sizeof(bsp_triangle_t));
			*f = *triangle;
			*front = NULL;
			*back = f;
			return;
		break;
		
		
		case POINT_CONTAINED:
			clip_vertex.x = triangle->a.position.x - point.x;
			clip_vertex.y = triangle->a.position.y - point.y;
			clip_vertex.z = triangle->a.position.z - point.z;
			
			//printf("triangle contained\n");
			
			if(dot3(clip_vertex, normal) >= 0.0)
			{
				goto _return_front_triangle;
			}
			else
			{
				goto _return_back_triangle;
			}
		break;
		
		case POINT_FRONT | POINT_BACK:
		case POINT_FRONT | POINT_BACK | POINT_CONTAINED:
		
			//printf("triangle straddling\n");
			
			if(pa == POINT_CONTAINED)
			{
				cur_side = pb;
			}
			else
			{
				cur_side = pa;
			}
						
			for(i = 0; i < 3; i++)
			{
				
				r = (i + 1) % 3;
				
				pa = bsp_ClassifyPoint(tri_verts[i].position, point, normal);
				pb = bsp_ClassifyPoint(tri_verts[r].position, point, normal);
				
				if((pa | pb) == (POINT_FRONT | POINT_BACK))
				{
					bsp_ClipEdge(tri_verts[i].position, tri_verts[r].position, point, normal, &clip_vertex, &time);
					
					//if(time < 0.0 || time > 1.0)
					//{
					//printf("%f\n", time);
					//}
					
				//	assert(time >= 0.0 && time <= 1.0);
					
					clip_normal.x = (1.0 - time) * tri_verts[i].normal.x + time * tri_verts[r].normal.x;
					clip_normal.y = (1.0 - time) * tri_verts[i].normal.y + time * tri_verts[r].normal.y;
					clip_normal.z = (1.0 - time) * tri_verts[i].normal.z + time * tri_verts[r].normal.z;
					
					
					clip_tangent.x = (1.0 - time) * tri_verts[i].tangent.x + time * tri_verts[r].tangent.x;
					clip_tangent.y = (1.0 - time) * tri_verts[i].tangent.y + time * tri_verts[r].tangent.y;
					clip_tangent.z = (1.0 - time) * tri_verts[i].tangent.z + time * tri_verts[r].tangent.z;
					
					clip_tex_coord.x = (1.0 - time) * tri_verts[i].tex_coord.x + time * tri_verts[r].tex_coord.x;
					clip_tex_coord.y = (1.0 - time) * tri_verts[i].tex_coord.y + time * tri_verts[r].tex_coord.y;
				
					
					if(cur_side == POINT_FRONT)
					{						
						cur_side = POINT_BACK;
					
						front_vertexes[front_vertex_count++] = tri_verts[i];
						front_vertexes[front_vertex_count].position = clip_vertex;
						front_vertexes[front_vertex_count].normal = clip_normal;
						front_vertexes[front_vertex_count].tangent = clip_tangent;
						front_vertexes[front_vertex_count++].tex_coord = clip_tex_coord;
						
						back_vertexes[back_vertex_count].position = clip_vertex;	
						back_vertexes[back_vertex_count].normal = clip_normal;
						back_vertexes[back_vertex_count].tangent = clip_tangent;
						back_vertexes[back_vertex_count++].tex_coord = clip_tex_coord;
					}
					else
					{
						cur_side = POINT_FRONT;
						
						back_vertexes[back_vertex_count++] = tri_verts[i];
						back_vertexes[back_vertex_count].position = clip_vertex;
						back_vertexes[back_vertex_count].normal = clip_normal;
						back_vertexes[back_vertex_count].tangent = clip_tangent;
						back_vertexes[back_vertex_count++].tex_coord = clip_tex_coord;
						
						front_vertexes[front_vertex_count].position = clip_vertex;
						front_vertexes[front_vertex_count].normal = clip_normal;
						front_vertexes[front_vertex_count].tangent = clip_tangent;
						front_vertexes[front_vertex_count++].tex_coord = clip_tex_coord;
					}
					
					clip_vertex_count++;
				}
				else
				{					
					if(cur_side == POINT_FRONT)
					{
						front_vertexes[front_vertex_count++] = tri_verts[i];
						
						if(pa == POINT_CONTAINED)
						{
							back_vertexes[back_vertex_count++] = tri_verts[i];
							cur_side = pb;
						}
					}
					else
					{
						back_vertexes[back_vertex_count++] = tri_verts[i];
						
						if(pa == POINT_CONTAINED)
						{
							front_vertexes[front_vertex_count++] = tri_verts[i];
							cur_side = pb;
						}
					}
					
				}
				
			}
			
		break;
		
	}
	
	//printf("front: %d   back: %d\n", front_vertex_count, back_vertex_count);
	
	assert(front_vertex_count >= 3);
	assert(back_vertex_count >= 3);
	
	
	f = malloc(sizeof(bsp_triangle_t));
	b = malloc(sizeof(bsp_triangle_t));
	
	f->a = front_vertexes[0];
	f->b = front_vertexes[1];
	f->c = front_vertexes[2];
	//f->plane_normal = triangle->plane_normal;
	//f->b_used = triangle->b_used;
	f->material_index = triangle->material_index;
	f->next = NULL;
	
	b->a = back_vertexes[0];
	b->b = back_vertexes[1];
	b->c = back_vertexes[2];
	//b->plane_normal = triangle->plane_normal;
	//b->b_used = triangle->b_used;
	b->material_index = triangle->material_index;
	b->next = NULL;
	
	/* this triangle had two of it's edges clipped,
	so one of the sides has a quad that need to 
	be triangulated... */
	if(front_vertex_count != back_vertex_count)
	{
		t = malloc(sizeof(bsp_triangle_t));
		
		/* front side has four vertices... */
		if(front_vertex_count > back_vertex_count)
		{
			t->a = front_vertexes[2];
			t->b = front_vertexes[3];
			t->c = front_vertexes[0];	
			f->next = t;
		}
		else
		{
			t->a = back_vertexes[2];
			t->b = back_vertexes[3];
			t->c = back_vertexes[0];
			b->next = t;
		}
		
		
		t->next = NULL;
		t->material_index = triangle->material_index;
		//t->b_used = triangle->b_used;
		//t->plane_normal = triangle->plane_normal;
	}
	
	//printf("%x %x\n", f, b);	
	
	*front = f;
	*back = b;
	
	//printf("end of bsp_SplitTriangle\n\n");
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
	assert(back_vertex_count > 2);
	assert(front_vertex_count > 2);
	
	//printf("front: %d   back: %d\n", front_vertex_count, back_vertex_count);
	
	//assert(clip_vertex_count > 0);
	
	f = malloc(sizeof(bsp_polygon_t ));
	f->normal = polygon->normal;
	f->vert_count = front_vertex_count;
	f->vertices = malloc(sizeof(vertex_t) * (f->vert_count));
	f->b_used = polygon->b_used;
	f->next = NULL;
	f->material_index = polygon->material_index;
	f->triangle_group = polygon->triangle_group;
	f->brush_index = polygon->brush_index;
	
	for(i = 0; i < f->vert_count; i++)
	{
		f->vertices[i] = front_vertexes[i];
	}
	
	
	b = malloc(sizeof(bsp_polygon_t ));
	b->normal = polygon->normal;
	b->vert_count = back_vertex_count;
	b->vertices = malloc(sizeof(vertex_t) * (b->vert_count));
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


int bsp_TrimPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal)
{

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
		free(polygon->vertices);
		polygon->vertices = malloc(sizeof(vertex_t) * front_vertex_count);
		polygon->vert_count = front_vertex_count;
	}
	
	for(i = 0; i < front_vertex_count; i++)
	{
		polygon->vertices[i] = front_vertexes[i];
	}
	
	return 0;
	
}


bsp_polygon_t *bsp_DeepCopyPolygon(bsp_polygon_t *src)
{
	int i;
	bsp_polygon_t *p = NULL;
	
	if(src)
	{
		p = malloc(sizeof(bsp_polygon_t) * 10);
		p->normal = src->normal;
		p->brush_index = src->brush_index;
		p->b_used = src->b_used;
		//p->next = src->next;
		p->next = NULL;
		p->vert_count = src->vert_count;
		p->vertices = malloc(sizeof(vertex_t) * p->vert_count * 10);
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
		r = malloc(sizeof(bsp_polygon_t ) * 10);
		
		r->brush_index = src->brush_index;
		r->b_used = src->b_used;
		r->normal = src->normal;
		r->vert_count = src->vert_count;
		r->vertices = malloc(sizeof(vertex_t) * r->vert_count * 10);
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
	
	out = malloc(sizeof(bsp_polygon_t ) * polygon_count);
	vertices = malloc(sizeof(vertex_t) * vert_count);
	
	i = 0;
	
	r = src;
	while(r)
	{
		out[i].b_used = r->b_used;
		out[i].material_index = r->material_index;
		out[i].normal = r->normal;
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
		
		i++;
		r = r->next;
	}
	
	for(i = 0; i < polygon_count; i++)
	{
		out[i].next = out + i + 1;
	}
	
	out[i - 1].next = NULL;
	
	return out;
}


void bsp_DeletePolygons(bsp_polygon_t *polygons)
{
	bsp_polygon_t *next;
	
	while(polygons)
	{
		next = polygons->next;
		free(polygons->vertices);
		free(polygons);
		polygons = next;
	}	
}

void bsp_DeletePolygonsContiguous(bsp_polygon_t *polygons)
{
	if(polygons)
	{
		free(polygons->vertices);
		free(polygons);
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
	vec3_t fn;
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
	
	fn = cross(s, t);
	
	for(i = 2; i < c; i++)
	{
		b = in_verts[i].position;
		
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
			
			if(dot3(h, fn) < 0.0)
			{
				p--;
			}
			else
			{
				p++;
			}
		}
		p = abs(p);
		if(p < best)
		{
			best = p;
			bi = i;
		}
		
	}
	
	k = in_vert_count >> 1;
	vin = malloc(sizeof(vertex_t) * in_vert_count * 10);
	
	if(out_indexes)
	{
		iin = malloc(sizeof(int) * in_vert_count * 10);
		
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

	free(vin);
	
	//if(out_indexes)
	//	free(iin);
	
	return 1;
}


void bsp_TriangulatePolygon(bsp_polygon_t *polygon, vertex_t **vertices, int *vertex_count)
{
	
	int count = 0;
	vertex_t *verts = NULL;
	
	if(polygon->vert_count >= 3)
	{
		verts = malloc(sizeof(vertex_t ) * (polygon->vert_count - 2) * 3 * 10);
		
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
		out_indexes = malloc(sizeof(int ) * out_index_count * 10);
		
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
		
		verts = malloc(sizeof(vertex_t ) * (polygon->vert_count - 2) * 3 * 10);
		in_indexes = malloc(sizeof(int) * polygon->vert_count * 10);
		
		c = polygon->vert_count;
		
		for(i = 0; i < c; i++)
		{
			in_indexes[i] = i;
		}
				
		bsp_RecursiveTriangulate(polygon->vertices, polygon->vert_count, verts, &count, in_indexes, out_indexes);
		
		/* welp, fuck this... */
		free(verts);
		
		free(in_indexes);
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
		out_indexes = malloc(sizeof(int) * count * 2);
	}
	else
	{
		out_indexes = *indexes;
	}
	
	
	in_indexes = malloc(sizeof(int) * max_verts * 3);
	verts = malloc(sizeof(vertex_t) * (max_verts - 2) * 3);
	
	polygon = polygons;
	
	count = 0;
	r = 0;
	
	while(polygon)
	{		
		/* avoid the whole thing if the polygon is a triangle... */
		if(polygon->vert_count == 3)
		{
			for(i = 0; i < 3; i++)
			{
				out_indexes[i + cursor] = i + cursor;
			}
		
			cursor += 3;
		}
		else
		{
			c = polygon->vert_count;
		
			for(i = 0; i < c; i++)
			{
				in_indexes[i] = i;
			}
			
			count = 0;
					
			bsp_RecursiveTriangulate(polygon->vertices, polygon->vert_count, verts, &count, in_indexes, out_indexes + cursor);
			
			//c = count;
			
			
			for(i = 0; i < count; i++)
			{
				out_indexes[i + cursor] += r;
			}
			r += c;
			
			cursor += count;
		}
		
		polygon = polygon->next;
	}
	
	free(verts);
	free(in_indexes);
	
	*indexes = out_indexes;
	*index_count = cursor;
	
	return;
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
	/*else if(unlink_splitter)
	{
		if(min_splitter)
		{
			
			if(prev_min_splitter)
			{
				prev_min_splitter->next = min_splitter->next;
			}
			else
			{
				*polygons = (*polygons)->next;
			}
			
			min_splitter->next = NULL;
			
			cur_splitter = *polygons;
			prev_splitter = NULL;
			last_splitter = min_splitter;
			
			
			while(cur_splitter)
			{
				
				i = bsp_ClassifyPolygon(cur_splitter, min_splitter->vertices[0].position, min_splitter->normal);
					
				if(i == POLYGON_CONTAINED_FRONT || i == POLYGON_CONTAINED_BACK)
				{					
					last_splitter->next = cur_splitter;
					last_splitter = cur_splitter;
					
					if(prev_splitter)
					{
						prev_splitter->next = cur_splitter->next;
						cur_splitter = cur_splitter->next;
					}
					else
					{
						*polygons = (*polygons)->next;
						cur_splitter = *polygons;
					}
					
					last_splitter->next = NULL;
					continue;
				}
				
				prev_splitter = cur_splitter;
				cur_splitter = cur_splitter->next;
				
			}
		}
	}*/
	
	//min_splitter->b_used = 1;
	
	return min_splitter;
}



#if 0

bsp_polygon_t *bsp_BuildPolygonsFromBrush(brush_t *brush)
{
	int i;
	int c;
	
	int j;
	int k;
	
	int r;
	int l;
	
	int n;
	int m;
	
	bsp_polygon_t *p = NULL;
	bsp_polygon_t *polygons = NULL;
	
	//int vertex_stack_top = -1;
	//int *vertex_index_stack = malloc(sizeof(int) * brush->vertex_count);
	
	int vertex_count = 0;
	vec3_t *vertex_buffer = malloc(sizeof(vec3_t) * 12800);
	
	int normal_count = 0;
	vec3_t *normal_buffer = malloc(sizeof(vec3_t) * (brush->vertex_count + 1000));
	int *plane_point_indexes = malloc(sizeof(int) * (brush->vertex_count + 1000));
	
	int plane_point_count = 0;
	
	vec3_t polygon_center;
	vec3_t v0;
	vec3_t v1;
	
	float angle;
	float smallest_angle;
	int closest_vertex_index;
	
	
	
	vec3_t plane_point;
	vec3_t plane_normal;
	//c = brush->triangle_group_count;
	c = brush->vertex_count;
	
	
	
	/* Gather all normals... */
	for(i = 0; i < c; i++)
	{
		normal_buffer[normal_count] = brush->vertices[i].normal;
		plane_point_indexes[normal_count] = i;
		normal_count++;
	}
	
	
	/* Remove duplicated normals... */
	for(i = 0; i < normal_count; i++)
	{
		for(j = i + 1; j < normal_count; j++)
		{
			if(normal_buffer[i].x == normal_buffer[j].x &&
			   normal_buffer[i].y == normal_buffer[j].y &&
			   normal_buffer[i].z == normal_buffer[j].z)
			{
				for(l = j; l < normal_count - 1; l++)
				{
					normal_buffer[l] = normal_buffer[l + 1];
					plane_point_indexes[l] = plane_point_indexes[l + 1];
				}
				j--;
				normal_count--;
			}
		}
	}
	
	
	
	for(j = 0; j < normal_count; j++)
	{
		printf("[%f %f %f] %d\n", normal_buffer[j].x, normal_buffer[j].y, normal_buffer[j].z, plane_point_indexes[j]);
	}
	
	
	
	c = normal_count;
	
	
	for(i = 0; i < c; i++)
	{
		plane_point = brush->vertices[plane_point_indexes[i]].position;
		plane_normal = normal_buffer[i];
		
		k = brush->vertex_count;
		
		vertex_count = 0;
		
		for(j = 0; j < k; j++)
		{	
			if(bsp_ClassifyPoint(brush->vertices[j].position, plane_point, plane_normal) == POINT_CONTAINED)
			{
				vertex_buffer[vertex_count] = brush->vertices[j].position;
				
				vertex_count++;
			}
		}	
		/* check for duplicated vertices */
		for(j = 0; j < vertex_count; j++)
		{
			for(k = j + 1; k < vertex_count; k++)
			{		
				
				if(vertex_buffer[j].x == vertex_buffer[k].x && 
				   vertex_buffer[j].y == vertex_buffer[k].y &&
				   vertex_buffer[j].z == vertex_buffer[k].z)
				{
					/* duplicated vertex, remove it and adjust everything else... */
					for(l = k; l < vertex_count - 1; l++)
					{
						vertex_buffer[l] = vertex_buffer[l + 1];
					}
					k--;
					vertex_count--;
				}
			}
		}
		
		for(j = 0; j < vertex_count; j++)
		{
			printf("[%f %f %f]\n", vertex_buffer[j].x, vertex_buffer[j].y, vertex_buffer[j].z);
		}
		printf("\n");
		
		
		/* avoid cylinder brushes for the time being... */
		
		#if 0
		
		/* remove any vertex "inside" this polygon... */
		for(j = 0; j < vertex_count - 1; j++)
		{
			r = (j + 1) % vertex_count;
			
			/* current edge... */
			v0.x = vertex_buffer[r].x - vertex_buffer[j].x;
			v0.y = vertex_buffer[r].y - vertex_buffer[j].y;
			v0.z = vertex_buffer[r].z - vertex_buffer[j].z;
			
			for(l = 1; l < vertex_count - 2; l++)
			{
				/* start at the next vertice past the end of the current edge... */
				k = (l + r) % vertex_count;	
				
				v1.x = vertex_buffer[k].x - vertex_buffer[j].x;
				v1.y = vertex_buffer[k].y - vertex_buffer[j].y;
				v1.z = vertex_buffer[k].z - vertex_buffer[j].z;
				
				v1 = cross(v0, v1);
				
				if(dot3(plane_normal, v1) < 0.0)
				{

				}
				
			}
			
		}
		
		#endif
		
		//plane_point = vertex_buffer[0];
		
		
		polygon_center = vertex_buffer[0];
		for(j = 1; j < vertex_count; j++)
		{
			polygon_center.x += vertex_buffer[j].x;
			polygon_center.y += vertex_buffer[j].y;
			polygon_center.z += vertex_buffer[j].z;
		}
		
		polygon_center.x /= (float)vertex_count;
		polygon_center.y /= (float)vertex_count;
		polygon_center.z /= (float)vertex_count;
		
		
		
		
		/* sort the vertices anti clock-wise... */
		for(j = 0; j < vertex_count - 1; j++)
		{
			v0.x = vertex_buffer[j].x - polygon_center.x;
			v0.y = vertex_buffer[j].y - polygon_center.y;
			v0.z = vertex_buffer[j].z - polygon_center.z;
			v0 = normalize3(v0);
			smallest_angle = 10.0;
			closest_vertex_index = j;
			
			for(k = j + 1; k < vertex_count; k++)
			{
				r = k % vertex_count;
			
				v1.x = vertex_buffer[r].x - polygon_center.x;
				v1.y = vertex_buffer[r].y - polygon_center.y;
				v1.z = vertex_buffer[r].z - polygon_center.z;
				
				v1 = normalize3(v1);
				angle = get_angle(v0, v1, plane_normal);
				if(angle < smallest_angle)
				{
					smallest_angle = angle;
					closest_vertex_index = r;
				}
			}
			
			/*printf("[%f %f %f]\n", vertex_buffer[closest_vertex_index].x,
								   vertex_buffer[closest_vertex_index].y,
								   vertex_buffer[closest_vertex_index].z);*/
			
			r = (j + 1) % vertex_count;
			
			v1 = vertex_buffer[r];
			vertex_buffer[r] = vertex_buffer[closest_vertex_index];
			vertex_buffer[closest_vertex_index] = v1;
			
		}
		
		//printf("polygon!\n");
		printf("\n");
		
		for(j = 0; j < vertex_count; j++)
		{
			printf("[%f %f %f] [%f %f %f]\n", vertex_buffer[j].x, vertex_buffer[j].y, vertex_buffer[j].z, plane_normal.x, plane_normal.y, plane_normal.z);
		}
		printf("\n\n");
		
		assert(vertex_count > 2);
		
		p = malloc(sizeof(bsp_polygon_t));
		p->vertices = malloc(sizeof(vertex_t) * vertex_count);
		p->normal = plane_normal;
		p->vert_count = vertex_count;
		p->polygon_index = polygon_index++;
		
		for(j = 0; j < vertex_count; j++)
		{
			p->vertices[j] = vertex_buffer[j];
		}
		
		p->next = polygons;
		polygons = p;
		
	}
	
	free(vertex_buffer);
	free(normal_buffer);
	free(plane_point_indexes);
	return polygons;
	
	//free(vertex_index_stack);
}
#endif


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
	
	
	out = malloc(sizeof(vec2_t ) * in_vert_count);
	
	
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
	
	free(out);
	
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
		*hull_verts = malloc(sizeof(vec2_t) * 3);
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
		
	
	
	out = malloc(sizeof(vec2_t ) * in_vert_count);
	hull = malloc(sizeof(vec2_t ) * in_vert_count * 2);
	
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
	
	
	
	*hull_verts = malloc(sizeof(vec2_t ) * hull_count);
	*hull_vert_count = hull_count;
	
	t = *hull_verts;
	
	for(i = 0; i < hull_count; i++)
	{
		t[i] = hull[i];
	}
	
	
	
	free(hull);
	free(out);
	
	
	
	
	
}


#if 0

bsp_polygon_t *bsp_BuildPolygonsFromBrush(brush_t *brush)
{
	int i;
	int c;
	int j;
	int k;
	
	int vert_count = 0;
	int hull_vert_count = 0;
	
	int plane_count = 0;
	
	float d;
	
	bsp_clipplane_t *planes;
	bsp_clipplane_t temp;
	vec3_t *verts = NULL;
	vec2_t *proj_verts = NULL;
	vec2_t *proj_hull = NULL;
	vec3_t basis0;
	vec3_t basis1;
	vec3_t basis2;
	
	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *polygon = NULL;
	
	vec3_t edge0;
	vec3_t edge1; 
	vec3_t triangle_normal;
	
	plane_count = brush->vertex_count / 3;
	planes = malloc(sizeof(bsp_clipplane_t ) * plane_count);
	verts = malloc(sizeof(vec3_t ) * brush->vertex_count);
	proj_verts = malloc(sizeof(vec3_t) * brush->vertex_count);
	
	
	c = brush->vertex_count;
	
	
	for(i = 0; i < c; i += 3)
	{
		edge0.x = brush->vertices[i + 1].position.x - brush->vertices[i].position.x;
		edge0.y = brush->vertices[i + 1].position.y - brush->vertices[i].position.y;
		edge0.z = brush->vertices[i + 1].position.z - brush->vertices[i].position.z;
		
		
		edge1.x = brush->vertices[i + 2].position.x - brush->vertices[i + 1].position.x;
		edge1.y = brush->vertices[i + 2].position.y - brush->vertices[i + 1].position.y;
		edge1.z = brush->vertices[i + 2].position.z - brush->vertices[i + 1].position.z;
		
		/* actual normal direction doesn't matter that much as
		long it's being calculated consistently... */
		
		/* NOTE: shouldn't allow brushes to have zero volume,
		as a zero area triangle will mess stuff up here... */
		triangle_normal = normalize3(cross(edge0, edge1));
		
		
		planes[i / 3].point = brush->vertices[i].position;
		planes[i / 3].normal = triangle_normal;
	}
	
	
	for(i = 0; i < plane_count; i++)
	{
		for(j = i + 1; j < plane_count; j++)
		{
			if(planes[i].normal.x == planes[j].normal.x &&
			   planes[i].normal.y == planes[j].normal.y &&
			   planes[i].normal.z == planes[j].normal.z)
			{
				if(j < plane_count - 1)
				{
					planes[j] = planes[plane_count - 1];
				}
				
				plane_count--;
				j--;
			}
		}
	}
	
	for(i = 0; i < plane_count; i++)
	{
		printf("[%f %f %f]\n", planes[i].normal.x, planes[i].normal.y, planes[i].normal.z);
	}
	
	
	for(i = 0; i < plane_count; i++)
	{
		vert_count = 0;
		
		c = brush->vertex_count;
		
		for(j = 0; j < c; j++)
		{
			edge0.x = brush->vertices[j].position.x - planes[i].point.x;
			edge0.y = brush->vertices[j].position.y - planes[i].point.y;
			edge0.z = brush->vertices[j].position.z - planes[i].point.z;
			
			d = dot3(planes[i].normal, edge0);
				
			/* vertex on plane... */
			if(d <= FUZZY_ZERO && d >= -FUZZY_ZERO)
			{
				verts[vert_count] = brush->vertices[j].position;
				
				vert_count++;
			}
				
		}
		
		for(j = 0; j < vert_count; j++)
		{
			edge0.x = verts[j].x - planes[i].point.x;
			edge0.y = verts[j].y - planes[i].point.y;
			edge0.z = verts[j].z - planes[i].point.z; 
				
			/* this is a valid edge... */
			if(edge0.x != 0.0 || edge0.y != 0.0 || edge0.z != 0.0)
			{
				break;
			}
		}
			
		basis0 = normalize3(edge0);
		basis1 = cross(planes[i].normal, basis0);
			//basis2 = cross(basis1, basis0);
			
			
		for(j = 0; j < vert_count; j++)
		{
			proj_verts[j].x = dot3(verts[j], basis0);
			proj_verts[j].y = dot3(verts[j], basis1);
		}
		
		hull_vert_count = 0;
		
		bsp_QuickHull(proj_verts, vert_count, &proj_hull, &hull_vert_count);
			
		/* quick hull here... */
		
		d = dot3(planes[i].point, planes[i].normal);

		for(j = 0; j < hull_vert_count; j++)
		{
			verts[j].x = proj_hull[j].x * basis0.x + proj_hull[j].y * basis1.x + planes[i].normal.x * d;
			verts[j].y = proj_hull[j].x * basis0.y + proj_hull[j].y * basis1.y + planes[i].normal.y * d;
			verts[j].z = proj_hull[j].x * basis0.z + proj_hull[j].y * basis1.z + planes[i].normal.z * d;
		}
			
			
		polygon = malloc(sizeof(bsp_polygon_t));
		polygon->vert_count = hull_vert_count;
		polygon->normal = planes[i].normal;
		polygon->vertices = malloc(sizeof(vertex_t) * hull_vert_count);
		polygon->b_used = 0;
			
		for(j = 0; j < hull_vert_count; j++)
		{
			polygon->vertices[j] = verts[j];
		}
			
		polygon->next = polygons;
		polygons = polygon;
		 
	}
	
	
	free(verts);
	free(proj_verts);
	free(proj_hull);
	free(planes);
	
	return polygons;
	
}


#endif



#if 1
/*
==============
bsp_BuildPolygonsFromBrush
==============
*/
bsp_polygon_t *bsp_BuildPolygonsFromBrush(brush_t *brush)
{
	int i;
	int c;
	
	int j;
	int k;
	
	int l;
	
	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *polygon;
	
	
	bsp_triangle_t *triangle = NULL;
	bsp_triangle_t *triangles = NULL;
	
	
	vertex_t tri_verts[3];
	
	vec3_t v;
	vertex_t p;
	
	
	switch(brush->type)
	{
		case BRUSH_CUBE:
		case BRUSH_CYLINDER:
		case BRUSH_BOUNDS:
			for(i = 0; i < 6; i++)
			{
				
				triangles = NULL;
				
				polygon = malloc(sizeof(bsp_polygon_t));
				v = cube_bmodel_collision_normals[i];
					
					
					
				p.normal.x = v.x * brush->orientation.floats[0][0] + 
				      v.y * brush->orientation.floats[1][0] +
					  v.z * brush->orientation.floats[2][0];
						  
						  
				p.normal.y = v.x * brush->orientation.floats[0][1] + 
				      v.y * brush->orientation.floats[1][1] +
					  v.z * brush->orientation.floats[2][1];
						  
						  
				p.normal.z = v.x * brush->orientation.floats[0][2] + 
				      v.y * brush->orientation.floats[1][2] +
					  v.z * brush->orientation.floats[2][2];
				
				
				polygon->normal = p.normal;
				
				polygon->vertices = malloc(sizeof(vertex_t) * 4);
				polygon->vert_count = 4;
				
				
				for(c = 0; c < 4; c++)
				{
					v = cube_bmodel_collision_verts[i * 4 + c];
					
					v.x *= brush->scale.x;
					v.y *= brush->scale.y;
					v.z *= brush->scale.z;
					
					
					p.position.x = v.x * brush->orientation.floats[0][0] + 
					       		   v.y * brush->orientation.floats[1][0] +
						   		   v.z * brush->orientation.floats[2][0] + brush->position.x;
						  
						  
					p.position.y = v.x * brush->orientation.floats[0][1] + 
					      		   v.y * brush->orientation.floats[1][1] +
						  		   v.z * brush->orientation.floats[2][1] + brush->position.y;
						  
						  
					p.position.z = v.x * brush->orientation.floats[0][2] + 
					      		   v.y * brush->orientation.floats[1][2] +
						  		   v.z * brush->orientation.floats[2][2] + brush->position.z;	  	  
					
					
					polygon->vertices[c] = p;
				}
				
		
				polygon->b_used = 0;
				polygon->next = polygons;
				polygons = polygon;
			}
		break;
			
	}
	
	
	
	
	return polygons;
	
}

#endif


bsp_polygon_t *bsp_BuildPolygonsFromTriangles(bsp_triangle_t *triangles)
{
	bsp_triangle_t *t;
	
	t = triangles;
	
	
	while(t)
	{
		
		
		t = t->next;
	}
	
}



bsp_triangle_t *bsp_BuildTrianglesFromBrushes()
{
	bsp_triangle_t *triangles = NULL;
	bsp_triangle_t *r = NULL;
	bsp_triangle_t *t;
	bsp_polygon_t *polygon;
	brush_t *brush;
	
	int i;
	int c = brush_count;
	int j;
	int k;
	
	
	for(i = 0; i < c; i++)
	{
		
		brush = &brushes[i];
		
		if(brush->type == BRUSH_INVALID)
			continue;
		
		if(brush->type == BRUSH_BOUNDS)
			continue;	
			
		
		//bsp_TriangulateP
		
		
		
		k = brush->index_count;
		
		for(j = 0; j < k;)
		{
			r = malloc(sizeof(bsp_triangle_t));
		
			r->material_index = 0;
			
			r->a = brush->vertices[brush->indexes[j]];
			j++;
			
			r->b = brush->vertices[brush->indexes[j]];
			j++;
			
			r->c = brush->vertices[brush->indexes[j]];
			j++;
			
			r->next = triangles;
			triangles = r;
		}
		
	}
	
	return triangles;
	
	
}

/*
==============
bsp_ClipPolygonToBsp
==============
*/
bsp_polygon_t *bsp_ClipPolygonToBsp(bsp_node_t *bsp, bsp_polygon_t *polygon, int copy)
{	
	bsp_polygon_t *frags = NULL;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *front = NULL;
	bsp_polygon_t *back = NULL;
	
	bsp_triangle_t *t = NULL;
	bsp_triangle_t *n = NULL;
	bsp_triangle_t *new_triangle = NULL;
	bsp_triangle_t *new_triangles = NULL;
		
	int i;
	int c;
	
	
	if(bsp->type == BSP_LEAF)
	{
		/* this polygon ended inside an 'in' node, so discard it (Union operation)... */
		if(bsp->bm_flags & BSP_SOLID)
		{
			/* (!copy) means that it's not necessary to copy this
			polygon, meaning it doesn't belong to any bsp tree,
			so it's safe to get rid of it here... */
			if(!copy)
			{				
				free(polygon->vertices);
				free(polygon);	
			}
			return NULL;
		}
		

		/* return a new copy of this polygon if this polygon
		didn't come from a split... */
		if(copy)
		{
			assert(polygon);
			
			front = bsp_DeepCopyPolygon(polygon);
			
			
			//front->next = NULL;
			return front;
		}
		else
		{
			polygon->next = NULL;
			return polygon;
		}
		
	}
	else
	{
		i = bsp_ClassifyPolygon(polygon, bsp->splitter->vertices[0].position, bsp->splitter->normal);
		
		switch(i)
		{
			case POLYGON_FRONT:
			case POLYGON_CONTAINED_FRONT:
				return bsp_ClipPolygonToBsp(bsp->front, polygon, copy);
			break;
			
			case POLYGON_BACK:
			case POLYGON_CONTAINED_BACK:
				return bsp_ClipPolygonToBsp(bsp->back, polygon, copy);
			break;
			
			case POLYGON_STRADDLING:
				
				/* bsp_SplitPolygon allocates two new polygons... */
				bsp_SplitPolygon(polygon, bsp->splitter->vertices[0].position, bsp->splitter->normal, &front, &back);
				
				if(!copy)
				{	
					free(polygon->vertices);
					free(polygon);
				}
				
				/*... so tell any recursive call that it isn't necessary 
				to make a copy before returning the polygon when it falls 
				within an out leaf (or freeing it when it falls in solid
				space)... */
				front = bsp_ClipPolygonToBsp(bsp->front, front, 0);
				back = bsp_ClipPolygonToBsp(bsp->back, back, 0);
				
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

bsp_polygon_t *bsp_ClipContiguousPolygonsToBsp(bsp_node_t *bsp, bsp_polygon_t *polygons, int do_copy)
{
	bsp_polygon_t *p; // = bsp_DeepCopyPolygons(polygons_a);	
	bsp_polygon_t *r;
	bsp_polygon_t *s;
	bsp_polygon_t *clipped;
	//bsp_BuildSolid(&bsp_a, polygons_a, 0, 0);

	r = bsp_DeepCopyPolygons(polygons);

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
	
	bsp_DeletePolygonsContiguous(polygons);
	r = bsp_DeepCopyPolygonsContiguous(clipped);
	bsp_DeletePolygons(clipped);
	
	return r;
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


struct bsp_ClipBspToBspInputData
{
	bsp_node_t *bsp;
	bsp_node_t *input;
	SDL_sem *semaphore;
	bsp_polygon_t **polygons;
};


int bsp_ClipBspToBspAssyncFn(void *data)
{
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
	struct bsp_ClipBspToBspInputData *in = (struct bsp_ClipBspToBspInputData *)data;	
	*(in->polygons) = bsp_ClipBspToBsp(in->bsp, in->input);
	SDL_SemPost(in->semaphore);
	free(in);
	SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
	return 0;
}





void bsp_ClipBspToBspAssync(bsp_polygon_t **polygons, bsp_node_t *bsp, bsp_node_t *input, SDL_sem *semaphore)
{
	
	struct bsp_ClipBspToBspInputData *in = malloc(sizeof(struct bsp_ClipBspToBspInputData));
	in->bsp = bsp;
	in->input = input;
	in->semaphore = semaphore;
	in->polygons = polygons;
	
	SDL_DetachThread(SDL_CreateThread(bsp_ClipBspToBspAssyncFn, "clip bsp to bsp thread", in));
	
	//printf("bsp_ClipBspToBspAsssync: %x\n", semaphore);
	
}


bsp_polygon_t *bsp_IntersectPolygonToBsp(bsp_node_t *bsp, bsp_polygon_t *polygon, int copy)
{
	bsp_polygon_t *frags = NULL;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *front = NULL;
	bsp_polygon_t *back = NULL;
	
	bsp_triangle_t *t = NULL;
	bsp_triangle_t *n = NULL;
	bsp_triangle_t *new_triangle = NULL;
	bsp_triangle_t *new_triangles = NULL;
		
	int i;
	int c;
	
	if(bsp->type == BSP_LEAF)
	{
		/* this polygon ended inside an 'in' node, so discard it (Union operation)... */
		if(!(bsp->bm_flags & BSP_SOLID))
		{
			/* (!copy) means that it's not necessary to copy this
			polygon, meaning it doesn't belong to any bsp tree,
			so it's safe to get rid of it here... */
			if(!copy)
			{				
				free(polygon->vertices);
				free(polygon);	
			}
			return NULL;
		}
		

		/* return a new copy of this polygon if this polygon
		didn't come from a split... */
		if(copy)
		{
			assert(polygon);
			
			front = bsp_DeepCopyPolygon(polygon);
			return front;
		}
		else
		{
			polygon->next = NULL;
			return polygon;
		}
		
	}
	else
	{
		i = bsp_ClassifyPolygon(polygon, bsp->splitter->vertices[0].position, bsp->splitter->normal);
		
		switch(i)
		{
			case POLYGON_FRONT:
			case POLYGON_CONTAINED_FRONT:
				return bsp_IntersectPolygonToBsp(bsp->front, polygon, copy);
			break;
			
			case POLYGON_BACK:
			case POLYGON_CONTAINED_BACK:
				return bsp_IntersectPolygonToBsp(bsp->back, polygon, copy);
			break;
			
			case POLYGON_STRADDLING:
				
				/* bsp_SplitPolygon allocates two new polygons... */
				bsp_SplitPolygon(polygon, bsp->splitter->vertices[0].position, bsp->splitter->normal, &front, &back);
				
				if(!copy)
				{	
					free(polygon->vertices);
					free(polygon);
				}
				
				/*... so tell any recursive call that it isn't necessary 
				to make a copy before returning the polygon when it falls 
				within an out leaf (or freeing it when it falls in solid
				space)... */
				front = bsp_IntersectPolygonToBsp(bsp->front, front, 0);
				back = bsp_IntersectPolygonToBsp(bsp->back, back, 0);
				
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


bsp_polygon_t *bsp_IntersectBspToBsp(bsp_node_t *bsp, bsp_node_t *input)
{
	bsp_polygon_t *polygon = NULL;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *s = NULL;
	
	int paths = 0;

	/* If this node is a leaf, it doesn't contain a
	polygon... */	
	if(input->type == BSP_LEAF) return NULL;	
			
	r = bsp_IntersectBspToBsp(bsp, input->back);
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
	r = bsp_IntersectPolygonToBsp(bsp, input->splitter, 1);
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
	
	
	r = bsp_IntersectBspToBsp(bsp, input->front);
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
	
	
	sem0 = SDL_CreateSemaphore(0);
	sem1 = SDL_CreateSemaphore(0);
	
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
				
			polygons_a = bsp_DeepCopyPolygons(brush_list[i].polygons);
	
			i++;
			
		}while(!polygons_a && i < c);
		
		brush_list[i - 1].bm_flags &= ~BRUSH_MOVED;
			
		for(; i < c; i++)
		{
			printf("%d/%d\n", i, c);
			if(brush_list[i].type == BRUSH_INVALID)
				continue;
				
			polygons_b = bsp_DeepCopyPolygons(brush_list[i].polygons);
			
			if(!polygons_b)
				continue;
			
	
			bsp_a = NULL;
			/* build the bsp for the polygons of brush A... */
			bsp_BuildSolid(&bsp_a, polygons_a, 0, 0);

			
			bsp_b = NULL;
			/* build the bsp for the polygons of brush B... */
			bsp_BuildSolid(&bsp_b, polygons_b, 0, 0);
			
			//#define MULTI_THREADS

			#ifdef MULTI_THREADS
			/* clip the polygons that belong to bsp_a against
			bsp_b... */
			bsp_ClipBspToBspAssync(&polygons_a, bsp_b, bsp_a, sem0);	
			//printf("thread 0\n");	
			
			/* clip the polygons that belong to bsp_b against
			bsp_a... */
			SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
			polygons_b = bsp_ClipBspToBsp(bsp_a, bsp_b);
			SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
			//bsp_ClipBspToBspAssync(&polygons_b, bsp_a, bsp_b, sem1);
			//printf("thread 1\n");
			
			SDL_SemWait(sem0);
			//SDL_SemWait(sem1);
			
			#else
		
			/* clip the polygons that belong to bsp_a against
			bsp_b... */
			polygons_a = bsp_ClipBspToBsp(bsp_b, bsp_a);	
	
			
			/* clip the polygons that belong to bsp_b against
			bsp_a... */
			polygons_b = bsp_ClipBspToBsp(bsp_a, bsp_b);
			
			#endif
			
			
			/* get rid of those bsp's, they're 
			useless now. Such useless motherfuckers... */
			/* NOTE: this has become a HUGE bottleneck... */
			bsp_DeleteSolidBsp(bsp_a);
			bsp_DeleteSolidBsp(bsp_b);
			
			//bsp_DeleteSolidBspAssync(bsp_a);
			//bsp_DeleteSolidBspAssync(bsp_b);

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
	
	printf("clipping took %f\n", end - start);
	
	SDL_DestroySemaphore(sem0);
	SDL_DestroySemaphore(sem1);
	
	
	
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
				
			p = bsp_DeepCopyPolygons(brush_list[i].polygons);
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
		
		bsp_DeleteSolidBsp(bsp_a);
	}
	
	end = engine_GetDeltaTime();
	
	printf("clipping took %f\n", end - start);
	
	//SDL_DestroySemaphore(sem0);
	//SDL_DestroySemaphore(sem1);
	
	
	
	return polygons_a;	
}


bsp_polygon_t *bsp_SubtractBrush(brush_t *brush, brush_t *subtractive)
{
	
}



/*
==============
bsp_ClipTriangleToSolidLeaves
==============
*/
void bsp_ClipTriangleToSolidLeaves(bsp_node_t *root, bsp_triangle_t *triangle)
{
	bsp_triangle_t *front = NULL;
	bsp_triangle_t *back = NULL;
	bsp_triangle_t *t = NULL;
	bsp_triangle_t *n;
	bsp_leaf_t *leaf;
	
	int s;
	int c;
	
	if(root->type == BSP_LEAF)
	{
		if(!(root->bm_flags & BSP_SOLID))
		{			
			/* add this triangle to the leaf... */
			leaf = (bsp_leaf_t *)root;
			triangle->next = leaf->triangles;
			leaf->triangles = triangle;
			leaf->triangle_count++;
			return;	
		}
		
		/* this triangle ended up inside solid space, so
		get rid of it... */
		free(triangle);
		return;
	}
	
	s = bsp_ClassifyTriangle(triangle, root->point, root->normal);
	
	switch(s)
	{
		case TRIANGLE_FRONT:
		case TRIANGLE_CONTAINED_FRONT:
			bsp_ClipTriangleToSolidLeaves(root->front, triangle);
		break;
		
		case TRIANGLE_BACK:
		case TRIANGLE_CONTAINED_BACK:
			bsp_ClipTriangleToSolidLeaves(root->back, triangle);
		break;
		
		case TRIANGLE_STRADDLING:
			bsp_SplitTriangle(triangle, root->point, root->normal, &front, &back);
				
			/* front can contain one or two triangles, so... */
			t = front;
			do
			{
				/* get the next before the call, as it
				can either unlink the triangle or free it... */
				n = t->next;
				
				bsp_ClipTriangleToSolidLeaves(root->front, t);

				t = n;
			}while(t);
			
			/* back can contain one or two triangles, so... */
			t = back;
			do
			{
				/* get the next before the call, as it
				can either unlink the triangle or free it... */
				n = t->next;
				
				bsp_ClipTriangleToSolidLeaves(root->back, t);
				
				t = n;
			}while(t);
			
			
			free(triangle);
			
		break;	
	}
}


/*
==============
bsp_ClipTrianglesToSolidLeaves
==============
*/
void bsp_ClipTrianglesToSolidLeaves(bsp_node_t *root, bsp_triangle_t *triangles)
{
	bsp_triangle_t *t;
	bsp_triangle_t *n;
		
	t = triangles;
	
	while(t)
	{
		n = t->next;
		
		bsp_ClipTriangleToSolidLeaves(root, t);
		
		t = n;
	}
	
	//bsp_LinearizeLeaves(root);
	
}

void bsp_TriangulateLeafPolygons(bsp_node_t *node)
{
	
	bsp_polygon_t *polygon = NULL;
	bsp_triangle_t *triangles = NULL;
	bsp_triangle_t *triangle = NULL;
	bsp_leaf_t *leaf;
	
	vertex_t *vertices = NULL;
	int vertice_count = 0;
	int triangle_count = 0;
	
	int i;
	
	
	
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
					triangle = malloc(sizeof(bsp_triangle_t));
					
					triangle->a = vertices[i];
					i++;
					triangle->b = vertices[i];
					i++;
					triangle->c = vertices[i];
					i++;
					
					triangle->material_index = polygon->material_index;
					
					triangle->next = triangles;
					triangles = triangle;
					
					triangle_count++;
					
				}
				polygon = polygon->next;
			}
			
			
			leaf->triangles = triangles;
			leaf->triangle_count = triangle_count;
			
			
		}
	}
	else
	{
		bsp_TriangulateLeafPolygons(node->front);
		bsp_TriangulateLeafPolygons(node->back);
	}
}

/*
==============
bsp_CountNodesAndLeaves
==============
*/
void bsp_CountNodesAndLeaves(bsp_node_t *bsp, int *leaves, int *nodes)
{
	
	bsp_leaf_t *leaf;
	
	if(!bsp)
		return;
	
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
}




/*
==============
bsp_BuildTriangleGroups
==============
*/
void bsp_BuildTriangleGroups(bsp_node_t *root, triangle_group_t **groups, int *count)
{
	triangle_group_t *g = NULL;
	triangle_group_t *n = NULL;
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
					n = malloc(sizeof(triangle_group_t) * (c + 1));
					
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
						free(g);
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

void bsp_RecursiveLinearizeBsp(bsp_node_t *bsp, vertex_t *vertices, int *vertex_count, bsp_pnode_t *lnodes, int *lnode_count, bsp_dleaf_t *lleaves, int *lleaves_count, triangle_group_t *groups, int tri_group_count, int create_leaves, int pvs_size)
{
	bsp_leaf_t *leaf;
	bsp_pnode_t *pnode;
	bsp_dleaf_t *dleaf;
	
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
			dleaf->tris = malloc(sizeof(bsp_striangle_t ) * dleaf->tris_count);
			//dleaf->pvs = leaf->pvs;
			dleaf->pvs = malloc(pvs_size);
			leaf->pvs = dleaf->pvs;
			
			for(i = 0; i < pvs_size; i++)
			{
				dleaf->pvs[i] = 0x0;
			}
			
			dleaf->leaf_index = leaf->leaf_index;
			
			
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
				/* ...and which triangle group it belongs to... */
				dleaf->tris[tris_index].triangle_group = i;
				
				tris_index++;
				
				/* copy this triangle's vertices to the appropriate region inside
				the triangle array... */			
				vertices[k + groups[i].next] = triangle->a;
				if(triangle->a.position.x > x_max) x_max = triangle->a.position.x;
				if(triangle->a.position.y > y_max) y_max = triangle->a.position.y;
				if(triangle->a.position.z > z_max) z_max = triangle->a.position.z;
				
				if(triangle->a.position.x < x_min) x_min = triangle->a.position.x;
				if(triangle->a.position.y < y_min) y_min = triangle->a.position.y;
				if(triangle->a.position.z < z_min) z_min = triangle->a.position.z;
				groups[i].next++;
				
				vertices[k + groups[i].next] = triangle->b;
				if(triangle->b.position.x > x_max) x_max = triangle->b.position.x;
				if(triangle->b.position.y > y_max) y_max = triangle->b.position.y;
				if(triangle->b.position.z > z_max) z_max = triangle->b.position.z;
				
				if(triangle->b.position.x < x_min) x_min = triangle->b.position.x;
				if(triangle->b.position.y < y_min) y_min = triangle->b.position.y;
				if(triangle->b.position.z < z_min) z_min = triangle->b.position.z;
				groups[i].next++;
				
				vertices[k + groups[i].next] = triangle->c;
				if(triangle->c.position.x > x_max) x_max = triangle->c.position.x;
				if(triangle->c.position.y > y_max) y_max = triangle->c.position.y;
				if(triangle->c.position.z > z_max) z_max = triangle->c.position.z;
				
				if(triangle->c.position.x < x_min) x_min = triangle->c.position.x;
				if(triangle->c.position.y < y_min) y_min = triangle->c.position.y;
				if(triangle->c.position.z < z_min) z_min = triangle->c.position.z;
				groups[i].next++;
				
				next_triangle = triangle->next;
				free(triangle);
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
			
			
			/* get rid of the polygons stored on this leaf,
			since this pointer based bsp is useless now... */
			/*polygon = leaf->polygons;
			while(polygon)
			{
				next_polygon = polygon->next;
				free(polygon->vertices);
				free(polygon);
				polygon = next_polygon;
			}*/
			
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
		bsp_RecursiveLinearizeBsp(bsp->front, vertices, vertex_count, lnodes, lnode_count, lleaves, lleaves_count, groups, tri_group_count, create_leaves, pvs_size);
		
		pnode->child[1] = (*lnode_count) - node_index;
		bsp_RecursiveLinearizeBsp(bsp->back, vertices, vertex_count, lnodes, lnode_count, lleaves, lleaves_count, groups, tri_group_count, create_leaves, pvs_size);
		
		#ifndef LEAK_FOR_FUN
		//free(bsp);
		#endif
		
	}
}



/*
==============
bsp_LinearizeBsp
==============
*/
void bsp_LinearizeBsp(bsp_node_t *bsp, vertex_t **vertices, int *vertex_count, bsp_pnode_t **lnodes, int *lnode_count, bsp_dleaf_t **lleaves, int *lleaves_count, triangle_group_t *groups, int tri_group_count, int create_leaves)
{
	
	int i;
	int c = 0;
	int n = 0;
	int l = 0;
	
	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *polygon = NULL;
	bsp_pnode_t *nodes = NULL;
	bsp_dleaf_t *leaves = NULL;
	vertex_t *v;
	
	int pvs_size = 0;
	
	if(vertex_count)
	{
		*vertex_count = groups[tri_group_count - 1].start + groups[tri_group_count - 1].next;
		v = malloc(sizeof(vertex_t) * (*vertex_count));
	}
		
	
	
	
	bsp_CountNodesAndLeaves(bsp, &l, &n);
	
//	printf("%d %d\n", l, n);
	
	nodes = malloc(sizeof(bsp_pnode_t) * n);
	
	if(lleaves)
	{
		leaves = malloc(sizeof(bsp_dleaf_t) * (l + 200));
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
	
	bsp_RecursiveLinearizeBsp(bsp, v, &i, nodes, &n, leaves, &l, groups, tri_group_count, create_leaves, pvs_size);
	
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
						
							
							edge = malloc(sizeof(bsp_edge_t) );
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

bsp_edge_t *bsp_BuildEdgesFromBrushes()
{
	
	/*int i;
	
	bsp_edge_t *edge_list = NULL;
	bsp_edge_t *partial_edge_list = NULL;
	bsp_edge_t *r = NULL;
	for(i = 0; i < brush_count; i++)
	{
		partial_edge_list = bsp_BuildEdgesFromBrush(&brushes[i]);
		
		assert(partial_edge_list);
		
		r = partial_edge_list;
		
		while(r->next)
		{
			r = r->next;
		}
		
		r->next = edge_list;
		edge_list = partial_edge_list;
	}
	
	return edge_list;*/
}

void bsp_ExpandBrushes(vec3_t box_extents)
{
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
			polygons = expanded_brushes[i].polygons;
			
			while(polygons)
			{
				polygon = polygons->next;
				free(polygons->vertices);
				free(polygons);
				polygons = polygon;
			}
			
		}
		
		free(expanded_brushes);
		
	}
	
	
	
	expanded_brushes = malloc(sizeof(brush_t) * brush_list_size);
/*	
	#ifdef DRAW_BEVEL_EDGES
		
	if(bevel_edges)
	{
		edges = bevel_edges;
		while(edges)
		{
			edge = edges->next;
			free(edges);
			edges = edge;
		}
		
		bevel_edges = NULL;
	}
	
	#endif*/
	
	
		
	for(i = 0, c = 0; i < brush_count; i++)
	{
		//edges = bsp_BuildEdgesFromBrush(&brushes[i]);
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		
		
		
	//	#if 0
		
		//polygons = bsp_BuildPolygonsFromBrush(&brushes[i]);
		polygons = bsp_DeepCopyPolygons(brushes[i].polygons);
		
		if(!polygons)
			continue;
		
		edges = bsp_BuildBevelEdges(polygons);
		
		expanded_brushes[c] = brushes[i];
		expanded_brushes[c].polygons = polygons;
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
			free(edges);
			edges = edge;
		}
		
		//#endif
		
		c++;

			
	}
	
	expanded_brush_count = c;
	
}

void bsp_BuildCollisionBsp()
{
	
	bsp_polygon_t *polygons;
	bsp_polygon_t *p;
	bsp_polygon_t *r;
	bsp_node_t *root = NULL;
	int c;
	
//	printf("bsp_BuildCollisionBsp: bsp_ExpandBrushes... ");
	bsp_ExpandBrushes(vec3(PLAYER_X_EXTENT, PLAYER_Y_EXTENT, PLAYER_Z_EXTENT));
//	printf("done\n");
	//printf("bsp_ExpandBrushes\n");
	
//	printf("bsp_BuildCollisionBsp: bsp_ClipBrushes... ");
	polygons = bsp_ClipBrushes(expanded_brushes, expanded_brush_count);
	//polygons = bsp_ClipBrushes2(expanded_brushes, expanded_brush_count);
//	printf("done\n");
	//printf("bsp_ClipBrushes\n");
		
	if(!polygons)
	{
		//printf("bsp_BuildCollisionBsp: !polygons\n");
		return;
	}
		
	
	/*#ifdef DRAW_EXPANDED_BRUSHES
	p = expanded_polygons;
	
	while(p)
	{
		r = p->next;
		free(p->vertices);
		free(p);
		p = r;
	}
	
	expanded_polygons = bsp_DeepCopyPolygons(polygons);
	#endif	*/
		
//	printf("bsp_BuildCollisionBsp: bsp_BuildSolidLeaf... ");
	bsp_BuildSolidLeaf(&collision_bsp, polygons);
	//bsp_BuildSolid(&collision_bsp, polygons, 0, 0);
//	printf("done\n");
	//bsp_BuildSolid(&root, polygons);
	//printf("bsp_BuildSolidLeaf\n");
	
//	printf("bsp_BuildCollisionBsp: bsp_LinearizeBsp... ");
	bsp_LinearizeBsp(collision_bsp, NULL, NULL, &collision_nodes, &collision_nodes_count, NULL, NULL, NULL, 0, 0);
//	printf("done\n");
	//printf("bsp_LinearizeBsp\n");
	
}


/*
==============
bsp_MergeLeafTriangles
==============
*/
void bsp_MergeLeafTriangles(bsp_node_t *root)
{
	bsp_leaf_t *leaf;
	bsp_triangle_t *triangles;
	bsp_triangle_t *t;
	bsp_triangle_t *pt = NULL;
	bsp_triangle_t *n;
	bsp_triangle_t *pn = NULL;
	bsp_triangle_t *q;
	//bsp_triangle_t *r;
	
	int new_triangle_vert_count = 0;
	bsp_triangle_t new_triangle;
	
	vertex_t vertexes[6];
	
	vec3_t e0;
	vec3_t e1;
	
	/*vec3_t center;
	
	vec3_t proj_x;
	vec3_t proj_y;*/
	
	int i;
	int c;
	
	int j;
	int k;
	int r;
	
	int s0;
	int s1;
	int shared_end0;
	int shared_end1;
	int l0;
	int l1;
	float d;
	
	int removed_tris = 0;
	 
	/*vec2_t proj_verts[6];
	vec2_t proj_vec;
	vec2_t edge_vec;
	
	vec3_t verts[6];*/
	
	vertex_t t0_verts[3];
	vertex_t t1_verts[3];
	
/*	float max_x = -999999999999.0;
	float min_x = 999999999999.0;
	int max_x_index;
	int min_x_index;*/
	
	if(!root)
		return;
	
	if(root->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)root;
		if(!(leaf->bm_flags & BSP_SOLID))
		{
			
			_go_all_over_again:
							
			triangles = leaf->triangles;
			t = triangles;
			removed_tris = 0;
			while(t)
			{
				
				_restart_search:
					
				t0_verts[0] = t->a;
				t0_verts[1] = t->b;
				t0_verts[2] = t->c;
							
				//n = triangles;
				
				
				n = t->next;
				pn = t;
				//n = triangles;
				//pn = NULL;
				
				_next_tris:
				while(n)
				{				
					
					
					/*if(t == n)
					{
						pn = n;
						n = n->next;
						continue;
					}*/
					
					t1_verts[0] = n->a;
					t1_verts[1] = n->b;
					t1_verts[2] = n->c;
					
					
					
					if(t->material_index == n->material_index)
					{
						e0.x = t->a.normal.x + t->b.normal.x + t->c.normal.x;
						e0.y = t->a.normal.y + t->b.normal.y + t->c.normal.y;
						e0.z = t->a.normal.z + t->b.normal.z + t->c.normal.z;
						
						
						e1.x = n->a.normal.x + n->b.normal.x + n->c.normal.x;
						e1.y = n->a.normal.y + n->b.normal.y + n->c.normal.y;
						e1.z = n->a.normal.z + n->b.normal.z + n->c.normal.z;
						
						
						e0 = normalize3(e0);
						e1 = normalize3(e1);
						
						if(dot3(e0, e1) == 1.0)
						{
							/* find a coincident edge... */
							for(j = 0; j < 3; j++)
							{
								
								e0.x = fabs(t0_verts[(j + 1) % 3].position.x - t0_verts[j].position.x);
								e0.y = fabs(t0_verts[(j + 1) % 3].position.y - t0_verts[j].position.y);
								e0.z = fabs(t0_verts[(j + 1) % 3].position.z - t0_verts[j].position.z);
								
								for(k = 0; k < 3; k++)
								{
									
									e1.x = fabs(t1_verts[(k + 1) % 3].position.x - t1_verts[k].position.x);
									e1.y = fabs(t1_verts[(k + 1) % 3].position.y - t1_verts[k].position.y);
									e1.z = fabs(t1_verts[(k + 1) % 3].position.z - t1_verts[k].position.z);
									
									#define SMALL_EPSILON 0.0004

									if(e0.x <= e1.x + SMALL_EPSILON && e0.x >= e1.x - SMALL_EPSILON) 
									{
										if(e0.y <= e1.y + SMALL_EPSILON && e0.y >= e1.y - SMALL_EPSILON)
										{
											if(e0.z <= e1.z + SMALL_EPSILON && e0.z >= e1.z - SMALL_EPSILON)
											{
												e0 = t0_verts[j].position;
												e1 = t1_verts[(k + 1) % 3].position;
																			
												if(e0.x <= e1.x + SMALL_EPSILON && e0.x >= e1.x - SMALL_EPSILON) 
												{
													if(e0.y <= e1.y + SMALL_EPSILON && e0.y >= e1.y - SMALL_EPSILON)
													{
														if(e0.z <= e1.z + SMALL_EPSILON && e0.z >= e1.z - SMALL_EPSILON)
														{
															e0 = t0_verts[(j + 1) % 3].position;
															e1 = t1_verts[k].position;
															
															/* in the blue corner: the ugliest if sequence on the world! */															
															if(e0.x <= e1.x + SMALL_EPSILON && e0.x >= e1.x - SMALL_EPSILON) 
															{
																if(e0.y <= e1.y + SMALL_EPSILON && e0.y >= e1.y - SMALL_EPSILON)
																{
																	if(e0.z <= e1.z + SMALL_EPSILON && e0.z >= e1.z - SMALL_EPSILON)
																	{
																		l0 = ((j + 1) * 2 - j) % 3;		
																		l1 = ((k + 1) * 2 - k) % 3;			
																		break;
																	}
																}
															}
														}
													}
												}
											}
										}											
									}
								}
								
								if(k < 3)
								{
									break;
								}
								
								
							}
							
							if(j < 3)
							{
								for(j = 0; j < 3; j++)
								{
									
									e0.x = t0_verts[(j + 1) % 3].position.x - t0_verts[j].position.x;
									e0.y = t0_verts[(j + 1) % 3].position.y - t0_verts[j].position.y;
									e0.z = t0_verts[(j + 1) % 3].position.z - t0_verts[j].position.z;
									
									e0 = normalize3(e0);
									
									for(k = 0; k < 3; k++)
									{
										e1.x = t1_verts[(k + 1) % 3].position.x - t1_verts[k].position.x;
										e1.y = t1_verts[(k + 1) % 3].position.y - t1_verts[k].position.y;
										e1.z = t1_verts[(k + 1) % 3].position.z - t1_verts[k].position.z;
										
										e1 = normalize3(e1);

										
										/* set fpu precision to 64 bits of mantissa
										(80 bit floats) before calling dot3. This
										avoid really thin but not mergeable triangles
										to get merged... */
										
										/* what a dodgy fix... */
										_control87(_PC_64, _MCW_PC);				
										
										if(dot3(e0, e1) == 1.0)
										{
											
											e0 = t0_verts[(j + 1) % 3].position;
											e1 = t1_verts[k].position;
											
											if(e0.x <= e1.x + SMALL_EPSILON && e0.x >= e1.x - SMALL_EPSILON)
											{
												if(e0.y <= e1.y + SMALL_EPSILON && e0.y >= e1.y - SMALL_EPSILON)
												{
													if(e0.z <= e1.z + SMALL_EPSILON && e0.z >= e1.z - SMALL_EPSILON)
													{
														e0 = t0_verts[(j + 1) % 3].position;
														e1 = t1_verts[k].position;
													}
												}
											} 							
											
											
											new_triangle_vert_count = 0;
											
											removed_tris++;
											
											vertexes[new_triangle_vert_count++] = t0_verts[l0];
		
											/* if l0 == j, then l0 + 1 is the vertex
											we're trying to get rid of...  */
											if(l0 != j)
											{
												vertexes[new_triangle_vert_count++] = t0_verts[(l0 + 1) % 3];
											}
											
											vertexes[new_triangle_vert_count++] = t1_verts[l1];
											
											if(new_triangle_vert_count < 3)
											{
												vertexes[new_triangle_vert_count++] = t1_verts[(l1 + 1) % 3];
											}
										
											leaf->triangle_count--;
										
											t->a = vertexes[0];
											t->b = vertexes[1];
											t->c = vertexes[2];
											
											
											pn->next = n->next;	
														
											free(n);
											
											goto _restart_search;
											
											
											//q = n->next;
											//free(n);
											//n = q;
											
											//goto _next_tris;
											
										}
																	
									}
								}
							}
						}
					}
					
					pn = n;
					n = n->next;
				}
				t = t->next;
			}
			
			t = leaf->triangles;
						
		}
		
		/* this last passage over the list removed triangles. 
		Go over it once more to make sure no more merging is
		possible... */
		if(removed_tris)
		{
			//printf("repeat\n");
			goto _go_all_over_again;
		}
	}
	else
	{
		bsp_MergeLeafTriangles(root->front);
		bsp_MergeLeafTriangles(root->back);
	}
	
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
	
	bsp_polygon_t *cur_polygon;
	bsp_polygon_t *r;
	bsp_polygon_t *front_split;
	bsp_polygon_t *back_split;
	/*bsp_triangle_t *cur_triangle = triangles;
	bsp_triangle_t *p;
	bsp_triangle_t *r;
	bsp_triangle_t *n;*/
	
	bsp_polygon_t *splitter;
	
	assert(root);
	assert(polygons);
	
	if(!root)
	{
		return;
	}
	
	*root = malloc(sizeof(bsp_node_t));
	
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
			
		switch(bsp_ClassifyPolygon(cur_polygon, p->splitter->vertices[0].position, p->splitter->normal))
		{
			case POLYGON_FRONT:
			case POLYGON_CONTAINED_FRONT:
			case POLYGON_CONTAINED_BACK:			
				cur_polygon->next = front_list;
				front_list = cur_polygon;
					
			break;
				
			case POLYGON_BACK:	
			//case POLYGON_CONTAINED_BACK:
				cur_polygon->next = back_list;
				back_list = cur_polygon;
			break;
				
			case POLYGON_STRADDLING:

				bsp_SplitPolygon(cur_polygon, p->splitter->vertices[0].position, p->splitter->normal, &front_split, &back_split);
				
				free(cur_polygon->vertices);
				free(cur_polygon);
					
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
		p->front = malloc(sizeof(bsp_leaf_t));
		
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
		p->back = malloc(sizeof(bsp_leaf_t));
		
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
	
	/*bsp_triangle_t *cur_triangle = triangles;
	bsp_triangle_t *p;
	bsp_triangle_t *r;
	bsp_triangle_t *n;*/
	
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
		
		*root = malloc(sizeof(bsp_node_t));
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
			
			switch(bsp_ClassifyPolygon(cur_polygon, splitter->vertices[0].position, splitter->normal))
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
	
					bsp_SplitPolygon(cur_polygon, splitter->vertices[0].position, splitter->normal, &front_split, &back_split);
					
					free(cur_polygon->vertices);	
					free(cur_polygon);
					
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
		
		/*root->front = malloc(sizeof(bsp_node_t));
		root->front->bm_flags = 0;
		bsp_BuildSolidLeaf(root->front, front_list);
		
		root->back = malloc(sizeof(bsp_node_t));
		root->back->bm_flags = 0;
		bsp_BuildSolidLeaf(root->back, back_list);*/
		
	}
	else
	{
		
		*root = malloc(sizeof(bsp_leaf_t));
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



void bsp_BuildSolidPooled(bsp_node_t **root, bsp_polygon_t *polygons)
{
	#if 0
	bsp_polygon_t *front_list = NULL;
	bsp_polygon_t *back_list = NULL;
	
	bsp_triangle_t *f = NULL;
	bsp_triangle_t *b = NULL;
	bsp_triangle_t *t;
	bsp_triangle_t *n;
	
	bsp_leaf_t *leaf;
	bsp_node_t *node;
	bsp_node_t *p;
	
	bsp_polygon_t *cur_polygon;
	bsp_polygon_t *r;
	bsp_polygon_t *front_split;
	bsp_polygon_t *back_split;
	/*bsp_triangle_t *cur_triangle = triangles;
	bsp_triangle_t *p;
	bsp_triangle_t *r;
	bsp_triangle_t *n;*/
	
	bsp_polygon_t *splitter;
	
	assert(root);
	assert(polygons);
	
	if(!root)
	{
		return;
	}
	
	*root = bsp_NewNode();
	
	p = *root;
	p->front = NULL;
	p->back = NULL;
	p->bm_flags = 0;
	p->type = BSP_NODE;
	
	p->splitter = bsp_FindSplitter(&polygons, 0, 1);
	
	
	cur_polygon = polygons;
			
	while(cur_polygon)
	{		
		
		r = cur_polygon->next;
					
		if(cur_polygon == p->splitter)
		{			
			cur_polygon = cur_polygon->next;
			continue;
		}
			
		switch(bsp_ClassifyPolygon(cur_polygon, p->splitter->vertices[0].position, p->splitter->normal))
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

				bsp_SplitPolygon(cur_polygon, p->splitter->vertices[0].position, p->splitter->normal, &front_split, &back_split);
				
				free(cur_polygon);
					
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
		p->front = (bsp_node_t *)bsp_NewLeaf();
		
		leaf = (bsp_leaf_t *)p->front;
		
		assert(leaf);
		
		leaf->bm_flags = 0;
		leaf->type = BSP_LEAF;
		leaf->polygons = NULL;
		leaf->triangles = NULL;
	}
	else
	{
		bsp_BuildSolid(&p->front, front_list);	
	}
		
	if(!back_list)
	{
		p->back = (bsp_node_t *)bsp_NewLeaf();
		
		leaf = (bsp_leaf_t *)p->back;
		
		assert(leaf);
		
		leaf->bm_flags = BSP_SOLID;
		leaf->type = BSP_LEAF;
		leaf->polygons = NULL;
		leaf->triangles = NULL;
	}
	else
	{
		bsp_BuildSolid(&p->back, back_list);
	}
	
	#endif
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


void bsp_DeleteSolidBsp(bsp_node_t *bsp)
{
	bsp_leaf_t *leaf;
	
	if(!bsp)
		return;
	
	if(bsp->type == BSP_NODE)
	{
		free(bsp->splitter->vertices);
		free(bsp->splitter);
		bsp_DeleteSolidBsp(bsp->front);
		bsp_DeleteSolidBsp(bsp->back);
		
	}
	free(bsp);
}

int bsp_DeleteSolidBspAssyncFn(void *bsp)
{
	bsp_DeleteSolidBsp(bsp);
	return 0;
}


void bsp_DeleteSolidBspAssync(bsp_node_t *bsp)
{
	SDL_Thread *delete_thread = SDL_CreateThread(bsp_DeleteSolidBspAssyncFn, "delete bsp thread", bsp);
	SDL_DetachThread(delete_thread);
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
			free(polygon->vertices);
			free(polygon);
			polygon = next;
		}
			
		/*portal = (bsp_portal_t *)leaf->portals;
		
		while(portal)
		{
			next_portal = portal->next;
			free(portal->portal_polygon->vertices);
			free(portal->portal_polygon);
			free(portal);
			portal = next_portal;
		}
		
		triangle = leaf->triangles;
		
		while(triangle)
		{
			next_triangle = triangle->next;
			free(triangle);
			triangle = next_triangle;
		}	*/
	}
	else
	{
		bsp_DeleteSolidLeafBsp(bsp->front);
		bsp_DeleteSolidLeafBsp(bsp->back);
	}
	
	free(bsp);
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


void bsp_InitPool()
{
	
	int i;

	if(!node_pool)
	{
		node_pool = malloc(sizeof(bsp_node_t) * BSP_POOL_SIZE);
	}
	else
	{
		for(i = 0; i < node_pool_cursor; i++)
		{
			if(node_pool[i].splitter)
			{
				free(node_pool[i].splitter->vertices);
				free(node_pool[i].splitter);
				
				node_pool[i].splitter = NULL;
			}
		}
	}
	
	node_pool_cursor = 0;
	
	
	if(!leaf_pool)
	{
		leaf_pool = malloc(sizeof(bsp_leaf_t) * BSP_POOL_SIZE);
	}
	else
	{
		for(i = 0; i < leaf_pool_cursor; i++)
		{
			if(leaf_pool[i].polygons)
			{
				bsp_DeletePolygons(leaf_pool[i].polygons);
				leaf_pool[i].polygons = NULL;
			}
			
			if(leaf_pool[i].pvs)
			{
				free(leaf_pool[i].pvs);
			}
		}
	}
	
	leaf_pool_cursor = 0;
}

void bsp_FinishPool()
{
	int i;
	
	for(i = 0; i < node_pool_cursor; i++)
	{
		if(node_pool[i].splitter)
		{
			free(node_pool[i].splitter->vertices);
			free(node_pool[i].splitter);
				
			node_pool[i].splitter = NULL;
		}
	}
	
	for(i = 0; i < leaf_pool_cursor; i++)
	{
		if(leaf_pool[i].polygons)
		{
			bsp_DeletePolygons(leaf_pool[i].polygons);
			leaf_pool[i].polygons = NULL;
		}
			
		if(leaf_pool[i].pvs)
		{
			free(leaf_pool[i].pvs);
		}
	}
	if(node_pool)
	{
		free(node_pool);
		node_pool = NULL;
	}
		
		
	if(leaf_pool)
	{
		free(leaf_pool);
		leaf_pool = NULL;
	}	
		
}




bsp_node_t *bsp_NewNode()
{
	assert(node_pool_cursor < BSP_POOL_SIZE);
	
	return &node_pool[node_pool_cursor++];
}


bsp_leaf_t *bsp_NewLeaf()
{
	assert(leaf_pool_cursor < BSP_POOL_SIZE);
	
	return &leaf_pool[leaf_pool_cursor++];
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
	int k = brush_count;
	int triangle_group_count = 0;
	bsp_polygon_t *polygons = NULL;
	bsp_polygon_t *r;
	bsp_polygon_t *s;
	bsp_node_t *bsp;
	
	
	if(b_compiling || b_calculating_pvs)
	{
		printf("bsp_CompileBsp: busy!\n");
		return;
	}
	
	b_compiling = 1;	
		
	if(world_triangle_groups)
	{
		free(world_triangle_groups);
		world_triangle_groups = NULL;
		world_triangle_group_count = 0;
	}
	
	if(world_vertices)
	{
		free(world_vertices);
		world_vertices = NULL;
	}
	
	if(world_nodes)
	{
		free(world_nodes);
		free(collision_nodes);
		
		world_nodes = NULL;
		collision_nodes = NULL;
	}
	
	if(world_leaves)
	{
		free(world_leaves);
		world_leaves = NULL;
	}
	
	if(world_bsp)
	{
		bsp_DeleteSolidLeafBsp(world_bsp);
		world_bsp = NULL;
	}
		
	bsp_triangle_t *triangles;
		
	
	
	bsp_BuildCollisionBsp();

	
		
	//polygons = bsp_ClipBrushes(brushes, brush_count); 
	
	//printf("")
	for(i = 0; i < brush_count; i++)
	{
		r = bsp_DeepCopyPolygons(brushes[i].clipped_polygons);
		s = r;
		while(s->next) s = s->next;
		s->next = polygons;
		polygons = r;
	}
		
	if(!polygons)
	{
		b_compiling = 0;
		printf("bsp_CompileBsp: !polygons\n");
		return;
	}
		
	
	printf("bsp_CompileBsp: bsp_SolidLeafBsp... ");
	world_bsp = bsp_SolidLeafBsp(polygons);
	printf("done\n");
	

	//triangles = bsp_BuildTrianglesFromBrushes();
	
	printf("bsp_CompileBsp: bsp_TriangulateLeafPolygons... ");
	bsp_TriangulateLeafPolygons(world_bsp);
	printf("done\n");
	
	//bsp_ClipTrianglesToSolidLeaves(world_bsp, triangles);

	

	//bsp_MergeLeafTriangles(world_bsp);

	
	printf("bsp_CompileBsp: bsp_BuildTriangleGroups... ");
	bsp_BuildTriangleGroups(world_bsp, &world_triangle_groups, &world_triangle_group_count);
	printf("done\n");
	
	printf("bsp_CompileBsp: bsp_LinearizeBsp... ");
	bsp_LinearizeBsp(world_bsp, &world_vertices, &world_vertices_count, &world_nodes, &world_nodes_count, &world_leaves, &world_leaves_count, world_triangle_groups, world_triangle_group_count, 1);
	printf("done\n");
	
	//bsp_CalculatePvsAssync(world_bsp);
	
	world_Update();
	
	 
	if(!step_semaphore)
	{
		step_semaphore = SDL_CreateSemaphore(0);
		polygon_copy_mutex = SDL_CreateMutex();
	}
	
	
	bsp_build_thread = SDL_CreateThread(bsp_CalculatePvsAssync, "calculate pvs thread", world_bsp);
	SDL_DetachThread(bsp_build_thread);
	
	b_compiling = 0;
	
	
	
	//bsp_DeleteSolidLeafBsp(bsp);
	

	
	
}



int bsp_CompileBspAsync(void *param)
{
	int i;
	int c;
	int j;
	int k = brush_count;
	
	
	#if 0
	
	bsp_polygon_t *polygons = NULL;
	bsp_triangle_t *triangles;
	
	printf("bsp_CompileBspAsync: bsp_ClipBrushes... ");		
	polygons = bsp_ClipBrushes(); 
	printf("done\n");
	
	printf("bsp_CompileBspAsync: bsp_DeleteSolidLeaf... ");
//	bsp_DeleteSolidLeaf(world_bsp);
	printf("done\n");
	
	printf("bsp_CompileBspAsync: bsp_SolidLeafBsp... ");
	world_bsp = bsp_SolidLeafBsp(polygons);
	printf("done\n");
	
	
	printf("bsp_CompileBspAsync: bsp_BuildTrianglesFromBrushes... ");
	triangles = bsp_BuildTrianglesFromBrushes();
	printf("done\n");
	
	printf("bsp_CompileBspAsync: bsp_ClipTrianglesToSolidLeaves... ");
	bsp_ClipTrianglesToSolidLeaves(world_bsp, triangles);
	printf("done\n\n");
		
	//bsp_MergeLeafTriangles(world_bsp);
	
	#endif
	
}

void bsp_DrawExpandedBrushes()
{
	#ifdef DRAW_EXPANDED_BRUSHES
	int i;
	int j;
	int c;
	bsp_polygon_t *polygon;
	camera_t *active_camera;
	
	
	if(!expanded_brushes)
		return;
		
		
		
	active_camera = camera_GetActiveCamera();
	
	glUseProgram(0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);	
	glLineWidth(2.0);
	glBegin(GL_LINES);
	glColor3f(1.0, 1.0 ,1.0);	
	
	polygon = expanded_polygons;
	
	while(polygon)
	{
		c = polygon->vert_count;
		
		for(j = 0; j < c;)
		{
			glVertex3f(polygon->vertices[j % c].position.x, polygon->vertices[j % c].position.y, polygon->vertices[j % c].position.z);
			j++;
			glVertex3f(polygon->vertices[j % c].position.x, polygon->vertices[j % c].position.y, polygon->vertices[j % c].position.z);
		}	
		
		polygon = polygon->next;
	}
	
		
	/*for(i = 0; i < brush_count; i++)
	{
		polygon = expanded_brushes[i].polygons;
		
		while(polygon)
		{
			c = polygon->vert_count;
			
			for(j = 0; j < c;)
			{
				glVertex3f(polygon->vertices[j % c].x, polygon->vertices[j % c].y, polygon->vertices[j % c].z);
				j++;
				glVertex3f(polygon->vertices[j % c].x, polygon->vertices[j % c].y, polygon->vertices[j % c].z);
			}	
		
			polygon = polygon->next;
		}
		
	}*/
	
	glEnd();	
	
	glLineWidth(1.0);
	
	glPopMatrix();
	
	#endif
	
}






void bsp_DrawBevelEdges()
{
	
	#ifdef DRAW_BEVEL_EDGES
	
	bsp_edge_t *edge;
	camera_t *active_camera = camera_GetActiveCamera();
	
	
	if(!bevel_edges)
		return;
	
	glUseProgram(0);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	
	edge = bevel_edges;
	
	glLineWidth(4.0);
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	
	while(edge)
	{
		glVertex3f(edge->v0.x, edge->v0.y, edge->v0.z);
		glVertex3f(edge->v1.x, edge->v1.y, edge->v1.z);
		edge = edge->next;
	}
	
	glEnd();
	
	#endif
		
}



void bsp_DrawPolygons()
{	
	#ifdef DRAW_WORLD_POLYGONS
	
	bsp_polygon_t *polygon;
	camera_t *active_camera = camera_GetActiveCamera();
	vec3_t center;
	vec3_t v;
	int i;
	int vert_count;
	
	if(!world_polygons_debug)
		return;
		
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	glUseProgram(0);	
		
	
	polygon = world_polygons_debug;
	
	glLineWidth(4.0);
	glBegin(GL_LINES);
	
	while(polygon)
	{
		
		vert_count = polygon->vert_count;
		
		center.x = 0.0;
		center.y = 0.0;
		center.z = 0.0;
		
		for(i = 0; i < vert_count; i++)
		{
			center.x += polygon->vertices[i].position.x;
			center.y += polygon->vertices[i].position.y;
			center.z += polygon->vertices[i].position.z;
		}
		
		center.x /= vert_count;
		center.y /= vert_count;
		center.z /= vert_count;
		
		#define SCALE 0.9
		
		glColor3f(1.0, 1.0, 1.0);
		for(i = 0; i < vert_count;)
		{
			v.x = polygon->vertices[i % vert_count].position.x - center.x;
			v.y = polygon->vertices[i % vert_count].position.y - center.y;
			v.z = polygon->vertices[i % vert_count].position.z - center.z;
			
			glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
			i++;
			
			v.x = polygon->vertices[i % vert_count].position.x - center.x;
			v.y = polygon->vertices[i % vert_count].position.y - center.y;
			v.z = polygon->vertices[i % vert_count].position.z - center.z;
			
			glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
					
			/*glVertex3f(polygon->vertices[i % vert_count].x, polygon->vertices[i % vert_count].y, polygon->vertices[i % vert_count].z);
			i++;
			glVertex3f(polygon->vertices[i % vert_count].x, polygon->vertices[i % vert_count].y, polygon->vertices[i % vert_count].z);*/
		}
		
		glColor3f(fabs(polygon->normal.x), fabs(polygon->normal.y), fabs(polygon->normal.z));
		glVertex3f(center.x, center.y, center.z);
		glVertex3f(center.x + polygon->normal.x, center.y + polygon->normal.y, center.z + polygon->normal.z);
		
		
		polygon = polygon->next;
	}
	
	glEnd();
	glLineWidth(1.0);
	#endif	
		
}


#define NUDGE_AMOUNT 0.005

int bsp_IntersectBsp(bsp_node_t *node, vec3_t start, vec3_t end)
{
	bsp_leaf_t *leaf;
	float d0;
	float d1;
	float frac;
	vec3_t mid;
	vec3_t vec;
	
	if(!node)
		return 0;
		
	
	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;
		
		if(leaf->bm_flags & BSP_SOLID)
			return 1;
		else	
			return 0;
	}
	else
	{
		
		vec.x = start.x - node->splitter->vertices[0].position.x + node->splitter->normal.x * NUDGE_AMOUNT;
		vec.y = start.y - node->splitter->vertices[0].position.y + node->splitter->normal.y * NUDGE_AMOUNT;
		vec.z = start.z - node->splitter->vertices[0].position.z + node->splitter->normal.z * NUDGE_AMOUNT;
		
		d0 = dot3(vec, node->splitter->normal);
		
		
		vec.x = end.x - node->splitter->vertices[0].position.x - node->splitter->normal.x * NUDGE_AMOUNT;
		vec.y = end.y - node->splitter->vertices[0].position.y - node->splitter->normal.y * NUDGE_AMOUNT;
		vec.z = end.z - node->splitter->vertices[0].position.z - node->splitter->normal.z * NUDGE_AMOUNT;
		
		d1 = dot3(vec, node->splitter->normal);
		
		
		
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
















