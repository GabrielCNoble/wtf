#include <stdio.h>
#include <stdlib.h>
#include "bsp.h"
#include "bsp_cmp.h"
#include "pvs.h"
#include "world.h"
#include "camera.h"

#include <float.h>

#include <fenv.h>

#include "SDL2\SDL.h"
#include "GL\glew.h"



/* from brush.c */
extern int brush_count;
extern brush_t *brushes;
extern brush_t *expanded_brushes;

/* from light.c */
extern int visible_light_count;

/* from brush.c */
extern vec3_t cube_bmodel_collision_verts[];
extern vec3_t cube_bmodel_collision_normals[];

/* from world.c */
bsp_node_t *world_bsp;
extern int world_vertices_count;
extern vertex_t *world_vertices;
extern int world_triangle_group_count;
extern triangle_group_t *world_triangle_groups;
extern int world_nodes_count;
extern bsp_pnode_t *world_nodes;
extern bsp_pnode_t *collision_nodes;
//extern bsp_polygon_t *node_polygons;			/* necessary to quickly build portals... */
extern int world_leaves_count;
extern bsp_dleaf_t *world_leaves;
extern int visited_leaves_count;
extern bsp_dleaf_t *visited_leaves;

bsp_polygon_t *node_polygons = NULL;			/* necessary to quickly build portals... */

static SDL_Thread *bsp_build_thread;


bsp_polygon_t *beveled_polygons = NULL;
bsp_edge_t *bevel_edges = NULL;

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
		switch(bsp_ClassifyPoint(polygon->vertices[i], point, normal))
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
	vec3_t clip_vertex;
	vec3_t front_back_vertex;
	vec3_t back_front_vertex;
	
	int front_vertex_count = 0;
	vec3_t front_vertexes[128];
	
	int back_vertex_count = 0;
	vec3_t back_vertexes[128];
	
	
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
	
	
	pa = bsp_ClassifyPoint(polygon->vertices[0], point, normal);
	pb = bsp_ClassifyPoint(polygon->vertices[1], point, normal);
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
		
		pa = bsp_ClassifyPoint(polygon->vertices[i], point, normal);
		pb = bsp_ClassifyPoint(polygon->vertices[r], point, normal);
		
		//assert(pa != POINT_CONTAINED && pb != POINT_CONTAINED);
		
		/* this edge straddles this splitter, so clip it... */
		if((pa | pb) == (POINT_FRONT | POINT_BACK))
		{
			bsp_ClipEdge(polygon->vertices[i], polygon->vertices[r], point, normal, &clip_vertex, &time);
			
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
	f->vertices = malloc(sizeof(vec3_t) * (f->vert_count));
	f->b_used = polygon->b_used;
	f->next = NULL;
	
	for(i = 0; i < f->vert_count; i++)
	{
		f->vertices[i] = front_vertexes[i];
	}
	
	
	b = malloc(sizeof(bsp_polygon_t ));
	b->normal = polygon->normal;
	b->vert_count = back_vertex_count;
	b->vertices = malloc(sizeof(vec3_t) * (b->vert_count));
	b->next = NULL;
	b->b_used = polygon->b_used;
	
	for(i = 0; i < b->vert_count; i++)
	{
		b->vertices[i] = back_vertexes[i];
	}
	
		
	*front = f;
	*back = b;
	
}


int bsp_TrimPolygon(bsp_polygon_t *polygon, vec3_t point, vec3_t normal)
{

	vec3_t clip_vertex;	
	int front_vertex_count = 0;
	vec3_t front_vertexes[128];
	
	
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
	
	pa = bsp_ClassifyPoint(polygon->vertices[0], point, normal);
	pb = bsp_ClassifyPoint(polygon->vertices[1], point, normal);
	
	
	

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
		
		pa = bsp_ClassifyPoint(polygon->vertices[i], point, normal);
		pb = bsp_ClassifyPoint(polygon->vertices[r], point, normal);
		
		//assert(pa != POINT_CONTAINED || pb != POINT_CONTAINED);
		
		/* this edge straddles this splitter, so clip it... */
		if((pa | pb) == (POINT_FRONT | POINT_BACK))
		{
			bsp_ClipEdge(polygon->vertices[i], polygon->vertices[r], point, normal, &clip_vertex, &time);
			
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
		polygon->vertices = malloc(sizeof(vec3_t) * front_vertex_count);
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
	bsp_polygon_t *p;
	
	p = malloc(sizeof(bsp_polygon_t));
	
	p->normal = src->normal;
	p->brush_index = src->brush_index;
	p->b_used = src->b_used;
	p->next = src->next;
	p->vert_count = src->vert_count;
	p->vertices = malloc(sizeof(vec3_t) * p->vert_count);
	
	for(i = 0; i < p->vert_count; i++)
	{
		p->vertices[i] = src->vertices[i];
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
		r = malloc(sizeof(bsp_polygon_t ));
		
		r->brush_index = src->brush_index;
		r->b_used = src->b_used;
		r->normal = src->normal;
		r->vert_count = src->vert_count;
		r->vertices = malloc(sizeof(vec3_t) * r->vert_count);
		
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


/*
==============
bsp_FindSplitter
==============
*/
bsp_polygon_t *bsp_FindSplitter(bsp_polygon_t *polygons, int ignore_used)
{
	bsp_polygon_t *cur_splitter = polygons;
	bsp_polygon_t *r;
	
	bsp_polygon_t *min_splitter = NULL;
	unsigned int min_split_count = 0xffffffff;
	unsigned int split_count = 0;
	int i;
	
	while(cur_splitter)
	{
		split_count = 0;
		
		/* skip polygons that have already been used as a splitter... */
		if(ignore_used)
		{
			if(cur_splitter->b_used)
			{
				cur_splitter = cur_splitter->next;
				continue;
			}	
		}
		
		
		
		r = polygons;
		
		while(r)
		{
			if(r == cur_splitter)
			{
				r = r->next;
				continue;	
			}
					
			if(bsp_ClassifyPolygon(r, cur_splitter->vertices[0], cur_splitter->normal) == POLYGON_STRADDLING) split_count++;
			r = r->next;
		}
		
		if(split_count < min_split_count)
		{
			min_split_count = split_count;
			min_splitter = cur_splitter;
		}
		
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
			it's considered straddling... */ 
			cur_splitter = polygons;
			while(cur_splitter)
			{
				if(cur_splitter == min_splitter)
				{
					cur_splitter = cur_splitter->next;
					continue;
				}
				
				i = bsp_ClassifyPolygon(cur_splitter, min_splitter->vertices[0], min_splitter->normal);
					
				if(i == POLYGON_CONTAINED_FRONT || i == POLYGON_CONTAINED_BACK)
				{
					cur_splitter->b_used = 1;
				}
				
				
				cur_splitter = cur_splitter->next;
			}
		}
		
	}
	
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
		p->vertices = malloc(sizeof(vec3_t) * vertex_count);
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
	vec3_t p;
	
	
	
	for(i = 0; i < 6; i++)
	{
		
		triangles = NULL;
		
		polygon = malloc(sizeof(bsp_polygon_t));
		v = cube_bmodel_collision_normals[i];
			
			
			
		p.x = v.x * brush->orientation.floats[0][0] + 
		      v.y * brush->orientation.floats[1][0] +
			  v.z * brush->orientation.floats[2][0];
				  
				  
		p.y = v.x * brush->orientation.floats[0][1] + 
		      v.y * brush->orientation.floats[1][1] +
			  v.z * brush->orientation.floats[2][1];
				  
				  
		p.z = v.x * brush->orientation.floats[0][2] + 
		      v.y * brush->orientation.floats[1][2] +
			  v.z * brush->orientation.floats[2][2];
		
		
		polygon->normal = p;
		
		polygon->vertices = malloc(sizeof(vec3_t) * 4);
		polygon->vert_count = 4;
		
		
		for(c = 0; c < 4; c++)
		{
			v = cube_bmodel_collision_verts[i * 4 + c];
			
			v.x *= brush->scale.x;
			v.y *= brush->scale.y;
			v.z *= brush->scale.z;
			
			
			p.x = v.x * brush->orientation.floats[0][0] + 
			      v.y * brush->orientation.floats[1][0] +
				  v.z * brush->orientation.floats[2][0] + brush->position.x;
				  
				  
			p.y = v.x * brush->orientation.floats[0][1] + 
			      v.y * brush->orientation.floats[1][1] +
				  v.z * brush->orientation.floats[2][1] + brush->position.y;
				  
				  
			p.z = v.x * brush->orientation.floats[0][2] + 
			      v.y * brush->orientation.floats[1][2] +
				  v.z * brush->orientation.floats[2][2] + brush->position.z;	  	  
			
			
			polygon->vertices[c] = p;
		}
		
		#if 0
		
		k = brush->vertex_count;
		
		for(j = 0; j < k;)
		{
			for(l = 0; l < 3; l++)
				tri_verts[l] = brush->vertices[j++];
							
			/* test to see if this triangle is contained in the polygon's plane... */				
			for(l = 0; l < 3; l++)
				if(bsp_ClassifyPoint(tri_verts[l].position, polygon->vertices[0], polygon->normal) != POINT_CONTAINED) break;
				
				
			/* it is, so append this triangle to the triangle list of this polygon... */	
			if(l >= 3)
			{
				triangle = malloc(sizeof(bsp_triangle_t));
				triangle->a = tri_verts[0];
				triangle->b = tri_verts[1];
				triangle->c = tri_verts[2];
				triangle->plane_normal = polygon->normal;
				triangle->b_used = 0;
				
				triangle->next = triangles;
				triangles = triangle;
			}	
			
			
			
		}
		
		
		polygon->triangles = triangles;
		
		#endif
		
		polygon->b_used = 0;
		polygon->next = polygons;
		polygons = polygon;
	}
	
	return polygon;
	
}


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
	brush_t *brush;
	
	int i;
	int c = brush_count;
	int j;
	int k;
	
	
	for(i = 0; i < c; i++)
	{
		
		brush = &brushes[i];
		
		k = brush->vertex_count;
		
		for(j = 0; j < k;)
		{
			r = malloc(sizeof(bsp_triangle_t));
		
			r->material_index = brush->triangle_groups[brush->triangles[j / 3].triangle_group].material_index;
			
			r->a = brush->vertices[j];
			j++;
			
			r->b = brush->vertices[j];
			j++;
			
			r->c = brush->vertices[j];
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
	//bsp_polygon_t *front_list;
	//bsp_polygon_t *back_list;
	//bsp_polygon_t *cur_polygon = polygons;
	
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
	
	/*if(input_GetKeyStatus(SDL_SCANCODE_K) & KEY_PRESSED)
	{
		printf("breakpoint!\n");
		
		printf("breakpoint!\n");
	}*/
	
	
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
			front = malloc(sizeof(bsp_polygon_t ));
			*front = *polygon;
			front->vertices = malloc(sizeof(vec3_t) * front->vert_count);
			c = front->vert_count;
			
			for(i = 0; i < c; i++)
			{
				front->vertices[i] = polygon->vertices[i];
			}
			
			front->next = NULL;
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
		i = bsp_ClassifyPolygon(polygon, bsp->splitter->vertices[0], bsp->splitter->normal);
		
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
				bsp_SplitPolygon(polygon, bsp->splitter->vertices[0], bsp->splitter->normal, &front, &back);
				
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

	
	
	/*if(input_GetKeyStatus(SDL_SCANCODE_K) & KEY_PRESSED)
	{
		printf("breakpoint!\n");
		
		printf("breakpoint!\n");
	}*/
	
	
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
	
	//printf("%x\n", polygon);
	
	/*if(!polygon)
	{
		printf("null return %x\n", polygon);
	}*/
	
	return polygon;	
}


/*
==============
bsp_ClipBrushes
==============
*/
bsp_polygon_t *bsp_ClipBrushes(brush_t *brushes, int brush_count)
{
	
	int i;
	int c;
	//brush_cluster_t *brush_clusters = NULL;
	//brush_cluster_t *cluster = NULL;
	//brush_cluster_t *cur_cluster;
	//brush_cluster_t *r;
	//brush_cluster_t *p;
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
	
	c = brush_count;
	
	
	/*if(input_GetKeyStatus(SDL_SCANCODE_K) & KEY_PRESSED)
	{
		printf("breakpoint!\n");
		
		printf("breakpoint!\n");
	}*/
	
	
	/* to do the union operation between two brushes, it's necessary
	to "push" brush A's polygons through brush B's bsp and vice versa,
	retaining just those polygons that are on the outside. */
	
	/* merging bsps direcly instead of incremental set-op
	might work better...  */
	if(c)
	{
		
		
		/* build the polygon list for the first brush in the list
		(brush A in the first iteration of the loop below)... */
		
		
		/* HACK!! */
		if(!brushes[0].polygons)
			polygons_a = bsp_BuildPolygonsFromBrush(&brushes[0]);
		else
			polygons_a = bsp_DeepCopyPolygons(brushes[0].polygons);
			//polygons_a = brushes[0].polygons;
		
		for(i = 1; i < c; i++)
		{
			
			/* build the polygon list for this brush... */
		//	polygons_b = bsp_BuildPolygonsFromBrush(&brushes[i]);
		
			/* HACK! HACK! HACK! */
			if(!brushes[i].polygons)
				polygons_b = bsp_BuildPolygonsFromBrush(&brushes[i]);
			else
				polygons_b = bsp_DeepCopyPolygons(brushes[i].polygons);
				
				//polygons_b = brushes[i].polygons;
			
			
			bsp_a = NULL;
			/* build the bsp for the polygons of brush A... */
			bsp_BuildSolid(&bsp_a, polygons_a);

			
			
			bsp_b = NULL;
			/* build the bsp for the polygons of brush B... */
			bsp_BuildSolid(&bsp_b, polygons_b);
			
			
			/* clip the polygons that belong to bsp_a against
			bsp_b... */
			polygons_a = bsp_ClipBspToBsp(bsp_b, bsp_a);			
			assert(polygons_a);
			
			/* clip the polygons that belong to bsp_b against
			bsp_a... */
			polygons_b = bsp_ClipBspToBsp(bsp_a, bsp_b);
			
			
			/* get rid of those bsp's, they're 
			useless now. Such useless motherfuckers... */
			bsp_DeleteSolid(bsp_a);
			bsp_DeleteSolid(bsp_b);

			p = polygons_a;
			
			/* link the two lists of polygons... */	
			while(p->next)
			{
				p = p->next;	
			}
			p->next = polygons_b;			
			
			
		}
		
	}
	return polygons_a;	
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
	triangle_group_t *g;
	triangle_group_t *n;
	bsp_triangle_t *triangle;
	bsp_leaf_t *leaf;
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
					
					//if(leaf->triangles[i].material_index == g[k].material_index)
					if(triangle->material_index == g[k].material_index);
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
					
					if(c > 1)
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
					for(k++; k < c - 1; k++)
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

void bsp_RecursiveLinearizeBsp(bsp_node_t *bsp, vertex_t *vertices, int *vertex_count, bsp_pnode_t *lnodes, int *lnode_count, bsp_dleaf_t *lleaves, int *lleaves_count, triangle_group_t *groups, int tri_group_count, int create_leaves)
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
			dleaf->pvs = leaf->pvs;
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
				dleaf->tris[tris_index].first_vertex = groups[i].next;
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
			polygon = leaf->polygons;
			while(polygon)
			{
				next_polygon = polygon->next;
				free(polygon->vertices);
				free(polygon);
				polygon = next_polygon;
			}
			
		}
		else
		{
			
			/* this node points to a solid leaf... */
			pnode->child[0] = BSP_SOLID_LEAF;
			pnode->child[1] = BSP_SOLID_LEAF;
		}
		
		#ifndef LEAK_FOR_FUN
		free(leaf);
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
		bsp_RecursiveLinearizeBsp(bsp->front, vertices, vertex_count, lnodes, lnode_count, lleaves, lleaves_count, groups, tri_group_count, create_leaves);
		
		pnode->child[1] = (*lnode_count) - node_index;
		bsp_RecursiveLinearizeBsp(bsp->back, vertices, vertex_count, lnodes, lnode_count, lleaves, lleaves_count, groups, tri_group_count, create_leaves);
		
		#ifndef LEAK_FOR_FUN
		free(bsp);
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
	
	if(vertex_count)
	{
		*vertex_count = groups[tri_group_count - 1].start + groups[tri_group_count - 1].next;
		v = malloc(sizeof(vertex_t) * (*vertex_count));
	}
		
	
	
	
	bsp_CountNodesAndLeaves(bsp, &l, &n);
	
//	printf("%d %d\n", l, n);
	
	nodes = malloc(sizeof(bsp_pnode_t) * n);
	
	if(lleaves)
		leaves = malloc(sizeof(bsp_dleaf_t) * (l + 200));
	
	i = 0;
	n = 0;
	l = 0;
	c = 0;
	
	for(i = 0; i < tri_group_count; i++)
	{
		groups[i].next = 0;
	}
	
	bsp_RecursiveLinearizeBsp(bsp, v, &i, nodes, &n, leaves, &l, groups, tri_group_count, create_leaves);
	
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
				p0 = p->vertices[i];
				p1 = p->vertices[(i + 1) % c];
				
				k = r->vert_count;
				
				for(j = 0; j < k; j++)
				{
					
					r0 = r->vertices[j];
					r1 = r->vertices[(j + 1) % k];
					
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
						
							
							edge = malloc(sizeof(bsp_edge_t) );
							edge->v0 = p0;
							edge->v1 = p1;
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
	int vert_count;
	float d;
	bsp_edge_t *edges;
	bsp_edge_t *edge;
	bsp_polygon_t *polygons;
	bsp_polygon_t *polygon;
	
	vec3_t polygon_normal;
	vec3_t v;
	vec3_t polygon_center;
	vec3_t r;
	
	
	if(!expanded_brushes)
		expanded_brushes = malloc(sizeof(brush_t) * brush_count);
		
	
	
		
	for(i = 0; i < brush_count; i++)
	{
		//edges = bsp_BuildEdgesFromBrush(&brushes[i]);
		
		expanded_brushes[i] = brushes[i];
		
	//	#if 0
		
		polygons = bsp_BuildPolygonsFromBrush(&brushes[i]);
		edges = bsp_BuildBevelEdges(polygons);
		
		expanded_brushes[i].polygons = polygons;
		
		
		#if 1
		polygon = polygons;
		
		while(polygon)
		{
			
			vert_count = polygon->vert_count;
			polygon_normal = polygon->normal;
			
			d = fabs(dot3(box_extents, polygon_normal));
			
			polygon_center.x = 0.0;
			polygon_center.y = 0.0;
			polygon_center.z = 0.0;
			
			for(j = 0; j < vert_count; j++)
			{
				polygon_center.x += polygon->vertices[j].x;
				polygon_center.y += polygon->vertices[j].y;
				polygon_center.z += polygon->vertices[j].z;
			}
			
			polygon_center.x /= vert_count;
			polygon_center.y /= vert_count;
			polygon_center.z /= vert_count;
			
			
			for(j = 0; j < vert_count; j++)
			{
				v = polygon->vertices[j];
					
				r.x = v.x - polygon_center.x;
				r.y = v.y - polygon_center.y;
				r.z = v.z - polygon_center.z;
				
				r = normalize3(r);
				
				/*r.x -= polygon_normal.x;
				r.y -= polygon_normal.y;
				r.z -= polygon_normal.z;*/
				
				//r = normalize3(r);
				
				v.x += polygon_normal.x * d;
				v.y += polygon_normal.y * d;
				v.z += polygon_normal.z * d;
				
				v.x += r.x * d;
				v.y += r.y * d;
				v.z += r.z * d;
				
				polygon->vertices[j] = v;
						
			}
				
			
			polygon = polygon->next;
		}
		
		#endif
		
	//	#endif
		
		
		
		
		
		//expanded_brushes[i].
		
		
	}
	
}

void bsp_BuildCollisionBsp()
{
	
	bsp_polygon_t *polygons;
	bsp_node_t *root = NULL;
	int c;
	
	bsp_ExpandBrushes(vec3(0.5, 2.0, 0.5));
	
	polygons = bsp_ClipBrushes(expanded_brushes, brush_count);
	
	bsp_BuildSolidLeaf(&root, polygons);
	
	bsp_LinearizeBsp(root, NULL, NULL, &collision_nodes, &c, NULL, NULL, NULL, 0, 0);
	
	
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
void bsp_BuildSolid(bsp_node_t **root, bsp_polygon_t *polygons)
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
	
	bsp_polygon_t *cur_polygon = polygons;
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
	
	p->splitter = bsp_FindSplitter(polygons, 0);
			
	while(cur_polygon)
	{		
		
		r = cur_polygon->next;
					
		if(cur_polygon == p->splitter)
		{			
			cur_polygon = cur_polygon->next;
			continue;
		}
			
		switch(bsp_ClassifyPolygon(cur_polygon, p->splitter->vertices[0], p->splitter->normal))
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

				bsp_SplitPolygon(cur_polygon, p->splitter->vertices[0], p->splitter->normal, &front_split, &back_split);
				
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
		p->back = malloc(sizeof(bsp_leaf_t));
		
		leaf = (bsp_leaf_t *)p->back;
		
		leaf->bm_flags = BSP_SOLID;
		leaf->type = BSP_LEAF;
		leaf->polygons = NULL;
		leaf->triangles = NULL;
	}
	else
	{
		bsp_BuildSolid(&p->back, back_list);
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
	
	bsp_polygon_t *cur_polygon = polygons;
	bsp_polygon_t *r = NULL;
	bsp_polygon_t *front_split = NULL;
	bsp_polygon_t *back_split = NULL;
	
	vec3_t v;
	
	int i;
	int c;
	
	/*bsp_triangle_t *cur_triangle = triangles;
	bsp_triangle_t *p;
	bsp_triangle_t *r;
	bsp_triangle_t *n;*/
	
	bsp_polygon_t *splitter = NULL;
	
	if(!root)
	{
		return;
	}
	splitter = bsp_FindSplitter(polygons, 1);

	if(splitter)
	{
		
		*root = malloc(sizeof(bsp_node_t));
		node = *root;
		
		node->type = BSP_NODE;
		node->bm_flags = 0;
		node->front = NULL;
		node->back = NULL;		
		
		//splitter->b_used = 1;
		
		node->point = splitter->vertices[0];
		node->normal = splitter->normal;
		
		v.x = splitter->vertices[1].x - node->point.x;
		v.y = splitter->vertices[1].y - node->point.y;
		v.z = splitter->vertices[1].z - node->point.z;
		
		v = normalize3(v);
		
		node->tangent = cross(v, node->normal);
			
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
			
			switch(bsp_ClassifyPolygon(cur_polygon, splitter->vertices[0], splitter->normal))
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
	
					bsp_SplitPolygon(cur_polygon, splitter->vertices[0], splitter->normal, &front_split, &back_split);
						
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
			
		if(!polygons)
		{
			leaf->bm_flags |= BSP_SOLID;
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
	bsp_BuildSolid(&root, polygons);
	
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


void bsp_DeleteSolid(bsp_node_t *root)
{
	
	if(root)
	{
		if(root->type != BSP_LEAF)
		{
			bsp_DeleteSolid(root->front);
			bsp_DeleteSolid(root->back);
			free(root->splitter);
		}
		free(root);
	}
}


void bsp_DeleteSolidLeaf(bsp_node_t *root)
{
	#if 0
	bsp_polygon_t *r;
	bsp_polygon_t *t;
	bsp_triangle_t *tri;
	bsp_triangle_t *n;
	bsp_leaf_t *leaf;
	
	if(root)
	{
		//if(root->bm_flags & BSP_NODE_LEAF && (!(root->bm_flags & BSP_NODE_SOLID)))
		if(root->type == BSP_SHORT_LEAF)
		{
			leaf = (bsp_leaf_t *)root;
			
			if(!(leaf->bm_flags & BSP_SOLID))
			{
				free(leaf->tris);			
			}
		}
		else
		{
			bsp_DeleteSolidLeaf(root->front);
			bsp_DeleteSolidLeaf(root->back);
		}
		
		free(root);
	}
	#endif
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
	bsp_node_t *bsp;
	
	
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
	}
	
	if(world_leaves)
	{
		//free(visited_leaves);
		free(world_leaves);
	}
		
	//bsp_polygon_t *polygons = NULL;
	bsp_triangle_t *triangles;
	
	
	printf("bsp_CompileBsp: bsp_ClipBrushes... ");		
	polygons = bsp_ClipBrushes(brushes, brush_count); 
	printf("done\n");
	
	printf("bsp_CompileBsp: bsp_SolidLeafBsp... ");
	bsp = bsp_SolidLeafBsp(polygons);
	printf("done\n");
	
	printf("bsp_CompileBsp: bsp_BuildNodePolygons... ");
	bsp_BuildNodePolygons(bsp, &node_polygons); 
	printf("done\n");
	
	printf("bsp_CompileBsp: bsp_BuildTrianglesFromBrushes... ");
	triangles = bsp_BuildTrianglesFromBrushes();
	printf("done\n");
	
	printf("bsp_CompileBsp: bsp_ClipTrianglesToSolidLeaves... ");
	bsp_ClipTrianglesToSolidLeaves(bsp, triangles);
	printf("done\n");
	
	printf("bsp_CompileBsp: bsp_MergeLeafTriangles... ");
	bsp_MergeLeafTriangles(bsp);
	printf("done\n");
	
	printf("bsp_CompileBsp: bsp_BuildTriangleGroups... ");
	bsp_BuildTriangleGroups(bsp, &world_triangle_groups, &world_triangle_group_count);
	printf("done\n");
	
	
	printf("bsp_CompileBsp: bsp_CalculatePvs...");
	bsp_CalculatePvs(bsp);
	printf("done\n");
	
	//bsp_CalculateApproximatePvs(bsp);
	
	printf("bsp_CompileBsp: bsp_LinearizeBsp... ");
	bsp_LinearizeBsp(bsp, &world_vertices, &world_vertices_count, &world_nodes, &world_nodes_count, &world_leaves, &world_leaves_count, world_triangle_groups, world_triangle_group_count, 1);
	printf("done\n");
	
	
	printf("bsp_CompileBsp: bsp_BuildCollisionBsp...\n");
	bsp_BuildCollisionBsp();
	printf("done\n\n");
	
	
	world_Update();
	
	
	//bsp_BuildEdgesFromBrush(&brushes[0]);
	
	
	/*printf("bsp_CompileBsp: bsp_ShortLeaves... ");
	bsp_ShortLeaves(world_bsp, &world_vertices, &world_vertices_count, global_triangle_groups, global_triangle_group_count);
	printf("done\n\n");*/
/*	bsp_build_thread = SDL_CreateThread(bsp_BuildBspAssync, "build bsp", NULL);
	SDL_DetachThread(bsp_build_thread);*/
	
	
	//bsp_MergeLeafTriangles(world_bsp);
	
	
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
		
	for(i = 0; i < brush_count; i++)
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
		
	}
	
	glEnd();	
	
	glLineWidth(1.0);
	
	glPopMatrix();
	
}

























