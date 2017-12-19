#include <stdio.h>
#include <stdlib.h>

#include "GL\glew.h"

#include "camera.h"
#include "bsp.h"
#include "bsp_cmp.h"
#include "pvs.h"


/* from world.c */
extern int world_nodes_count;
extern bsp_pnode_t *world_nodes;
extern bsp_polygon_t *node_polygons;			/* necessary to quickly build portals... */
extern int world_leaves_count;
extern bsp_dleaf_t *world_leaves;


bsp_portal_t *world_portals = NULL;

int bsp_ClassifyPortalVertex(bsp_pnode_t *node, vec3_t point)
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


int bsp_ClassifyPortal(bsp_pnode_t *node, bsp_portal_t *portal)
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
		d = dot3(p->vertices[i], node->normal) - node->dist;
		
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


void bsp_SplitPortal(bsp_pnode_t *node, bsp_portal_t *portal, bsp_portal_t **front, bsp_portal_t **back)
{
	vec3_t clip_vertex;
	
	int front_vertex_count = 0;
	vec3_t front_vertexes[128];
	
	int back_vertex_count = 0;
	vec3_t back_vertexes[128];
	
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
	
	pa = bsp_ClassifyPortalVertex(node, polygon->vertices[0]);
	pb = bsp_ClassifyPortalVertex(node, polygon->vertices[1]);
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
		
		pa = bsp_ClassifyPortalVertex(node, polygon->vertices[i]);
		pb = bsp_ClassifyPortalVertex(node, polygon->vertices[r]);
		
		//assert(pa != POINT_CONTAINED && pb != POINT_CONTAINED);
		
		/* this edge straddles this splitter, so clip it... */
		if((pa | pb) == (POINT_FRONT | POINT_BACK))
		{
			//bsp_ClipEdge(polygon->vertices[i], polygon->vertices[r], point, normal, &clip_vertex, &time);
			
			
			d0 = dot3(polygon->vertices[i], node->normal) - node->dist;
			d1 = dot3(polygon->vertices[r], node->normal) - node->dist;
			
			time = d0 / (d0 - d1);
			
			
			clip_vertex.x = polygon->vertices[i].x * (1.0 - time) + polygon->vertices[r].x * time;
			clip_vertex.y = polygon->vertices[i].y * (1.0 - time) + polygon->vertices[r].y * time;
			clip_vertex.z = polygon->vertices[i].z * (1.0 - time) + polygon->vertices[r].z * time;
			
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
	
	f = malloc(sizeof(bsp_portal_t));
	f->leaf0 = portal->leaf0;
	f->leaf1 = portal->leaf1;
	f->next = NULL;
	
	f->portal_polygon = malloc(sizeof(bsp_polygon_t));	
	f->portal_polygon->normal = polygon->normal;
	f->portal_polygon->vert_count = front_vertex_count;
	f->portal_polygon->vertices = malloc(sizeof(vec3_t) * (front_vertex_count));
	f->portal_polygon->next = NULL;
	
	for(i = 0; i < front_vertex_count; i++)
	{
		f->portal_polygon->vertices[i] = front_vertexes[i];
	}
	
	
	b = malloc(sizeof(bsp_portal_t ));
	b->leaf0 = portal->leaf0;
	b->leaf1 = portal->leaf1;
	b->next = NULL;
	
	b->portal_polygon = malloc(sizeof(bsp_polygon_t));
	b->portal_polygon->normal = polygon->normal;
	b->portal_polygon->vert_count = back_vertex_count;
	b->portal_polygon->vertices = malloc(sizeof(vec3_t) * (back_vertex_count));
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
					if(polygon->vertices[i].x > maxs->x) maxs->x = polygon->vertices[i].x;
					if(polygon->vertices[i].y > maxs->y) maxs->y = polygon->vertices[i].y;
					if(polygon->vertices[i].z > maxs->z) maxs->z = polygon->vertices[i].z;
					
					if(polygon->vertices[i].x < mins->x) mins->x = polygon->vertices[i].x;
					if(polygon->vertices[i].y < mins->y) mins->y = polygon->vertices[i].y;
					if(polygon->vertices[i].z < mins->z) mins->z = polygon->vertices[i].z;
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
	
	/*if(node->child[0] == BSP_SOLID_LEAF)
	{
		free(portal->portal_polygon->vertices);
		free(portal->portal_polygon);
		free(portal);
		return NULL;
	}
	else
	{
		if(node->child[0] == BSP_EMPTY_LEAF)
		{
			leaf_index = *(int *)&node->dist;
			
			if(!portal->leaf0)
			{
				portal->leaf0 = &world_leaves[leaf_index];
			}
			else
			{
				portal->leaf1 = &world_leaves[leaf_index];
			}
			portal->next = NULL;
			
			return portal;
		}*/
		
	if(bsp->type == BSP_LEAF)
	{
		if(bsp->bm_flags & BSP_SOLID)
		{
			free(portal->portal_polygon->vertices);
			free(portal->portal_polygon);
			free(portal);
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
		//switch(bsp_ClassifyPortal(node, portal))
		switch(bsp_ClassifyPolygon(portal->portal_polygon, bsp->point, bsp->normal))
		{
			//case PORTAL_FRONT:
			case POLYGON_FRONT:
				//return bsp_ClipPortalToBsp(&world_nodes[node->child[0]], portal);
				//return bsp_ClipPortalToBsp(node + node->child[0], portal);
				return bsp_ClipPortalToBsp(bsp->front, portal);
			break;
				
				
			//case PORTAL_BACK:
			case POLYGON_BACK:
					//return bsp_ClipPortalToBsp(&world_nodes[node->child[1]], portal);
				//return bsp_ClipPortalToBsp(node + node->child[1], portal);
				bsp_ClipPortalToBsp(bsp->back, portal);
			break;
				
				
			//case PORTAL_STRADDLING:
			case POLYGON_STRADDLING:
					
					front = malloc(sizeof(bsp_portal_t));
					front->leaf0 = portal->leaf0;
					front->leaf1 = portal->leaf1;
					front->next = NULL;
					
					
					back = malloc(sizeof(bsp_portal_t));
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
					
				free(portal->portal_polygon->vertices);
				free(portal->portal_polygon);
				free(portal);
				
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
				//f = bsp_ClipPortalToBsp(&world_nodes[node->child[0]], portal);
				//f = bsp_ClipPortalToBsp(node + node->child[0], portal);
				f = bsp_ClipPortalToBsp(bsp->front, portal);
				r = f;
					
				while(r)
				{
					n = r->next;
					r->next = NULL;
							
						//back = bsp_ClipPortalToBsp(&world_nodes[node->child[1]], r);
					//back = bsp_ClipPortalToBsp(node + node->child[1], r);
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
	
	//printf("%d %d\n", t_count, c_count);
	
	*portals = t;
}

void bsp_RemoveBadPortals(bsp_portal_t **portals)
{
	bsp_portal_t *p = NULL;
	bsp_portal_t *prev_p = NULL;
	bsp_portal_t *n = NULL;
	bsp_portal_t *prev_n = NULL;
	bsp_portal_t *s;
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
	
	int i;
	
	p = *portals;
	while(p)
	{
		p = p->next;
	}
	
	
	p = *portals;
	while(p)
	{
		/* bad portal... */
		if(!p->leaf0 || !p->leaf1)
		{
			if(n)
			{
				n->next = p->next;
			}
			else
			{
				n = p->next;
				s = n;
			}
			
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(p);
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
			center.x += polygon->vertices[i].x;
			center.y += polygon->vertices[i].y;
			center.z += polygon->vertices[i].z;
		}
		
		center.x /= polygon->vert_count;
		center.y /= polygon->vert_count;
		center.z /= polygon->vert_count;
		
		v0.x = center.x - polygon->vertices[0].x;
		v0.y = center.y - polygon->vertices[0].y;
		v0.z = center.z - polygon->vertices[0].z;
		
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
			v.x = center.x - polygon->vertices[i].x;
			v.y = center.y - polygon->vertices[i].y;
			v.z = center.z - polygon->vertices[i].z;
			
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
		p->approx_area = (x_max - x_min) * (y_max - y_min);
		
		n = p;
		p = p->next;
	}
	
	*portals = s;
	
	#if 0
	p = s;
	
	while(p)
	{
		
		biggest = p;
		
		n = p->next;
		prev_n = p;
		while(n)
		{
			/* duplicated... */
			if((p->leaf0 == n->leaf0 && p->leaf1 == n->leaf1) ||
			   (p->leaf1 == n->leaf0 && p->leaf1 == n->leaf0))
			{
				
				prev_n->next = n->next;
				
				if(p->approx_area >= n->approx_area)
				{
					//q = n->next;
					
					free(n->portal_polygon->vertices);
					free(n->portal_polygon);
					//free(n);
					
					//n = q;
					//continue;	
				}
				else
				{
					
					free(p->portal_polygon->vertices);
					free(p->portal_polygon);
					
					p->portal_polygon = n->portal_polygon;
					p->approx_area = n->approx_area;
					p->leaf0 = n->leaf0;
					p->leaf1 = n->leaf1;
					
					
					
					//n = prev_n->next;
					//continue;
				}
				
				free(n);
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
	
	//*portals = s;
	
	
	
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
	
	
	
	x_max += 2.0;
	y_max += 2.0;
	z_max += 2.0;
	
	x_min -= 2.0;
	y_min -= 2.0;
	z_min -= 2.0;
		
	while(p)
	//while(0)
	{
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, y_max, 0.0), vec3(0.0, -1.0, 0.0)) == POLYGON_STRADDLING)
		{
			/*bsp_SplitPolygon(p->portal_polygon, vec3(0.0, y_max, 0.0), vec3(0.0, -1.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;*/
			bsp_TrimPolygon(p->portal_polygon, vec3(0.0, y_max, 0.0), vec3(0.0, -1.0, 0.0));
		}
			
		
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, y_min, 0.0), vec3(0.0, 1.0, 0.0)) == POLYGON_STRADDLING)
		{
			/*bsp_SplitPolygon(p->portal_polygon, vec3(0.0, y_min, 0.0), vec3(0.0, 1.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;*/
			bsp_TrimPolygon(p->portal_polygon, vec3(0.0, y_min, 0.0), vec3(0.0, 1.0, 0.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(x_max, 0.0, 0.0), vec3(-1.0, 0.0, 0.0)) == POLYGON_STRADDLING)
		{
			/*bsp_SplitPolygon(p->portal_polygon, vec3(x_max, 0.0, 0.0), vec3(-1.0, 0.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;*/
			bsp_TrimPolygon(p->portal_polygon, vec3(x_max, 0.0, 0.0), vec3(-1.0, 0.0, 0.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(x_min, 0.0, 0.0), vec3(1.0, 0.0, 0.0)) == POLYGON_STRADDLING)
		{
			/*bsp_SplitPolygon(p->portal_polygon, vec3(x_min, 0.0, 0.0), vec3(1.0, 0.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;*/
			bsp_TrimPolygon(p->portal_polygon, vec3(x_min, 0.0, 0.0), vec3(1.0, 0.0, 0.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, 0.0, z_max), vec3(0.0, 0.0, -1.0)) == POLYGON_STRADDLING)
		{
			/*bsp_SplitPolygon(p->portal_polygon, vec3(0.0, 0.0, z_max), vec3(0.0, 0.0, -1.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;*/
			bsp_TrimPolygon(p->portal_polygon, vec3(0.0, 0.0, z_max), vec3(0.0, 0.0, -1.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, 0.0, z_min), vec3(0.0, 0.0, 1.0)) == POLYGON_STRADDLING)
		{
			/*bsp_SplitPolygon(p->portal_polygon, vec3(0.0, 0.0, z_min), vec3(0.0, 0.0, 1.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;*/
			bsp_TrimPolygon(p->portal_polygon, vec3(0.0, 0.0, z_min), vec3(0.0, 0.0, 1.0));
		}
			
			
			
		p = p->next;
	}
	
	
}


void bsp_LinkPortalAndLeaves(bsp_dleaf_t *leaves, bsp_portal_t *portals)
{
	
}


//void bsp_GeneratePortals(bsp_pnode_t *nodes, bsp_polygon_t *portal_polygons, bsp_portal_t **portals)
void bsp_GeneratePortals(bsp_node_t *bsp, bsp_portal_t **portals)
{
	bsp_portal_t *portal_list = NULL;
	bsp_portal_t *t = NULL;
	bsp_polygon_t *polygon;
	bsp_polygon_t *next;
	
	bsp_BuildNodePolygons(bsp, &polygon);
	
	
	if(polygon)
	{
		while(polygon)
		{
			next = polygon->next;
		
			t = malloc(sizeof(bsp_portal_t));
			t->leaf0 = NULL;
			t->leaf1 = NULL;
			t->portal_polygon = polygon;
			t->approx_area = -1.0;
			polygon->next = NULL;
			
			t->next = portal_list;
			portal_list = t;
			
			polygon = next;
		}
	}
	
	bsp_ClipPortalsToBounds(bsp, portal_list);
	bsp_ClipPortalsToBsp(bsp, &portal_list);
	bsp_RemoveBadPortals(&portal_list);
	bsp_LinkBack(portal_list);
	
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
	
	polygon = malloc(sizeof(bsp_polygon_t));
	polygon->vertices = malloc(sizeof(vec3_t) * 4);
	
	/* make sure we have an orthonormal basis... */
	v0 = cross(root->tangent, root->normal);
	v1 = cross(v0, root->normal);
	
	vertex.x = root->point.x - v0.x * NODE_POLYGON_SCALE + v1.x * NODE_POLYGON_SCALE; 
	vertex.y = root->point.y - v0.y * NODE_POLYGON_SCALE + v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z - v0.z * NODE_POLYGON_SCALE + v1.z * NODE_POLYGON_SCALE;
	
	polygon->vertices[3] = vertex;
	
	vertex.x = root->point.x - v0.x * NODE_POLYGON_SCALE - v1.x * NODE_POLYGON_SCALE; 
	vertex.y = root->point.y - v0.y * NODE_POLYGON_SCALE - v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z - v0.z * NODE_POLYGON_SCALE - v1.z * NODE_POLYGON_SCALE;
	
	polygon->vertices[2] = vertex;
	
	vertex.x = root->point.x + v0.x * NODE_POLYGON_SCALE - v1.x * NODE_POLYGON_SCALE; 
	vertex.y = root->point.y + v0.y * NODE_POLYGON_SCALE - v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z + v0.z * NODE_POLYGON_SCALE - v1.z * NODE_POLYGON_SCALE;
	
	polygon->vertices[1] = vertex;
	
	vertex.x = root->point.x + v0.x * NODE_POLYGON_SCALE + v1.x * NODE_POLYGON_SCALE; 
	vertex.y = root->point.y + v0.y * NODE_POLYGON_SCALE + v1.y * NODE_POLYGON_SCALE;
	vertex.z = root->point.z + v0.z * NODE_POLYGON_SCALE + v1.z * NODE_POLYGON_SCALE;
	
	polygon->vertices[0] = vertex;
	
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
		free(portals->portal_polygon->vertices);
		free(portals->portal_polygon);
		free(portals);
		portals = n;
	}
}



void bsp_PvsForLeaf(bsp_leaf_t *leaf)
{
	int i;
	int c;
	int leaf_index;
	bsp_portal_t **portals;
	c = leaf->portal_count;
	
	portals = leaf->portals;
	
	for(i = 0; i < c; i++)
	{
		if(portals[i]->leaf0 != leaf)
		{
			leaf_index = portals[i]->leaf0->leaf_index;
		}
		else
		{
			leaf_index = portals[i]->leaf1->leaf_index;
		}
		
		leaf->pvs[leaf_index >> 3] |= 1 << (leaf_index % 8);
	}
}

void bsp_RecursivePvsForLeaf(bsp_leaf_t *src, bsp_leaf_t *dst)
{
	
}

void bsp_RecursivePvsForLeaves(bsp_node_t *bsp)
{
	bsp_leaf_t *leaf;
	
	if(bsp->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)bsp;
		if(!(leaf->bm_flags & BSP_SOLID))
		{
			bsp_PvsForLeaf(leaf);
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
	}	
	
	bsp_RecursivePvsForLeaves(bsp);
		
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
	//c = leaf_count;
	
	//pvs_size = 1 + c >> 5;
	
	bsp_GeneratePortals(bsp, &world_portals);	
	bsp_PvsForLeaves(bsp, world_portals);
	//bsp_CountNodesAndLeaves(bsp, &leaf_count, &node_count);
	//pvs_size = 4 + (leaf_count >> 3);
	
	
	
	
	/*portals = world_portals;
	
	while(portals)
	{
		
		leaf0 = portals->leaf0;
		leaf1 = portals->leaf1;
			
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
				
		portals = portals->next;
	}*/
	
	
	/*for(i = 0; i < c; i++)
	{
		leaves[i].pvs = malloc(sizeof(int) * (pvs_size + 1));
		
		#ifndef EMPTY_PVS
		
		for(j = 0; j < c; j++)
		{
			if(i == j)
			{
				leaves[i].pvs[j >> 5] &= ~(1 << (j % 32));
				continue;
			}		
			
			bsp_FirstHit(world_nodes, leaves[i].center, leaves[j].center, &trace);
			if(trace.frac == 1.0)
			{
				leaves[i].pvs[j >> 5] |= 1 << (j % 32);
				continue;
			}
						
			leaves[i].pvs[j >> 5] &= ~(1 << (j % 32));
		
		}
		
		#else
		
		for(j = 0; j < pvs_size; j++)
		{
			leaves[i].pvs[j] = 0;
		}
		
		#endif
		
	}*/
	
	/*bsp_DeletePortals(world_portals);
	world_portals = NULL;*/
	
}


void bsp_DrawPortals()
{
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
		
		//if(p->leaf0 && p->leaf1)
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
				center.x += poly->vertices[i].x;
				center.y += poly->vertices[i].y;
				center.z += poly->vertices[i].z;
			}
			
			center.x /= c;
			center.y /= c;
			center.z /= c;
						
			
			#define SCALE 0.95
			
			for(i = 0; i < c;)
			{	
			
				v.x = (poly->vertices[i%c].x - center.x) * SCALE;
				v.y = (poly->vertices[i%c].y - center.y) * SCALE;
				v.z = (poly->vertices[i%c].z - center.z) * SCALE;
						
				glVertex3f(center.x + v.x, center.y + v.y, center.z + v.z);
				i++;
				
				v.x = (poly->vertices[i%c].x - center.x) * SCALE;
				v.y = (poly->vertices[i%c].y - center.y) * SCALE;
				v.z = (poly->vertices[i%c].z - center.z) * SCALE;
				
				glVertex3f(center.x + v.x, center.y + v.y, center.z + v.z);
			}
		}
		
		

		p = p->next;
	}	
	
	glEnd();
	glLineWidth(1.0);
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

















