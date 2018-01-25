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
		
		/*if(p->approx_area < 0.0001)
			goto _remove;*/

		n = p;
		p = p->next;
	}
	
/*	if(s)
	{
		*portals = s;	
	}*/
	
	
	
	#if 1
	p = *portals;
	
	while(p)
	{
		
		biggest = p;
		
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
		
		//if(p->leaf0 && p->leaf1)
		{
			leaf = p->leaf0;
			leaf->portals[leaf->portal_count++] = p;
			
			assert(leaf->portal_count < MAX_PORTALS_PER_LEAF);
			
			leaf = p->leaf1;
			leaf->portals[leaf->portal_count++] = p;
			
			assert(leaf->portal_count < MAX_PORTALS_PER_LEAF);
		}
		
		
		
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
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, y_max, 0.0), vec3(0.0, -1.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3(0.0, y_max, 0.0), vec3(0.0, -1.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, y_max, 0.0), vec3(0.0, -1.0, 0.0));
		}
			
		
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, y_min, 0.0), vec3(0.0, 1.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3(0.0, y_min, 0.0), vec3(0.0, 1.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, y_min, 0.0), vec3(0.0, 1.0, 0.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(x_max, 0.0, 0.0), vec3(-1.0, 0.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3(x_max, 0.0, 0.0), vec3(-1.0, 0.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(x_max, 0.0, 0.0), vec3(-1.0, 0.0, 0.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(x_min, 0.0, 0.0), vec3(1.0, 0.0, 0.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3(x_min, 0.0, 0.0), vec3(1.0, 0.0, 0.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(x_min, 0.0, 0.0), vec3(1.0, 0.0, 0.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, 0.0, z_max), vec3(0.0, 0.0, -1.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3(0.0, 0.0, z_max), vec3(0.0, 0.0, -1.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, 0.0, z_max), vec3(0.0, 0.0, -1.0));
		}
			
			
		if(bsp_ClassifyPolygon(p->portal_polygon, vec3(0.0, 0.0, z_min), vec3(0.0, 0.0, 1.0)) == POLYGON_STRADDLING)
		{
			bsp_SplitPolygon(p->portal_polygon, vec3(0.0, 0.0, z_min), vec3(0.0, 0.0, 1.0), &front, &back);
			free(p->portal_polygon->vertices);
			free(p->portal_polygon);
			free(back->vertices);
			free(back);		
			p->portal_polygon = front;
			//bsp_TrimPolygon(p->portal_polygon, vec3(0.0, 0.0, z_min), vec3(0.0, 0.0, 1.0));
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
			t->pass_through0 = -1;
			t->pass_through1 = -1;
			t->go_through = -1;
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



#define MAX_CLIP_PLANES 512
#define MAX_OUT_VALID_PORTALS 512

void bsp_RecursivePvsForLeaf(bsp_leaf_t *src_leaf, bsp_portal_t *src_portal, bsp_leaf_t *dst_leaf, bsp_portal_t **in_valid_dst_portals, int in_valid_dst_portal_count)
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
	
	
	
	int out_valid_dst_portal_count;
	bsp_portal_t *valid_portals;
	bsp_portal_t **out_valid_dst_portals;
	bsp_portal_t *dst_portal;
	
	bsp_leaf_t *dst_dst_leaf;
	bsp_portal_t *dst_dst_portal;
	
	
	int clipplane_count;
	bsp_clipplane_t *clipplanes;
	//bsp_clipplane_t clipping_planes[MAX_CLIPPING_PLANES];
	
	bsp_polygon_t *src_portal_polygon;
	bsp_polygon_t *dst_portal_polygon;
	
	bsp_polygon_t *front_split;
	bsp_polygon_t *back_split;
	
	vec3_t v0;
	vec3_t v1;
	vec3_t point;
	vec3_t normal;

	
	if(dst_leaf == src_leaf)
		return;	
		
	src_leaf->pvs[dst_leaf->leaf_index >> 3] |= 1 << (dst_leaf->leaf_index % 8);
	
	//return;
	
	/* this leaf is a terminal leaf... */
	if(!in_valid_dst_portal_count)
		return;
		
	
	if(dst_leaf->pvs[dst_leaf->leaf_index >> 3] & (1 << (dst_leaf->leaf_index % 8)))
		return;
		
	/* mark this leaf on it's own pvs to avoid cyclic recursion chains... */
	dst_leaf->pvs[dst_leaf->leaf_index >> 3] |= 1 << (dst_leaf->leaf_index % 8);	
		
	//return;	
	//printf(">>>leaf %d\n", dst_leaf->leaf_index);
	
	
	src_portal_polygon = src_portal->portal_polygon;
	src_poly_vert_count = src_portal_polygon->vert_count;
	
	/* alloc this stuff in the heap given that deep enough trees
	will actually cause a stack overflow... */
	valid_portals = malloc(sizeof(bsp_portal_t ) * MAX_OUT_VALID_PORTALS);
	out_valid_dst_portals = malloc(sizeof(bsp_portal_t *) * MAX_OUT_VALID_PORTALS);
	clipplanes = malloc(sizeof(bsp_clipplane_t ) * MAX_CLIP_PLANES);
		
	for(i = 0; i < in_valid_dst_portal_count; i++)
	{
		
		clipplane_count = 0;
		out_valid_dst_portal_count = 0;
		
		
		dst_portal = in_valid_dst_portals[i];
		dst_portal_polygon = dst_portal->portal_polygon;
		
		dst_poly_vert_count = dst_portal_polygon->vert_count;
		
		/* skip coplanar portals... */
		switch(bsp_ClassifyPolygon(dst_portal_polygon, src_portal_polygon->vertices[0], src_portal_polygon->normal))
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
				point = src_portal_polygon->vertices[j];
				
				v0.x = point.x - dst_portal_polygon->vertices[k].x;
				v0.y = point.y - dst_portal_polygon->vertices[k].y;
				v0.z = point.z - dst_portal_polygon->vertices[k].z;
				
				
				if(v0.x <= 0.0001 && v0.x >= -0.0001)
				{
					if(v0.y <= 0.0001 && v0.y >= -0.0001)
					{
						if(v0.z <= 0.0001 && v0.z >= -0.0001)
						{
							/* src and dst portals share this vertex, 
							so skip it... */
							continue;
						}
					}
				}
				
				
				v1.x = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].x - dst_portal_polygon->vertices[k].x;
				v1.y = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].y - dst_portal_polygon->vertices[k].y;
				v1.z = dst_portal_polygon->vertices[(k + 1) % dst_poly_vert_count].z - dst_portal_polygon->vertices[k].z;
				
				
				normal = cross(v0, v1);
				normal = normalize3(normal);
				
				/* is this still necessary? */
				if(normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
					continue;
					
					
					
				src_side = bsp_ClassifyPolygon(src_portal_polygon, point, normal);	
				dst_side = bsp_ClassifyPolygon(dst_portal_polygon, point, normal);
				
				cls = src_side | dst_side;
				
				switch(cls)
				{
					case POLYGON_FRONT | POLYGON_BACK:
					//case POLYGON_FRONT | POLYGON_STRADDLING:
					//case POLYGON_BACK  | POLYGON_STRADDLING:
						
					/*	if(cls == (POLYGON_FRONT | POLYGON_STRADDLING) || cls == (POLYGON_BACK | POLYGON_STRADDLING))
						{
							if(dst_side == POLYGON_STRADDLING)
								break;
						}*/
						
						clipplanes[clipplane_count].point = point;
						clipplanes[clipplane_count].normal = normal;
						clipplane_count++;
					
					break;
				}
			}
		}
		
		/*if(clipplane_count < dst_portal_polygon->vert_count)
			continue;*/
		
		
		/*if(!clipplane_count)
			printf("no clip planes!\n");
		else 
			printf("clip planes!\n");*/
			
		/*if(!clipplane_count)
			continue;	
		
		if(clipplane_count < 3)
			continue;*/
			
		//assert(clipplane_count > 2);	
		
		if(dst_portal->leaf0 == dst_leaf)
		{
			dst_dst_leaf = dst_portal->leaf1;
		}
		else
		{
			dst_dst_leaf = dst_portal->leaf0;
		}
		
		/* skip this leaf if it's already in the pvs... */
		/*if(src_leaf->pvs[dst_dst_leaf->leaf_index >> 3] & (1 << (dst_dst_leaf->leaf_index % 8)))
			continue;*/
		
		/* skip this leaf if we recursed through it and didn't return yet... */
		if(dst_dst_leaf->pvs[dst_dst_leaf->leaf_index >> 3] & (1 << (dst_dst_leaf->leaf_index % 8)))
			continue;
		
		dst_dst_leaf_portal_count = dst_dst_leaf->portal_count;
		
		
		for(j = 0; j < dst_dst_leaf_portal_count; j++)
		{
			dst_dst_portal = dst_dst_leaf->portals[j];
			
			/* skip coplanar portals... */
			switch(bsp_ClassifyPolygon(dst_dst_portal->portal_polygon, dst_portal->portal_polygon->vertices[0], dst_portal->portal_polygon->normal))
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
			if(dst_dst_portal->leaf0 == dst_dst_leaf)
				if(dst_dst_portal->leaf1->pvs[dst_dst_portal->leaf1->leaf_index >> 3] & (1 << (dst_dst_portal->leaf1->leaf_index % 8)))
					continue;
				
			
			/* skip this portal if it leads to a leaf
			we recursed through and didn't return from yet... */		
			if(dst_dst_portal->leaf1 == dst_dst_leaf)
				if(dst_dst_portal->leaf0->pvs[dst_dst_portal->leaf0->leaf_index >> 3] & (1 << (dst_dst_portal->leaf0->leaf_index % 8)))
					continue;
					
			/* HACK HACK HACK -- skip this portal if we already went through it... */
			if(dst_dst_portal->go_through == src_leaf->leaf_index)
				continue;		
			
			
			/* skip this portal if it leads to the source leaf... */				
			if(dst_dst_portal->leaf0 == src_leaf || dst_dst_portal->leaf1 == src_leaf)
				continue;
				
			
			valid_portals[out_valid_dst_portal_count].leaf0 = dst_dst_portal->leaf0;
			valid_portals[out_valid_dst_portal_count].leaf1 = dst_dst_portal->leaf1;
			valid_portals[out_valid_dst_portal_count].portal_polygon = bsp_DeepCopyPolygon(dst_dst_portal->portal_polygon);
			
			for(k = 0; k < clipplane_count; k++)
			{
				src_side = bsp_ClassifyPolygon(src_portal_polygon, clipplanes[k].point, clipplanes[k].normal);
				dst_side = bsp_ClassifyPolygon(valid_portals[out_valid_dst_portal_count].portal_polygon, clipplanes[k].point, clipplanes[k].normal);
				
				if(src_side == dst_side || (dst_side & (POLYGON_CONTAINED_BACK | POLYGON_CONTAINED_FRONT)))
				{
					free(valid_portals[out_valid_dst_portal_count].portal_polygon->vertices);
					free(valid_portals[out_valid_dst_portal_count].portal_polygon);
					break;
				}
				
				if(dst_side == POLYGON_STRADDLING)
				{
					bsp_SplitPolygon(valid_portals[out_valid_dst_portal_count].portal_polygon, clipplanes[k].point, clipplanes[k].normal, &front_split, &back_split);
					
					free(valid_portals[out_valid_dst_portal_count].portal_polygon->vertices);
					free(valid_portals[out_valid_dst_portal_count].portal_polygon);
					
					if(src_side == POLYGON_FRONT)
					{
						free(front_split->vertices);
						free(front_split);
						valid_portals[out_valid_dst_portal_count].portal_polygon = back_split;
					}
					else
					{
						assert(src_side == POLYGON_BACK);
						
						free(back_split->vertices);
						free(back_split);
						valid_portals[out_valid_dst_portal_count].portal_polygon = front_split;
					}
					
				}
					
					
			}
			
			if(k >= clipplane_count)
			{
				valid_portals[out_valid_dst_portal_count].go_through = src_leaf->leaf_index;
				out_valid_dst_portal_count++;
			}
				
					
		}
		
		for(j = 0; j < out_valid_dst_portal_count; j++)
		{
			out_valid_dst_portals[j] = &valid_portals[j];
		}
		
		bsp_RecursivePvsForLeaf(src_leaf, src_portal, dst_dst_leaf, out_valid_dst_portals, out_valid_dst_portal_count);
		
		for(j = 0; j < out_valid_dst_portal_count; j++)
		{
			free(valid_portals[j].portal_polygon->vertices);
			free(valid_portals[j].portal_polygon);
		}
		
	}
	
	dst_leaf->pvs[dst_leaf->leaf_index >> 3] &= ~(1 << (dst_leaf->leaf_index % 8));
	
	_bail:
	
	free(valid_portals);
	free(out_valid_dst_portals);
	free(clipplanes);	
	
	/* clear this leaf's bit from it's own pvs, to signal we're done recursing
	through this leaf, and it's safe to go through it again if so is needed... */
	
	//printf("<<<leaf %d\n", dst_leaf->leaf_index);
	
}


void bsp_PvsForLeaf(bsp_leaf_t *leaf)
{
	int i;
	int c;
	
	int j;
	int k;
	
	int leaf_index;
	bsp_portal_t **portals;
	bsp_leaf_t *dst_leaf;
	
	int in_dst_portal_count = 0;
	bsp_portal_t *in_dst_portals[512];	
	
	c = leaf->portal_count;
	
	portals = leaf->portals;
	
	printf("leaf %d\n", leaf->leaf_index);
	
	/*for(i = 0; i < leaf->pvs_size; i++)
	{
		leaf->pvs[i] = 0;
	}*/
	
	
	for(i = 0; i < c; i++)
	{
		//portal_clip_plane_count = 0;
		
		if(portals[i]->leaf0 != leaf)
		{
			dst_leaf = portals[i]->leaf0;
		}
		else
		{
			dst_leaf = portals[i]->leaf1;
		}
		
		/* add the leaf connected to this portal to the
		src leaf's pvs, and recurse down... */
		//leaf_index = dst_leaf->leaf_index;
		//leaf->pvs[leaf_index >> 3] |= 1 << (leaf_index % 8);
		
		in_dst_portal_count = 0;
		k = dst_leaf->portal_count;
		
		for(j = 0; j < k; j++)
		{
			if(dst_leaf->portals[j] != portals[i])
			{
				in_dst_portals[in_dst_portal_count] = dst_leaf->portals[j];
				
				//printf("%x\n", in_dst_portals[in_dst_portal_count]);
				
				in_dst_portal_count++;
				
			}
		}
		
		printf("dst_leaf: %d\n", dst_leaf->leaf_index);
		
		
		bsp_RecursivePvsForLeaf(leaf, portals[i], dst_leaf, in_dst_portals, in_dst_portal_count);
	
	}
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
			//leaf0->pvs_size = pvs_size;	
			for(i = 0; i < pvs_size; i++)
			{
				leaf0->pvs[i] = 0;
			}
		}
		
		if(!leaf1->pvs)
		{
			leaf1->pvs = malloc(pvs_size);
			//leaf1->pvs_size = pvs_size;
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
					
					if(polygons->vertices[i].x > dst_maxs.x) dst_maxs.x = polygons->vertices[i].x;
					if(polygons->vertices[i].y > dst_maxs.y) dst_maxs.y = polygons->vertices[i].y;
					if(polygons->vertices[i].z > dst_maxs.z) dst_maxs.z = polygons->vertices[i].z;
					
					
					if(polygons->vertices[i].x < dst_mins.x) dst_mins.x = polygons->vertices[i].x;
					if(polygons->vertices[i].y < dst_mins.y) dst_mins.y = polygons->vertices[i].y;
					if(polygons->vertices[i].z < dst_mins.z) dst_mins.z = polygons->vertices[i].z;
					
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
	bsp_leaf_t *leaf;
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
					center.x += polygons->vertices[i].x;
					center.y += polygons->vertices[i].y;
					center.z += polygons->vertices[i].z;
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
	}
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
		
		glColor3f(fabs(poly->normal.x), fabs(poly->normal.y), fabs(poly->normal.z));
		glVertex3f(center.x, center.y, center.z);
		glVertex3f(center.x + poly->normal.x, center.y + poly->normal.y, center.z + poly->normal.z);

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

















