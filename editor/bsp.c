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


#if 0
void bsp_DrawBsp(bsp_node_t *root, bsp_node_t *parent, vec3_t camera_position, int level)
{
	
//	#if 0
	vec3_t v;
	bsp_node_t *near;
	bsp_node_t *far;
	
	vec3_t center;
	vec3_t parent_center;
	vec3_t r;
	vec3_t q;
	int i;
	int c;
	float d;
	if(!root) return;
	//if(!root->splitter)
	if(root->type == BSP_LEAF)
	{		
		c = parent->splitter->vert_count;
	
		center = vec3(0.0, 0.0, 0.0);
		
		for(i = 0; i < c; i++)
		{
			center.x += parent->splitter->vertices[i].x;
			center.y += parent->splitter->vertices[i].y;
			center.z += parent->splitter->vertices[i].z;	
		}
		
		center.x /= c;
		center.y /= c;
		center.z /= c;
		
		glBegin(GL_LINES);
			glColor3f(fabs(parent->splitter->normal.x), fabs(parent->splitter->normal.y), fabs(parent->splitter->normal.z));
			glVertex3f(center.x, center.y, center.z);
			glVertex3f(center.x + parent->splitter->normal.x, center.y + parent->splitter->normal.y, center.z + parent->splitter->normal.z);
		glEnd();
			
		return;
	}
	
	level = (level + 1) % 16;
	
	
	
	v.x = camera_position.x - root->splitter->vertices[0].x;
	v.y = camera_position.y - root->splitter->vertices[0].y;
	v.z = camera_position.z - root->splitter->vertices[0].z;
	
	d = dot3(v, root->splitter->normal);
	
	if(d > 0.0)
	{
		near = root->front;
		far = root->back;
	}
	else
	{
		near = root->back;
		far = root->front;
	}
	
	bsp_DrawBsp(far, root, camera_position, level);
		
	c = parent->splitter->vert_count;
	
	parent_center = vec3(0.0, 0.0, 0.0);
	
	for(i = 0; i < c; i++)
	{
		parent_center.x += parent->splitter->vertices[i].x;
		parent_center.y += parent->splitter->vertices[i].y;
		parent_center.z += parent->splitter->vertices[i].z;	
	}
		
	parent_center.x /= c;
	parent_center.y /= c;
	parent_center.z /= c;
	
	
	c = root->splitter->vert_count;
	
	center = vec3(0.0, 0.0, 0.0);
	
	for(i = 0; i < c; i++)
	{
		center.x += root->splitter->vertices[i].x;
		center.y += root->splitter->vertices[i].y;
		center.z += root->splitter->vertices[i].z;
		
	}
	
	center.x /= c;
	center.y /= c;
	center.z /= c;
	
	
	glBegin(GL_LINES);
	
	
	
	for(i = 0; i < c; i++)
	{
		/*glColor4f(0.0, 1.0, 0.0, 1.0);
		glVertex3f(center.x, center.y, center.z);
		glVertex3f(center.x + root->splitter->normal.x, center.y + root->splitter->normal.y, center.z + root->splitter->normal.z);*/
		
		glColor4f(color_table[level][0], color_table[level][1], color_table[level][2], 0.4);
		r = root->splitter->vertices[i % c];	
		q.x = (r.x - center.x) * 1.00;
		q.y = (r.y - center.y) * 1.00;
		q.z = (r.z - center.z) * 1.00;
		glVertex3f(center.x + q.x , center.y + q.y , center.z + q.z);
		
		
		r = root->splitter->vertices[(i + 1) % c];	
		q.x = (r.x - center.x) * 1.00;
		q.y = (r.y - center.y) * 1.00;
		q.z = (r.z - center.z) * 1.00;
		glVertex3f(center.x + q.x, center.y + q.y, center.z + q.z);
	}
	
	/*glColor3f(1.0, 0.0, 1.0);
	glVertex3f(center.x, center.y, center.z);
	glColor3f(0.0, 1.0, 1.0);
	glVertex3f(parent_center.x, parent_center.y, parent_center.z);*/
	
	glEnd();
	//printf("%d\n", root->splitter->polygon_index);
	
	bsp_DrawBsp(near, root, camera_position, level + 1);
	
//	#endif
}
#endif

//#define DRAW_LEAF_EXTENTS
#define DRAW_TRIS
#define SCALE 0.98

#if 0

void bsp_DrawSolidLeaf(bsp_node_t *root, vec3_t *camera_position, int level)
{
	bsp_polygon_t *polygon;
	bsp_triangle_t *triangle;
	bsp_striangle_t *striangle;
	bsp_node_t *near;
	bsp_node_t *far;
	bsp_leaf_t *leaf;
	//bsp_sleaf_t *sleaf;
	vertex_t *first_vertex;
	vec3_t center;
	vec3_t v;
	float d;
	
	int i;
	int c;
	
	float x_min = 9999999999999.0;
	float x_max =-9999999999999.0;
	
	float y_min = 9999999999999.0;
	float y_max =-9999999999999.0;
	
	float z_min = 9999999999999.0;
	float z_max =-9999999999999.0;
	
	
	
	
	if(!root) 
		return;
		
	if(root->type == BSP_LEAF) 
		return;
	
	
	if(root->type != BSP_NODE)
	{
		if(!(root->bm_flags & BSP_SOLID))
		{
			
			sleaf = (bsp_sleaf_t *)root;
			
			//polygon = leaf->polygons;
			
			/*while(polygon)
			{
				
				c = polygon->vert_count;
				
				
				
				glLineWidth(4.0);
				glBegin(GL_LINES);
				glColor3f(color_table[level][0], color_table[level][1], color_table[level][2]);
				for(i = 0; i < c;)
				{
					glVertex3f(polygon->vertices[i % c].x, polygon->vertices[i % c].y, polygon->vertices[i % c].z);
					i++;
					glVertex3f(polygon->vertices[i % c].x, polygon->vertices[i % c].y, polygon->vertices[i % c].z);
				}
				
				glEnd();
				glLineWidth(1.0);
				
				polygon = polygon->next;
			}*/
			striangle = sleaf->tris;
			
			
			#ifdef DRAW_LEAF_EXTENTS
			
			/*
			
			for(i = 0; i < leaf->triangle_count; i++)
			{
				if(triangle[i].a.position.x < x_min) x_min = triangle[i].a.position.x;
				if(triangle[i].a.position.x > x_max) x_max = triangle[i].a.position.x;
				
				if(triangle[i].a.position.y < y_min) y_min = triangle[i].a.position.y;
				if(triangle[i].a.position.y > y_max) y_max = triangle[i].a.position.y;
				
				if(triangle[i].a.position.z < z_min) z_min = triangle[i].a.position.z;
				if(triangle[i].a.position.z > z_max) z_max = triangle[i].a.position.z;
				
				
				
				
				if(triangle[i].b.position.x < x_min) x_min = triangle[i].b.position.x;
				if(triangle[i].b.position.x > x_max) x_max = triangle[i].b.position.x;
				
				if(triangle[i].b.position.y < y_min) y_min = triangle[i].b.position.y;
				if(triangle[i].b.position.y > y_max) y_max = triangle[i].b.position.y;
				
				if(triangle[i].b.position.z < z_min) z_min = triangle[i].a.position.z;
				if(triangle[i].b.position.z > z_max) z_max = triangle[i].a.position.z;
				
				
				
				
				if(triangle[i].c.position.x < x_min) x_min = triangle[i].c.position.x;
				if(triangle[i].c.position.x > x_max) x_max = triangle[i].c.position.x;
				
				if(triangle[i].c.position.y < y_min) y_min = triangle[i].c.position.y;
				if(triangle[i].c.position.y > y_max) y_max = triangle[i].c.position.y;
				
				if(triangle[i].c.position.z < z_min) z_min = triangle[i].c.position.z;
				if(triangle[i].c.position.z > z_max) z_max = triangle[i].c.position.z;
				

			}
			
			
			center.x = (x_max + x_min) / 2.0;
			center.y = (y_max + y_min) / 2.0;
			center.z = (z_max + z_min) / 2.0;
			
			
			x_max -= center.x;
			x_min -= center.x;
			
			y_max -= center.y;
			y_min -= center.y;
			
			z_max -= center.z;
			z_min -= center.z;
			
			
			
			glLineWidth(2.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glBegin(GL_QUADS);
			glColor4f(color_table[level][0], color_table[level][1], color_table[level][2], 0.4);
			
			glVertex3f(center.x + x_min * SCALE, center.y + y_max * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_min * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_min * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_max * SCALE, center.z + z_min * SCALE);
			
			glVertex3f(center.x + x_min * SCALE, center.y + y_max * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_min * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_min * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_max * SCALE, center.z + z_max * SCALE);
			
		
			glVertex3f(center.x + x_max * SCALE, center.y + y_max * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_min * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_min * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_max * SCALE, center.z + z_max * SCALE);
			
			glVertex3f(center.x + x_min * SCALE, center.y + y_max * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_min * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_min * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_max * SCALE, center.z + z_min * SCALE);
			
			
			glVertex3f(center.x + x_min * SCALE, center.y + y_max * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_max * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_max * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_max * SCALE, center.z + z_min * SCALE);
			
			glVertex3f(center.x + x_max * SCALE, center.y + y_min * SCALE, center.z + z_max * SCALE);
			glVertex3f(center.x + x_max * SCALE, center.y + y_min * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_min * SCALE, center.z + z_min * SCALE);
			glVertex3f(center.x + x_min * SCALE, center.y + y_min * SCALE, center.z + z_max * SCALE);
			
			
			
		*/
			
				
			#endif
			//#else	
			
			#ifdef DRAW_TRIS
				
			glLineWidth(2.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glBegin(GL_TRIANGLES);
			
			
			//triangle = leaf->triangles;
			
			glColor4f(color_table[level][0], color_table[level][1], color_table[level][2], 0.4);
			//while(triangle)
			for(i = 0; i < sleaf->tris_count; i++)
			{
				
				first_vertex = &world_vertices[striangle[i].first_vertex];
				
				glVertex3f(first_vertex->position.x, first_vertex->position.y, first_vertex->position.z);
				first_vertex++;
				glVertex3f(first_vertex->position.x, first_vertex->position.y, first_vertex->position.z);
				first_vertex++;
				glVertex3f(first_vertex->position.x, first_vertex->position.y, first_vertex->position.z);
				first_vertex++;
				
				//glNormal3f(triangle[i].a.normal.x, triangle[i].a.normal.y, triangle[i].a.normal.z);
				//glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
				
				/*center.x = (triangle[i].a.position.x + triangle[i].b.position.x + triangle[i].c.position.x) / 3.0;
				center.y = (triangle[i].a.position.y + triangle[i].b.position.y + triangle[i].c.position.y) / 3.0;
				center.z = (triangle[i].a.position.z + triangle[i].b.position.z + triangle[i].c.position.z) / 3.0;
				
				v.x = triangle[i].a.position.x - center.x;
				v.y = triangle[i].a.position.y - center.y;
				v.z = triangle[i].a.position.z - center.z;
				
				glNormal3f(triangle[i].a.normal.x, triangle[i].a.normal.y, triangle[i].a.normal.z);
				glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
				
				v.x = triangle[i].b.position.x - center.x;
				v.y = triangle[i].b.position.y - center.y;
				v.z = triangle[i].b.position.z - center.z;
				
				glNormal3f(triangle[i].b.normal.x, triangle[i].b.normal.y, triangle[i].b.normal.z);
				glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
				
				v.x = triangle[i].c.position.x - center.x;
				v.y = triangle[i].c.position.y - center.y;
				v.z = triangle[i].c.position.z - center.z;
				
				glNormal3f(triangle[i].c.normal.x, triangle[i].c.normal.y, triangle[i].c.normal.z);
				glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);*/
				//triangle = triangle[i].next;						
			}
			glEnd();
			glDisable(GL_BLEND);
			
			#endif
			
			
			#ifdef DRAW_NORMALS
			
			//triangle = leaf->triangles;
			glBegin(GL_LINES);
			//while(triangle)
			for(i = 0; i < leaf->triangle_count; i++)
			{
				
				center.x = (triangle[i].a.position.x + triangle[i].b.position.x + triangle[i].c.position.x) / 3.0;
				center.y = (triangle[i].a.position.y + triangle[i].b.position.y + triangle[i].c.position.y) / 3.0;
				center.z = (triangle[i].a.position.z + triangle[i].b.position.z + triangle[i].c.position.z) / 3.0;
				
				v.x = triangle[i].a.position.x - center.x;
				v.y = triangle[i].a.position.y - center.y;
				v.z = triangle[i].a.position.z - center.z;
				
				glColor3f(fabs(triangle[i].a.normal.x), fabs(triangle[i].a.normal.y), fabs(triangle[i].a.normal.z));
				glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
				glVertex3f(center.x + v.x * SCALE + triangle[i].a.normal.x, center.y + v.y * SCALE + triangle[i].a.normal.y, center.z + v.z * SCALE + triangle[i].a.normal.z);
				
				v.x = triangle[i].b.position.x - center.x;
				v.y = triangle[i].b.position.y - center.y;
				v.z = triangle[i].b.position.z - center.z;
				
				glColor3f(fabs(triangle[i].b.normal.x), fabs(triangle[i].b.normal.y), fabs(triangle[i].b.normal.z));
				glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
				glVertex3f(center.x + v.x * SCALE + triangle[i].b.normal.x, center.y + v.y * SCALE + triangle[i].b.normal.y, center.z + v.z * SCALE + triangle[i].b.normal.z);
				
				v.x = triangle[i].c.position.x - center.x;
				v.y = triangle[i].c.position.y - center.y;
				v.z = triangle[i].c.position.z - center.z;
				
				glColor3f(fabs(triangle[i].c.normal.x), fabs(triangle[i].c.normal.y), fabs(triangle[i].c.normal.z));
				glVertex3f(center.x + v.x * SCALE, center.y + v.y * SCALE, center.z + v.z * SCALE);
				glVertex3f(center.x + v.x * SCALE + triangle[i].c.normal.x, center.y + v.y * SCALE + triangle[i].c.normal.y, center.z + v.z * SCALE + triangle[i].c.normal.z);
				
				//triangle = triangle->next;
										
			}
			glEnd();
			
			
			#endif
			
			glLineWidth(1.0);
			glPointSize(1.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					
			
			//#endif
			
			
		}
		
		return;
	}
	else
	{
		v.x = camera_position->x - root->point.x;
		v.y = camera_position->y - root->point.y;
		v.z = camera_position->z - root->point.z;
		
		d = dot3(v, root->normal);
		
		
		if(d >= 0)
		{
			near = root->front;
			far = root->back;
		}
		else
		{
			near = root->back;
			far = root->front;
		}
		
		level = (level + 1) % 16;
		
		bsp_DrawSolidLeaf(far, camera_position, level);
		
		level = (level + 1) % 16;
		
		bsp_DrawSolidLeaf(near, camera_position, level);
		
		
		
	}
	
	
}

#endif


void bsp_DrawBsp(bsp_pnode_t *node, vec3_t *camera_position, int level)
{
	/*bsp_polygon_t *polygon;
	bsp_triangle_t *triangle;
	bsp_striangle_t *striangle;
	bsp_node_t *near;
	bsp_node_t *far;
	bsp_leaf_t *leaf;
	bsp_sleaf_t *sleaf;*/
	int leaf_index;
	int child_index;
	bsp_dleaf_t *leaf;
	vertex_t *first_vertex;
	vec3_t center;
	vec3_t v;
	float d;
	
	int i;
	int c;
	
	float x_min = 9999999999999.0;
	float x_max =-9999999999999.0;
	
	float y_min = 9999999999999.0;
	float y_max =-9999999999999.0;
	
	float z_min = 9999999999999.0;
	float z_max =-9999999999999.0;
	
	
	
	
	if(!node) 
		return;	
		
	if(node->child[0] == BSP_SOLID_LEAF)
		return;
		
	else if(node->child[0] == BSP_EMPTY_LEAF)
	{
		leaf_index = *(int *)&node->dist;
		leaf = &world_leaves[leaf_index];
		
		glLineWidth(2.0);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBegin(GL_TRIANGLES);
		glColor3f(color_table[level][0], color_table[level][1], color_table[level][2]);
		
		c = leaf->tris_count;
		
		for(i = 0; i < c; i++)
		{
			first_vertex = &world_vertices[leaf->tris[i].first_vertex];
			glNormal3f(first_vertex->normal.x, first_vertex->normal.y, first_vertex->normal.z);
			glVertex3f(first_vertex->position.x, first_vertex->position.y, first_vertex->position.z);
			first_vertex++;
			glNormal3f(first_vertex->normal.x, first_vertex->normal.y, first_vertex->normal.z);
			glVertex3f(first_vertex->position.x, first_vertex->position.y, first_vertex->position.z);
			first_vertex++;
			glNormal3f(first_vertex->normal.x, first_vertex->normal.y, first_vertex->normal.z);
			glVertex3f(first_vertex->position.x, first_vertex->position.y, first_vertex->position.z);
			first_vertex++;	
		}
		
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(1.0);
		
	}
	else
	{
		d = dot3(node->normal, *camera_position) - node->dist;
		
		if(d >= 0.0)
		{
			child_index = 0;
		}
		else
		{
			child_index = 1;
		}
		
		level = (level + 1) % 16;
		
		bsp_DrawBsp(&world_nodes[node->child[child_index]], camera_position, level);
		
		level = (level + 1) % 16;
		
		bsp_DrawBsp(&world_nodes[node->child[child_index^1]], camera_position, level);
		
	}	 
}

#if 0
void bsp_Draw()
{
	camera_t *active_camera = camera_GetActiveCamera();
	float color[4];
	
	if(!world_bsp) return;
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	glUseProgram(0);
	
	shader_UseShader(draw_bsp_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	
	//glEnable(GL_BLEND);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);
	//glLineWidth(4.0);
	//glDisable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
	color[3] = 1.0;
			
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	
	light_BindLightCache();
	
	//glBegin(GL_TRIANGLES);
	//bsp_DrawBsp(world_bsp, world_bsp, active_camera->world_position, 0);
	
	//bsp_DrawSolidLeaf(world_bsp, &active_camera->world_position, 0);
	bsp_DrawBsp(world_nodes, &active_camera->world_position, 0);
	
	//glEnd();
	
	light_UnbindLightCache();
	//glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0);
	//glEnable(GL_DEPTH_TEST);
	
}
#endif





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
	
	/*while(root->type != BSP_LEAF)
	{
		v.x = point.x - root->splitter->vertices[0].x;
		v.y = point.y - root->splitter->vertices[0].y;
		v.z = point.z - root->splitter->vertices[0].z;
		
		d = dot3(v, root->splitter->normal);
		
		if(d >= 0.0)
		{
			root = root->front;
		}
		else
		{
			root = root->back;
		}
	}
	
	return (root->bm_flags & BSP_SOLID) && 1;*/
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
	
	#if 0
	
	if(root->type == BSP_SHORT_LEAF)
	{
	//	printf(">>>>leaf\n");
		if(root->bm_flags & BSP_SOLID)
		{
	//		printf(">>>>>>>>solid\n");
			return 0;
		}
		trace->bm_flags &= ~TRACE_ALL_SOLID;
		return 1;
	}
	else
	{
		
		/* TODO: use Quake's bloddy quick plane tricks? */
		/*v.x = start->x - root->splitter->vertices[0].x;
		v.y = start->y - root->splitter->vertices[0].y;
		v.z = start->z - root->splitter->vertices[0].z;*/
		
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
	//		printf("recurse front\n");
			return bsp_RecursiveFirstHit(root->front, start, end, t0, t1, trace); /* 0 */
		}
		else if(d0 < 0.0 && d1 < 0.0)
		{
	//		printf("recurse back\n");
			return bsp_RecursiveFirstHit(root->back, start, end, t0, t1, trace);	/* 1 */
		}

		
		if(d0 < 0.0)
		{
			/* nudge the intersection away from the plane
			on both sides... */
			frac = (d0 + DIST_EPSILON) / (d0 - d1);
			frac2 = (d0 - DIST_EPSILON) / (d0 - d1);
			near = root->back;
			far = root->front;
			
			//d0 -= DIST_EPSILON;
			
		}
		else
		{
			frac = (d0 - DIST_EPSILON) / (d0 - d1);
			frac2 = (d0 + DIST_EPSILON) / (d0 - d1);
			near = root->front;
			far = root->back;
			
			//d0 += DIST_EPSILON;
		}
		
		
		
		if(frac > 1.0) frac = 1.0;
		else if(frac < 0.0) frac = 0.0;
		
		
	//	printf("%f\n", frac);
		
		midf = t0 + (t1 - t0) * frac;
		mid.x = start->x + (end->x - start->x) * frac;
		mid.y = start->y + (end->y - start->y) * frac;
		mid.z = start->z + (end->z - start->z) * frac;
				
	//	printf("recurse near\n");
		if(!bsp_RecursiveFirstHit(near, start, &mid, t0, midf, trace))	/* 2 */
		{
	//		printf("near solid\n");
			return 0;
		}

		if(frac2 > 1.0) frac2 = 1.0;
		else if(frac2 < 0.0) frac2 = 0.0;

		midf2 = t0 + (t1 - t0) * frac2;
		
		mid2.x = start->x + (end->x - start->x) * frac2;
		mid2.y = start->y + (end->y - start->y) * frac2;
		mid2.z = start->z + (end->z - start->z) * frac2;
		
		//printf("recurse far\n");
		
		//i = bsp_RecursiveFirstHit(far, &mid2, end, midf2, t1, trace);
		
		//if(i)	/* 3 */
		if(bsp_RecursiveFirstHit(far, &mid2, end, midf2, t1, trace))
		{
	//		printf("far empty\n");
			return 1;	
		}

		
		
		/* I'm not satisfied with this... */
		if(frac < trace->frac)
		{
		//	printf(">>>>>>>>>>>>set trace\n");
			
			trace->frac = frac;
			trace->dist = d0;
			trace->normal = root->normal;
			trace->position = mid;
			
			
		}
		
	//	printf(">>>>>>>normal: [%f %f %f]  %f\n", trace->normal.x, trace->normal.y, trace->normal.z, trace->frac);
		
		
		
		return 0;
	}
	
	#endif
	
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










