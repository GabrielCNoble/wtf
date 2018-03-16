#include "r_common.h"
#include "r_editor.h"
#include "editor.h"
#include "r_main.h"

#include "vector.h"
#include "matrix.h"


#include "world.h"
#include "model.h"
#include "camera.h"
#include "brush.h"
#include "l_main.h"
#include "l_cache.h"
#include "shader.h"
#include "material.h"
#include "player.h"
#include "bsp_cmp.h"
#include "bsp_common.h"
#include "pvs.h"
#include "entity.h"
#include "model.h"
 
/* from r_main.c */
extern int z_pre_pass_shader;
extern int forward_pass_shader;
int forward_pass_brush_shader;
extern int geometry_pass_shader;
extern int stencil_lights_pass_shader;

/* from editor.c */
extern int max_selections;
extern int selection_count;
extern pick_record_t *selections;
extern vec3_t cursor_3d_position;
extern vec3_t handle_3d_position;
extern int ed_handle_3d_mode;
extern int ed_editing_mode;
extern int ed_handle_3d_tranform_mode;
extern int draw_cursors_shader;

extern int ed_selected_brush_polygon_index;
extern int ed_selected_brush_selection_index;



/* from material.c */
extern material_t *materials;

/* from brush.c */
extern int brush_count;
extern brush_t *brushes;

extern int ent_entity_list_cursor;
extern entity_t *ent_entities;


/* from world.c */
extern int global_triangle_group_count;
extern triangle_group_t *global_triangle_groups;
extern int visible_leaves_count;
extern bsp_dleaf_t **visible_leaves;
extern bsp_lights_t *leaf_lights;
extern vertex_t *world_vertices;
extern bsp_dleaf_t *world_leaves;
extern int world_leaves_count;
extern bsp_pnode_t *collision_nodes;


/* from l_main.c */
extern light_position_t *visible_light_positions;
extern light_params_t *visible_light_params;
extern light_position_t *light_positions;
extern light_params_t *light_params;
extern int light_count;
extern int visible_light_count;
extern int visible_lights[];
extern int light_cache_cursor;
extern light_cache_slot_t light_cache[];
extern unsigned int stencil_light_mesh_handle;
extern unsigned int stencil_light_mesh_start;
extern unsigned int stencil_light_mesh_vert_count;
extern unsigned int cluster_texture;


/* from editor.c */
extern unsigned int cursor_framebuffer_id;
extern unsigned int cursor_color_texture_id;
extern unsigned int cursor_depth_texture_id;
extern int editor_state;


/* from r_main.c */
extern int r_frame;
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;
extern unsigned int r_backbuffer_id;


/* from player.c */
extern int spawn_point_count;
extern spawn_point_t *spawn_points;


/* from bsp_cmp.c */
extern bsp_node_t *world_bsp;
extern bsp_node_t *collision_bsp;
extern int expanded_brush_count;
extern brush_t *expanded_brushes;

/* from pvs.c */
extern bsp_portal_t *world_portals;
extern int b_step;
extern int b_calculating_pvs;
extern SDL_mutex *polygon_copy_mutex;
extern SDL_sem *step_semaphore;
extern pvs_for_leaf_stack_t *pvs_for_leaf_stack;

static float angles_lut[ROTATION_HANDLE_DIVS][2];





int b_draw_editor = 1;


int b_draw_brushes = 1;
int b_draw_lights = 1;
int b_draw_grid = 1;
int b_draw_spawn_points = 1;
int b_draw_cursors = 1;
int b_draw_selected = 1;
int b_draw_leaves = 0;
int b_draw_light_leaves = 0;
int b_draw_world_polygons = 0;
int b_draw_brush_polygons = 0;
int b_draw_brush_polygons_normals = 1;
int b_draw_portals = 0;
int b_draw_pvs_steps = 0;
int b_draw_collision_polygons = 0;
int b_draw_brush_obbs = 1;



/* this file is the heaven of fixed function pipeline. The CPU regards
those functions with a big middle finger... */


void renderer_EditorDraw()
{
	
	//renderer_DrawBrushes();
	
	if(b_draw_editor && editor_state == EDITOR_EDITING)
	{
		if(b_draw_brushes)
		{
			renderer_DrawBrushes();
		}
		
		/*if(b_draw_lights)
		{
			renderer_DrawLights();
		}
		
		if(b_draw_grid)
		{
			renderer_DrawGrid();
		}
		
		if(b_draw_spawn_points)
		{
			renderer_DrawSpawnPoints();
		}
		
		if(b_draw_selected)
		{
			renderer_DrawSelected();
		}
		
		if(b_draw_light_leaves)
		{
			renderer_DrawLightLeaves();
		}
		
		if(b_draw_leaves)
		{
			renderer_DrawLeaves();
		}

		if(b_draw_world_polygons)
		{
			renderer_DrawWorldPolygons();
		}
		
		if(b_draw_brush_polygons)
		{
			renderer_DrawBrushPolygons();
		}
		
		if(b_draw_portals)
		{
			renderer_DrawPortals();
		}*/
		
		/*if(b_draw_pvs_steps)
		{
			renderer_DrawPvsSteps();
		}*/
		
		/*if(b_draw_cursors)
		{
			renderer_DrawCursors();	
		}*/
		
		//bsp_DrawPolygons();
		//bsp_DrawPortals();
		
	}
	
	//bsp_DrawExpandedBrushes();
	
	
}

void renderer_PostDraw()
{
	
	
	/*if(b_draw_world_polygons)
		{
			renderer_DrawWorldPolygons();
		}*/
	
	if(b_draw_editor && editor_state == EDITOR_EDITING)
	{	
		if(b_draw_lights)
		{
			renderer_DrawLights();
		}
		
		if(b_draw_brush_obbs)
		{
			renderer_DrawBrushesOBBs();
		}
		
		
		if(b_draw_grid)
		{
			renderer_DrawGrid();
		}
		
	
		
		if(b_draw_spawn_points)
		{
			renderer_DrawSpawnPoints();
		}
		
		
		
		if(b_draw_selected)
		{
			renderer_DrawSelected();
		}
		
		
		
		if(b_draw_light_leaves)
		{
			renderer_DrawLightLeaves();
		}
		
		
		
		if(b_draw_leaves)
		{
			renderer_DrawLeaves();
		}
		
		

		if(b_draw_world_polygons)
		{
			renderer_DrawWorldPolygons();
		}
		
		
		if(b_draw_collision_polygons)
		{
			renderer_DrawCollisionPolygons();
		}
		
		
		
		if(b_draw_brush_polygons)
		{
			//renderer_DrawBrushPolygons();
			renderer_DrawClippedPolygons();
			//renderer_DrawBrushBsp();
		}
		
		
		
		if(b_draw_portals)
		{
			renderer_DrawPortals();
		}
		
		

		
		if(b_draw_cursors)
		{
			renderer_DrawCursors();	
		}
			
		
		//bsp_DrawPolygons();
		//bsp_DrawPortals();
		
	}
}

mat4_t transform;

void renderer_DrawBrushes()
{
	
	
/*	transform = mat4_t_id();
	int i;*/
	
	
	/*for(i = 0; i < brush_count; i++)
	{
		renderer_SubmitDrawCommand(&transform, GL_TRIANGLES, brushes[i].)
	}*/
	
	
	
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	bsp_polygon_t *polygon;
	int i;
	int c = brush_count;
	int light_index;
	int j;
	int k;
	
	float color[] = {1.0, 1.0, 1.0, 1.0};

	
	if(!b_draw_brushes) 
		return;

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	
	
	renderer_SetShader(z_pre_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	
	renderer_SetProjectionMatrix(&active_camera->projection_matrix);
	renderer_SetViewMatrix(&active_camera->world_to_camera_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();
	
	for(i = 0; i < c; i++)
	{	
	
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;	
		
		if(brushes[i].bm_flags & BRUSH_INVISIBLE)
			continue;
		
		if(brushes[i].bm_flags & BRUSH_SUBTRACTIVE)
			continue;	
				
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brushes[i].element_buffer);	
		glDrawElements(GL_TRIANGLES, brushes[i].index_count, GL_UNSIGNED_INT, 0);
			
	}
	
	renderer_SetShader(forward_pass_brush_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, (int)&((vertex_t *)0)->normal, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, (int)&((vertex_t *)0)->tangent, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, (int)&((vertex_t *)0)->tex_coord, sizeof(vertex_t));
	renderer_SetClusterTexture();
	renderer_SetUniform1i(UNIFORM_r_width, r_width);
	renderer_SetUniform1i(UNIFORM_r_height, r_height);
	renderer_SetUniform1i(UNIFORM_r_frame, r_frame);
	renderer_SetUniform4fv(UNIFORM_active_camera_position, &active_camera->world_position.floats[0]);
	
	renderer_UpdateMatrices();
	
	glEnable(GL_CULL_FACE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	
	c = brush_count;
	
	for(i = 0; i < c; i++)
	{
		
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;	
		
		if(brushes[i].bm_flags & BRUSH_INVISIBLE)
			continue;	
	
		/*if(brushes[i].bm_flags & BRUSH_SUBTRACTIVE)
		{
			renderer_SetShader(-1);
			glColor3f(0.3, 0.0, 0.0);
			glDisable(GL_CULL_FACE);
			polygon = brushes[i].polygons;
			while(polygon)
			{		
				glBegin(GL_LINE_LOOP);
				
				for(j = 0; j < polygon->vert_count; j++)
				{
					glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
				}
				
				glEnd();
				polygon = polygon->next;
			}
			
			glEnable(GL_CULL_FACE);
		}*/
		
		//else
		{
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brushes[i].element_buffer);
			
			k = brushes[i].triangle_group_count;	
			triangle_group = brushes[i].triangle_groups;				
			for(j = 0; j < k; j++)
			{
				
				renderer_SetMaterial(triangle_group[j].material_index);				
				glDrawElements(GL_TRIANGLES, triangle_group[j].next, GL_UNSIGNED_INT, (void *)(triangle_group[j].start * sizeof(int)));
			}
		}
		
	}
}


void renderer_RecursiveDrawBrushBsp(bsp_node_t *node)
{
	
	int i;
	int c;
	
	vec3_t center;
	
	bsp_polygon_t *polygon;
	if(!node)
		return;
	
	if(node->type == BSP_LEAF)
		return;
		
	polygon = node->splitter;
	c = polygon->vert_count;
	
	
	center.x = 0.0;
	center.y = 0.0;
	center.z = 0.0;
	
	for(i = 0; i < c; i++)
	{
		center.x += polygon->vertices[i].position.x;
		center.y += polygon->vertices[i].position.y;
		center.z += polygon->vertices[i].position.z;
	}
	
	center.x /= c;
	center.y /= c;
	center.z /= c;
	
	
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < c;)
	{
		glVertex3f(polygon->vertices[i].position.x, polygon->vertices[i].position.y, polygon->vertices[i].position.z);
		i++;
		glVertex3f(polygon->vertices[i % c].position.x, polygon->vertices[i % c].position.y, polygon->vertices[i % c].position.z);
	}
	glEnd();
	
	
	glBegin(GL_LINES);
		glVertex3f(center.x, center.y, center.z);
		glVertex3f(center.x + polygon->normal.x, center.y + polygon->normal.y, center.z + polygon->normal.z);
	glEnd();
	
	renderer_RecursiveDrawBrushBsp(node->front);
	renderer_RecursiveDrawBrushBsp(node->back);
		
}

void renderer_DrawBrushBsp()
{
	int i;
	camera_t *active_camera = camera_GetActiveCamera();
	
	
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	
	renderer_SetShader(-1);
	glColor3f(0.0, 1.0, 0.0);
	
	
	for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;
			
		renderer_RecursiveDrawBrushBsp(brushes[i].brush_bsp);	

	}
	
	
	
	
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	
}


void renderer_DrawBrushesOBBs()
{
	int i;
	int j;
	
	camera_t *active_camera = camera_GetActiveCamera();
	
	
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	
	renderer_SetShader(-1);
	
	
	glPointSize(8.0);
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	
	
	for(i = 0; i < brush_count; i++)
	{
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		/*for(j = 0; j < 3; j++)
		{
			glVertex3f(brushes[i].obb[j].x + brushes[i].position.x, brushes[i].obb[j].y + brushes[i].position.y, brushes[i].obb[j].z + brushes[i].position.z);	
			glVertex3f(-brushes[i].obb[j].x + brushes[i].position.x, -brushes[i].obb[j].y + brushes[i].position.y, -brushes[i].obb[j].z + brushes[i].position.z);
		}*/
		
		//for(j = 0; j < 3; j++)
		//{
			/*glVertex3f(brushes[i].obb[0].x + brushes[i].position.x, brushes[i].obb[0].y + brushes[i].position.y, brushes[i].obb[0].z + brushes[i].position.z);
			
			glVertex3f(-brushes[i].obb[0].x + brushes[i].position.x, brushes[i].obb[0].y + brushes[i].position.y, brushes[i].obb[0].z + brushes[i].position.z);
			
			glVertex3f(brushes[i].obb[0].x + brushes[i].position.x, -brushes[i].obb[0].y + brushes[i].position.y, brushes[i].obb[0].z + brushes[i].position.z);
			
			glVertex3f(-brushes[i].obb[0].x + brushes[i].position.x, -brushes[i].obb[0].y + brushes[i].position.y, brushes[i].obb[0].z + brushes[i].position.z);
			
			glVertex3f(brushes[i].obb[0].x + brushes[i].position.x, brushes[i].obb[0].y + brushes[i].position.y, -brushes[i].obb[0].z + brushes[i].position.z);
			
			glVertex3f(-brushes[i].obb[0].x + brushes[i].position.x, brushes[i].obb[0].y + brushes[i].position.y, -brushes[i].obb[0].z + brushes[i].position.z);
			
			glVertex3f(brushes[i].obb[0].x + brushes[i].position.x, -brushes[i].obb[0].y + brushes[i].position.y, -brushes[i].obb[0].z + brushes[i].position.z);
			
			glVertex3f(-brushes[i].obb[0].x + brushes[i].position.x, -brushes[i].obb[0].y + brushes[i].position.y, -brushes[i].obb[0].z + brushes[i].position.z);*/
		//}
		
		
	}
	
	glEnd();
	glPointSize(1.0);
	
	
	
	
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
	
}

/*
==============
renderer_DrawGrid
==============
*/
void renderer_DrawGrid()
{
	
	int i;
	int j;
	camera_t *active_camera = camera_GetActiveCamera();
	
	//glUseProgram(0);
	renderer_SetShader(-1);
	glLineWidth(2.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	glBegin(GL_QUADS);
	glColor3f(0.3, 0.3, 0.3);
	for(i = 0; i <= 10; i++)
	{
		glVertex3f(-i, 0.0,-10.0);
		glVertex3f(-i, 0.0, 10.0);
		glVertex3f( i, 0.0, 10.0);
		glVertex3f( i, 0.0,-10.0);
	}
	
	for(i = 0; i <= 10; i++)
	{
		glVertex3f(-10.0, 0.0,-i);
		glVertex3f( 10.0, 0.0,-i);
		glVertex3f( 10.0, 0.0, i);
		glVertex3f(-10.0, 0.0, i);
	}
	
	glEnd();
	
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0);
	
	
	
	
}



/*
==============
renderer_DrawSelected
==============
*/
void renderer_DrawSelected()
{
	int i;
	int c = selection_count;
	int j;
	int k;
	int index;
	int vertice_count;
	int *indexes;
	model_t *model;
	mesh_t *mesh;
	entity_t *entity;
	
	mat4_t transform;
	
	vec3_t pos;
	
	triangle_group_t *triangle_group;
	//brush_triangle_t *triangles;
	vertex_t *vertices;
	bsp_polygon_t *polygon;
	camera_t *active_camera = camera_GetActiveCamera();
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	//glUseProgram(0);
	renderer_SetShader(-1);
	gpu_BindGpuHeap();
	
		
	if(ed_editing_mode == EDITING_MODE_OBJECT)
	{
		for(i = 0; i < c; i++)
		{
			
			if(i == c - 1)
			{
				glColor3f(0.2, 0.2, 1.0);
			}
			else
			{
				glColor3f(0.2, 0.2, 0.6);
			}
			
			
			
			switch(selections[i].type)
			{
				case PICK_BRUSH:
					

					index = selections[i].index0;
					indexes = brushes[index].indexes;
					vertices = brushes[index].vertices;
					k = brushes[index].index_count;
					
					glEnable(GL_STENCIL_TEST);
					glClear(GL_STENCIL_BUFFER_BIT);
					glStencilFunc(GL_ALWAYS, 0x1, 0xff);
					glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					glDepthMask(GL_FALSE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					
					glBegin(GL_TRIANGLES);
					//glColor3f(0.0, 0.0, 0.6);
					for(j = 0; j < k;)
					{
						glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
						j++;
						glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
						j++;
						glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
						j++;
					}
					glEnd();
					
					
					glStencilFunc(GL_NOTEQUAL, 0x1, 0xff);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glDepthMask(GL_TRUE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glLineWidth(4.0);
					glBegin(GL_TRIANGLES);
					
					for(j = 0; j < k;)
					{
						glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
						j++;
						glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
						j++;
						glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
						j++;
					}
					glEnd();
					glLineWidth(1.0);
					
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDisable(GL_STENCIL_TEST);
				
				break;
				
				
				
				case PICK_ENTITY:
					index = selections[i].index0;
					entity = &ent_entities[index];
					model = model_GetModelPointerIndex(entity->model_index);
					mesh = model->mesh;
					
					vertices = mesh->vertices;
					k = mesh->vert_count;
					
					//indexes = brushes[index].indexes;
					//vertices = brushes[index].vertices;
					//k = brushes[index].index_count;
					
					glPushMatrix();
					
					mat4_t_compose(&transform, &entity->orientation, entity->position);
					
					transform.floats[0][0] *= entity->scale.x;
					transform.floats[0][1] *= entity->scale.x;
					transform.floats[0][2] *= entity->scale.x;
					
					
					transform.floats[1][0] *= entity->scale.y;
					transform.floats[1][1] *= entity->scale.y;
					transform.floats[1][2] *= entity->scale.y;
					
					
					transform.floats[2][0] *= entity->scale.z;
					transform.floats[2][1] *= entity->scale.z;
					transform.floats[2][2] *= entity->scale.z;
					
					mat4_t_mult_fast(&transform, &transform, &active_camera->world_to_camera_matrix);
					
					glLoadMatrixf(&transform.floats[0][0]);
					
					
					glEnable(GL_STENCIL_TEST);
					glClear(GL_STENCIL_BUFFER_BIT);
					glStencilFunc(GL_ALWAYS, 0x1, 0xff);
					glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					glDepthMask(GL_FALSE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					
					glBegin(GL_TRIANGLES);
					//glColor3f(0.0, 0.0, 0.6);
					for(j = 0; j < k;)
					{
						glVertex3f(vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);
						j++;
						glVertex3f(vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);
						j++;
						glVertex3f(vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);
						j++;
					}
					glEnd();
					
					
					glStencilFunc(GL_NOTEQUAL, 0x1, 0xff);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glDepthMask(GL_TRUE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glLineWidth(4.0);
					glBegin(GL_TRIANGLES);
					
					for(j = 0; j < k;)
					{
						glVertex3f(vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);
						j++;
						glVertex3f(vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);
						j++;
						glVertex3f(vertices[j].position.x, vertices[j].position.y, vertices[j].position.z);
						j++;
					}
					glEnd();
					glLineWidth(1.0);
					
					glPopMatrix();
					
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDisable(GL_STENCIL_TEST);
				break;	
				
				case PICK_LIGHT:
					
					glEnable(GL_STENCIL_TEST);
					
					index = selections[i].index0;
					
					glStencilFunc(GL_ALWAYS, 0x1, 0xff);
					glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
					
					glClear(GL_STENCIL_BUFFER_BIT);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					glDepthMask(GL_FALSE);
					glPointSize(12.0);
					glEnable(GL_POINT_SMOOTH);
					
					glBegin(GL_POINTS);
					glVertex3f(light_positions[index].position.x, light_positions[index].position.y, light_positions[index].position.z);
					glEnd();
					
					
					glStencilFunc(GL_NOTEQUAL, 0x1, 0xff);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					
					
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glDepthMask(GL_TRUE);
					glPointSize(16.0);
					
					glBegin(GL_POINTS);
					
					glVertex3f(light_positions[index].position.x, light_positions[index].position.y, light_positions[index].position.z);
					glEnd();
					
					glDisable(GL_POINT_SMOOTH);
					glDisable(GL_STENCIL_TEST);
				break;
				
				case PICK_SPAWN_POINT:
					
					index = selections[i].index0;
						
					pos = spawn_points[index].position;
					
					glEnable(GL_STENCIL_TEST);
					glClear(GL_STENCIL_BUFFER_BIT);
					glStencilFunc(GL_ALWAYS, 0x1, 0xff);
					glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					glDepthMask(GL_FALSE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						
					glBegin(GL_QUADS);
					
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
							
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					
						
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
							
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					
					
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
							
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					
					glEnd();
					
					
					
					glStencilFunc(GL_NOTEQUAL, 0x1, 0xff);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glDepthMask(GL_TRUE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glLineWidth(4.0);
					
					
					glBegin(GL_QUADS);
					
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
							
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					
						
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
							
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					
					
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
							
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
					glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
					
					glEnd();
					
					glLineWidth(1.0);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDisable(GL_STENCIL_TEST);
					
						
				break;
			}
		}
	}
	else if(ed_editing_mode == EDITING_MODE_BRUSH || ed_editing_mode == EDITING_MODE_UV)
	{
		
		//index = selections[selection_count - 1].index0;
		index = selections[ed_selected_brush_selection_index].index0;
		
		vertices = brushes[index].vertices;
		indexes = brushes[index].indexes;
		k = brushes[index].index_count;
		
		glEnable(GL_STENCIL_TEST);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilFunc(GL_ALWAYS, 0x1, 0xff);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					
		glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.5, 0.0);
		for(j = 0; j < k;)
		{
			glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
			j++;
			glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
			j++;
			glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
			j++;
		}
		glEnd();
					
					
		glStencilFunc(GL_NOTEQUAL, 0x1, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(4.0);
		glBegin(GL_TRIANGLES);
					
		for(j = 0; j < k;)
		{
			glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
			j++;
			glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
			j++;
			glVertex3f(vertices[indexes[j]].position.x, vertices[indexes[j]].position.y, vertices[indexes[j]].position.z);
			j++;
		}
		glEnd();
	
		glDisable(GL_STENCIL_TEST);
		
		if(ed_selected_brush_polygon_index > -1)
		{
			polygon = brushes[index].polygons + ed_selected_brush_polygon_index;
			vertice_count = polygon->vert_count;

			glColor4f(1.0, 0.0, 0.0, 0.25);
			glBegin(GL_LINE_LOOP);
			for(j = 0; j < vertice_count; j++)
			{
				glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
			}
			glEnd();
			
			glEnable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
			glBegin(GL_TRIANGLE_FAN);
			for(j = 0; j < vertice_count; j++)
			{
				glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
			}
			glEnd();
			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
		}
		
		
		glLineWidth(1.0);
			
		
		
		
		
	}
	
	
	
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
}


/*
==============
renderer_DrawCursors
==============
*/
void renderer_DrawCursors()
{
	camera_t *active_camera = camera_GetActiveCamera();
	
	vec4_t cursor_position;
	vec4_t handle_position;
	
	float color[3];
	
	int i;
	float step = (2.0 * 3.14159265) / ROTATION_HANDLE_DIVS;
	float angle = 0.0;
	
	float angles_lut[ROTATION_HANDLE_DIVS][2];
	
	float s;
	float c;
	
	float nzfar = -active_camera->frustum.zfar;
	float nznear = -active_camera->frustum.znear;
	float ntop = active_camera->frustum.top;
	float nright = active_camera->frustum.right;
	float qt = nznear / ntop;
	float qr = nznear / nright;
	float d;
	
	int index;
	
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t forward_vector;
	vec3_t v;
		
	
	
	
	//glUseProgram(0);
	renderer_SetShader(-1);
	
	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);
	
	//glEnable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0xff);
	
	glEnable(GL_POINT_SMOOTH);
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cursor_framebuffer_id);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glEnable(GL_BLEND);
	
	glViewport(0, 0, r_width, r_height);
	
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cursor_framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0xff, 0xff);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	
	
	
	if(selection_count && ed_editing_mode != EDITING_MODE_UV)
	{
		cursor_position.vec3 = handle_3d_position;
		cursor_position.w = 1.0;
		
		mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &cursor_position);		
		
		if(cursor_position.z < nznear)
		{
			
			glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);		
			d = sqrt(cursor_position.x * cursor_position.x + cursor_position.y * cursor_position.y + cursor_position.z * cursor_position.z);
			
			
			/*right_vector = vec3(1.0, 0.0, 0.0);
			up_vector = vec3(0.0, 1.0, 0.0);
			forward_vector = vec3(0.0, 0.0, 1.0);*/
			
			if(ed_handle_3d_tranform_mode == HANDLE_3D_TRANFORM_GLOBAL || ed_handle_3d_mode == HANDLE_3D_TRANSLATION)
			{
				_global_vectors:
					
				right_vector = vec3(1.0, 0.0, 0.0);
				up_vector = vec3(0.0, 1.0, 0.0);
				forward_vector = vec3(0.0, 0.0, 1.0);
			}
			else
			{
				index = selections[selection_count - 1].index0;
					
				switch(selections[selection_count - 1].type)
				{
					case PICK_BRUSH:
						right_vector = brushes[index].orientation.r_axis;
						up_vector = brushes[index].orientation.u_axis;
						forward_vector = brushes[index].orientation.f_axis;
					break;
					
					case PICK_ENTITY:
						right_vector = ent_entities[index].orientation.r_axis;
						up_vector = ent_entities[index].orientation.u_axis;
						forward_vector = ent_entities[index].orientation.f_axis;
					break;
					
					case PICK_LIGHT:
					case PICK_SPAWN_POINT:
						goto _global_vectors;
					break;
				}
			}
			
			
			d *= 0.2;		
					
			right_vector.x *= d;
			right_vector.y *= d;
			right_vector.z *= d;
			
			up_vector.x *= d;
			up_vector.y *= d;
			up_vector.z *= d;
			
			forward_vector.x *= d;
			forward_vector.y *= d;
			forward_vector.z *= d;
			
				
			switch(ed_handle_3d_mode)
			{
				case HANDLE_3D_TRANSLATION:
				case HANDLE_3D_SCALE:
					
					
					
					glDisable(GL_BLEND);
					glPointSize(16.0);
					glBegin(GL_POINTS);
					glColor4f(1.0, 1.0, 1.0, 1.0);
					glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
					glEnd();
					
					right_vector.x += handle_3d_position.x;
					right_vector.y += handle_3d_position.y;
					right_vector.z += handle_3d_position.z;
						
					up_vector.x += handle_3d_position.x;
					up_vector.y += handle_3d_position.y;
					up_vector.z += handle_3d_position.z;
						
					forward_vector.x += handle_3d_position.x;
					forward_vector.y += handle_3d_position.y;
					forward_vector.z += handle_3d_position.z;
					
					if(ed_handle_3d_mode == HANDLE_3D_TRANSLATION)
					{
						
						
						glPointSize(6.0);
						glBegin(GL_POINTS);
						glColor4f(1.0, 0.0, 0.0, 1.0);
						glVertex3f(right_vector.x, right_vector.y, right_vector.z);
						glColor4f(0.0, 1.0, 0.0, 1.0);
						glVertex3f(up_vector.x, up_vector.y, up_vector.z);
						glColor4f(0.0, 0.0, 1.0, 1.0);
						glVertex3f(forward_vector.x, forward_vector.y, forward_vector.z);
						glEnd();
					}
					else
					{
						#define SCALE_HANDLE_CUBE_EXTENT 0.08 * d
						
						glDisable(GL_CULL_FACE);
						glBegin(GL_QUADS);
						glColor3f(1.0, 0.0, 0.0);
						
						v.x = right_vector.x;
						v.y = right_vector.y;
						v.z = right_vector.z;
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
						
						
						glColor3f(0.0, 1.0, 0.0);
						
						v.x = up_vector.x;
						v.y = up_vector.y;
						v.z = up_vector.z;
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
						
						
						glColor3f(0.0, 0.0, 1.0);
						
						v.x = forward_vector.x;
						v.y = forward_vector.y;
						v.z = forward_vector.z;
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y + SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x - SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z + SCALE_HANDLE_CUBE_EXTENT);
						glVertex3f(v.x + SCALE_HANDLE_CUBE_EXTENT, v.y - SCALE_HANDLE_CUBE_EXTENT, v.z - SCALE_HANDLE_CUBE_EXTENT);
						
						glEnd();
						glEnable(GL_CULL_FACE);
					}
					
					
					
					glLineWidth(6.0);
					glBegin(GL_LINES);
					glColor4f(1.0, 0.0, 0.0, 1.0);
					glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
					glVertex3f(right_vector.x, right_vector.y, right_vector.z);
					
					glColor4f(0.0, 1.0, 0.0, 1.0);
					glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
					glVertex3f(up_vector.x, up_vector.y, up_vector.z);
					
					glColor4f(0.0, 0.0, 1.0, 1.0);
					glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
					glVertex3f(forward_vector.x, forward_vector.y, forward_vector.z);
					glEnd();
					
				break;
				
				case HANDLE_3D_ROTATION:
					angle = 0.0;

					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						s = sin(angle);
						c = cos(angle);
						/* a lut that gets regenerated every
						frame is not exactly a lut... */
						angles_lut[i][0] = sin(angle) * d;
						angles_lut[i][1] = cos(angle) * d;	
						angle += step;
					}
					
					glLineWidth(6.0);
					glBegin(GL_LINE_LOOP);
					glColor4f(1.0, 0.0, 0.0, 1.0);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(handle_3d_position.x, handle_3d_position.y + angles_lut[i][0], handle_3d_position.z + angles_lut[i][1]);
					}		
					glEnd();
					glPointSize(4.0);
					glBegin(GL_POINTS);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(handle_3d_position.x, handle_3d_position.y + angles_lut[i][0], handle_3d_position.z + angles_lut[i][1]);
					}		
					glEnd();
					
					
					glBegin(GL_LINE_LOOP);
					glColor4f(0.0, 1.0, 0.0, 1.0);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y, handle_3d_position.z + angles_lut[i][0]);
					}		
					glEnd();
					glBegin(GL_POINTS);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y, handle_3d_position.z + angles_lut[i][0]);
					}		
					glEnd();
					
					
					glBegin(GL_LINE_LOOP);
					glColor4f(0.0, 0.0, 1.0, 1.0);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y + angles_lut[i][0], handle_3d_position.z);
					}		
					glEnd();
					glBegin(GL_POINTS);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(handle_3d_position.x + angles_lut[i][1], handle_3d_position.y + angles_lut[i][0], handle_3d_position.z);
					}		
					glEnd();
				break;	
			}
			
			glLoadIdentity();
			
			
		}
	}	
	
	
	
	//glDisable(GL_STENCIL_TEST);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	cursor_position.vec3 = cursor_3d_position;
	cursor_position.w = 1.0;
	
	mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &cursor_position);
	cursor_position.x = (cursor_position.x * qr) / cursor_position.z;
	cursor_position.y = (cursor_position.y * qt) / cursor_position.z;
	
	//renderer_SetShader(draw_cursors_shader);

	glLineWidth(1.0);
	glPointSize(16.0);
	if(cursor_position.z < nznear)
	{	
		/*color[0] = 1.0;
		color[1] = 1.0;
		color[2] = 1.0;*/
		
		glBegin(GL_POINTS);
		//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(cursor_position.x, cursor_position.y, -0.5);
		glEnd();
		
		glBegin(GL_LINES);
		/*color[0] = 0.0;
		color[1] = 1.0;
		color[2] = 1.0;*/
		//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(cursor_position.x, cursor_position.y + 0.08 * qt, -0.5);
		glVertex3f(cursor_position.x, cursor_position.y - 0.08 * qt, -0.5);
		glEnd();
		
		glBegin(GL_LINES);
		//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(cursor_position.x - 0.08 * qr, cursor_position.y, -0.5);
		glVertex3f(cursor_position.x + 0.08 * qr, cursor_position.y, -0.5);
		
		glEnd();
	}
	
	
/*	glBegin(GL_POINTS);
	glColor3f(1.0, 0.0 ,0.0);
	glVertex3f(0.0, 0.0, -0.5);
	glEnd();*/
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, cursor_framebuffer_id);
	
	
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_window_width, r_window_height, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glViewport(0, 0, r_window_width, r_window_height);
	
	//glReadBuffer(GL_STENCIL_ATTACHMENT);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cursor_color_texture_id);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0xff, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendEquation(GL_ADD);
	
	renderer_SetShader(draw_cursors_shader);
	renderer_BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, cursor_color_texture_id);
	renderer_SetUniform1i(UNIFORM_texture_sampler0, 0);
	//glUseProgram(0);
	
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0, 1.0, 0.0);
	
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, 0.0);
	
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0, -1.0, 0.0);
	
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0, 1.0, 0.0);
	
	glEnd();
	glDisable(GL_TEXTURE_2D);
	
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_STENCIL_TEST);
	
	/*glBegin(GL_POINTS);
	glColor3f(1.0, 0.0 ,0.0);
	glVertex3f(0.0, 0.0, -0.5);
	glEnd();*/
	
	
	glLineWidth(1.0);
	glPointSize(1.0);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);
	
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	
	
}



/*
==============
renderer_DrawLights
==============
*/
void renderer_DrawLights()
{
	int i;
	int c = light_count;
	camera_t *active_camera = camera_GetActiveCamera();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	glPointSize(12.0);
	glEnable(GL_POINT_SMOOTH);
	//glUseProgram(0);
	renderer_SetShader(-1);
	glBegin(GL_POINTS);
	glColor3f(0.5, 0.5, 1.0);
	for(i = 0; i < c; i++)
	{
		if(light_params[i].bm_flags & LIGHT_INVALID)
			continue;
		glVertex3f(light_positions[i].position.x, light_positions[i].position.y, light_positions[i].position.z);
	}
	
	glEnd();
	glPointSize(1.0);
	glDisable(GL_POINT_SMOOTH);
	
}


void renderer_DrawSpawnPoints()
{
	int i;
	camera_t *active_camera = camera_GetActiveCamera();
	vec3_t pos;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	//glUseProgram(0);
	renderer_SetShader(-1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(4.0);
	
	glDisable(GL_CULL_FACE);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	
	for(i = 0; i < spawn_point_count; i++)
	{
		
		if(spawn_points[i].bm_flags & SPAWN_POINT_INVALID)
			continue;
		
		pos = spawn_points[i].position;
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
	}
	
	glEnd();
	glLineWidth(1.0);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(1.0, 1.0, 1.0, 0.1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	glBegin(GL_QUADS);
	
	for(i = 0; i < spawn_point_count; i++)
	{
		if(spawn_points[i].bm_flags & SPAWN_POINT_INVALID)
			continue;
		
		pos = spawn_points[i].position;
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y + PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
		
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		glVertex3f(pos.x - PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z + PLAYER_Z_EXTENT);
		glVertex3f(pos.x + PLAYER_X_EXTENT, pos.y - PLAYER_Y_EXTENT, pos.z - PLAYER_Z_EXTENT);
		
	}
	
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glPopMatrix();
	
}

void renderer_DrawLeaves()
{
	int i;
	int c = visible_leaves_count;
	
	int j;
	vec3_t center;
	vec3_t extents;
	bsp_dleaf_t *leaf;
	int triangle_count;
	bsp_striangle_t *triangle;
	
	//glUseProgram(0);
	renderer_SetShader(-1);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBegin(GL_TRIANGLES);
	glColor3f(0.0, 1.0, 0.0);
	
	vec3_t vec = {1.0, 0.0, 0.0};
	mat3_t rot;
	
	
	mat3_t_rotate(&rot, vec3(0.0, 0.0, 1.0), 0.23, 1);
	
	vec = MultiplyVector3(&rot, vec);
	
	
	for(i = 0; i < c; i++)
	{
		leaf = visible_leaves[i];		
		triangle_count = leaf->tris_count;
		
		for(j = 0; j < triangle_count; j++)
		{
			triangle = &leaf->tris[j];
			glVertex3f(world_vertices[triangle->first_vertex].position.x, world_vertices[triangle->first_vertex].position.y, world_vertices[triangle->first_vertex].position.z);
			glVertex3f(world_vertices[triangle->first_vertex + 1].position.x, world_vertices[triangle->first_vertex + 1].position.y, world_vertices[triangle->first_vertex + 1].position.z);
			glVertex3f(world_vertices[triangle->first_vertex + 2].position.x, world_vertices[triangle->first_vertex + 2].position.y, world_vertices[triangle->first_vertex + 2].position.z);
		}
		glColor3f(0.35, 0.35, 0.35);
		
	}
	
	glEnd();
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_BLEND);
	glBegin(GL_TRIANGLES);
	glColor4f(0.0, 1.0, 0.0, 0.2);
	//for(i = 0; i < c; i++)
	//{
	
	
	if(visible_leaves_count)
	{
		leaf = visible_leaves[0];
		triangle_count = leaf->tris_count;
		
		for(j = 0; j < triangle_count; j++)
		{
			triangle = &leaf->tris[j];
			glVertex3f(world_vertices[triangle->first_vertex].position.x, world_vertices[triangle->first_vertex].position.y, world_vertices[triangle->first_vertex].position.z);
			glVertex3f(world_vertices[triangle->first_vertex + 1].position.x, world_vertices[triangle->first_vertex + 1].position.y, world_vertices[triangle->first_vertex + 1].position.z);
			glVertex3f(world_vertices[triangle->first_vertex + 2].position.x, world_vertices[triangle->first_vertex + 2].position.y, world_vertices[triangle->first_vertex + 2].position.z);
		}
	}
		
		//glColor3f(0.35, 0.35, 0.35);
		
	//}
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
		
}


void renderer_DrawLightLeaves()
{
	int i;
	int c = world_leaves_count;
	
	int j;
	int k = light_count;
	
	int light_index;
	vec3_t center;
	vec3_t extents;
	bsp_dleaf_t *leaf;
	int triangle_count;
	bsp_striangle_t *triangle;
	
	if(!selection_count)
		return;
	
	//if(selections[0].type != PICK_LIGHT)
	//	return;
			
	light_index = selections[0].index0;
	
	//glUseProgram(0);
	renderer_SetShader(-1);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glLineWidth(2.0);
	
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
		
	for(i = 0; i < c; i++)
	{	
		for(j = 0; j < k; j++)
		{
			if(leaf_lights[i].lights[j >> 5] & (1 << (j % 32)))
				break;
		}	
		
		if(j >= k)
			continue;
		
		
		
		leaf = &world_leaves[i];
		
		triangle_count = leaf->tris_count;
		
		for(j = 0; j < triangle_count; j++)
		{
			triangle = &leaf->tris[j];
			glVertex3f(world_vertices[triangle->first_vertex].position.x, world_vertices[triangle->first_vertex].position.y, world_vertices[triangle->first_vertex].position.z);
			glVertex3f(world_vertices[triangle->first_vertex + 1].position.x, world_vertices[triangle->first_vertex + 1].position.y, world_vertices[triangle->first_vertex + 1].position.z);
			glVertex3f(world_vertices[triangle->first_vertex + 2].position.x, world_vertices[triangle->first_vertex + 2].position.y, world_vertices[triangle->first_vertex + 2].position.z);
		}
		
	}
	glEnd();
	
	glLineWidth(1.0);
		
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void renderer_DrawLightBoxes()
{
	int i;
	int c = light_count;
	light_params_t *parms;
	if(!world_leaves)
		return;
	
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0);
	glDisable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	glColor3f(0.0, 1.0, 0.0);	
		
	for(i = 0; i < c; i++)
	{
		parms = &light_params[i];
		
		glVertex3f(parms->box_min.x, parms->box_max.y, parms->box_max.z);
		glVertex3f(parms->box_min.x, parms->box_min.y, parms->box_max.z);
		glVertex3f(parms->box_max.x, parms->box_min.y, parms->box_max.z);
		glVertex3f(parms->box_max.x, parms->box_max.y, parms->box_max.z);
		
		glVertex3f(parms->box_max.x, parms->box_min.y, parms->box_min.z);
		glVertex3f(parms->box_max.x, parms->box_max.y, parms->box_min.z);
		glVertex3f(parms->box_min.x, parms->box_max.y, parms->box_min.z);
		glVertex3f(parms->box_min.x, parms->box_min.y, parms->box_min.z);
		
		glVertex3f(parms->box_min.x, parms->box_max.y, parms->box_min.z);
		glVertex3f(parms->box_min.x, parms->box_max.y, parms->box_max.z);
		glVertex3f(parms->box_max.x, parms->box_max.y, parms->box_max.z);
		glVertex3f(parms->box_max.x, parms->box_max.y, parms->box_min.z);
		
		glVertex3f(parms->box_min.x, parms->box_min.y, parms->box_min.z);
		glVertex3f(parms->box_min.x, parms->box_min.y, parms->box_max.z);
		glVertex3f(parms->box_max.x, parms->box_min.y, parms->box_max.z);
		glVertex3f(parms->box_max.x, parms->box_min.y, parms->box_min.z);
	}	
	
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glLineWidth(1.0);
}


void renderer_RecursiveDrawWorldPolygons(bsp_node_t *node)
{
	
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	int i;
	vec3_t center;
	vec3_t vert_vec;
	
	if(!node)
		return;
		
		
	
	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;
		
		if(!(leaf->bm_flags & BSP_SOLID))
		{
			polygon = leaf->polygons;
				
			while(polygon)
			{
				center.x = 0.0;
				center.y = 0.0;
				center.z = 0.0;
					
				for(i = 0; i < polygon->vert_count; i++)
				{
					center.x += polygon->vertices[i].position.x;
					center.y += polygon->vertices[i].position.y;
					center.z += polygon->vertices[i].position.z;
				}
				
				center.x /= polygon->vert_count;
				center.y /= polygon->vert_count;
				center.z /= polygon->vert_count;
				
				#define POLYGON_SCALE 0.99
				glColor3f(1.0, 1.0, 1.0);
				glBegin(GL_LINE_LOOP);
				for(i = 0; i < polygon->vert_count; i++)
				{
					vert_vec.x = (polygon->vertices[i].position.x - center.x) * POLYGON_SCALE;
					vert_vec.y = (polygon->vertices[i].position.y - center.y) * POLYGON_SCALE;
					vert_vec.z = (polygon->vertices[i].position.z - center.z) * POLYGON_SCALE;
					
					glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
				}
				glEnd();
				
				
				glBegin(GL_LINES);
				glColor3f(fabs(polygon->normal.x), fabs(polygon->normal.y), fabs(polygon->normal.z));
				glVertex3f(center.x, center.y, center.z);
				glVertex3f(center.x + polygon->normal.x, center.y + polygon->normal.y, center.z + polygon->normal.z);
				glEnd();
				polygon = polygon->next;
				
				#undef POLYGON_SCALE
				
				
				
				
			}
			
		}		
	}	
	else
	{
		renderer_RecursiveDrawWorldPolygons(node->front);
		renderer_RecursiveDrawWorldPolygons(node->back);
	}
	
	
}


void renderer_DrawWorldPolygons()
{
	
	
	
	bsp_polygon_t *polygon;
	camera_t *active_camera;
	vec3_t center;
	vec3_t v;
	int i;
	int vert_count;
	
	if(!world_bsp)
		return;
		
	
	active_camera = camera_GetActiveCamera();	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	//glUseProgram(0);	
	renderer_SetShader(-1);
		
	glLineWidth(4.0);
	glColor3f(1.0, 1.0, 1.0);
	//glBegin(GL_LINE_LOOP);
	renderer_RecursiveDrawWorldPolygons(world_bsp);
	//glEnd();
	glLineWidth(1.0);
}


void renderer_RecursiveDrawCollisionPolygons(bsp_node_t *node)
{
		bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	int i;
	vec3_t center;
	vec3_t vert_vec;
	
	if(!node)
		return;
		
		
	
	if(node->type == BSP_LEAF)
	{
		leaf = (bsp_leaf_t *)node;
		
		if(!(leaf->bm_flags & BSP_SOLID))
		{
			polygon = leaf->polygons;
				
			while(polygon)
			{
				center.x = 0.0;
				center.y = 0.0;
				center.z = 0.0;
					
				for(i = 0; i < polygon->vert_count; i++)
				{
					center.x += polygon->vertices[i].position.x;
					center.y += polygon->vertices[i].position.y;
					center.z += polygon->vertices[i].position.z;
				}
				
				center.x /= polygon->vert_count;
				center.y /= polygon->vert_count;
				center.z /= polygon->vert_count;
				
				#define POLYGON_SCALE 0.99
				glColor4f(1.0, 0.5, 0.5, 0.1);
				glBegin(GL_LINE_LOOP);
				for(i = 0; i < polygon->vert_count; i++)
				{
					vert_vec.x = (polygon->vertices[i].position.x - center.x) * POLYGON_SCALE;
					vert_vec.y = (polygon->vertices[i].position.y - center.y) * POLYGON_SCALE;
					vert_vec.z = (polygon->vertices[i].position.z - center.z) * POLYGON_SCALE;
					
					glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
				}
				glEnd();
				
				glEnable(GL_BLEND);
				glBegin(GL_TRIANGLE_FAN);
				for(i = 0; i < polygon->vert_count; i++)
				{
					vert_vec.x = (polygon->vertices[i].position.x - center.x) * POLYGON_SCALE;
					vert_vec.y = (polygon->vertices[i].position.y - center.y) * POLYGON_SCALE;
					vert_vec.z = (polygon->vertices[i].position.z - center.z) * POLYGON_SCALE;
					
					glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
				}
				glEnd();
				glDisable(GL_BLEND);
				
				
				glBegin(GL_LINES);
				glColor3f(fabs(polygon->normal.x), fabs(polygon->normal.y), fabs(polygon->normal.z));
				glVertex3f(center.x, center.y, center.z);
				glVertex3f(center.x + polygon->normal.x, center.y + polygon->normal.y, center.z + polygon->normal.z);
				glEnd();
				polygon = polygon->next;
				
				#undef POLYGON_SCALE
				
				
				
				
			}
			
		}		
	}	
	else
	{
		renderer_RecursiveDrawCollisionPolygons(node->front);
		renderer_RecursiveDrawCollisionPolygons(node->back);
	}
}


void renderer_DrawCollisionPolygons()
{
	bsp_polygon_t *polygon;
	camera_t *active_camera;
	vec3_t center;
	vec3_t v;
	int i;
	int vert_count;
	
	if(!collision_bsp)
		return;
		
	
	active_camera = camera_GetActiveCamera();	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	//glUseProgram(0);	
	renderer_SetShader(-1);
		
	//glLineWidth(4.0);
	//glColor3f(1.0, 1.0, 1.0);
	//glBegin(GL_LINE_LOOP);
	glDepthMask(GL_FALSE);
	renderer_RecursiveDrawCollisionPolygons(collision_bsp);
	glDepthMask(GL_TRUE);
	//glEnd();
	//glLineWidth(1.0);
}






void renderer_DrawCollisionNodes()
{
	
}




#if 0
void renderer_DrawCollisionPolygons()
{
	int i;
	int j;
	bsp_polygon_t *polygon;
	vec3_t center;
	vec3_t vert_vec;
	
	
	if(expanded_brushes)
	{
		glDepthMask(GL_FALSE);
		for(j = 0; j < expanded_brush_count; j++)
		{
			polygon = expanded_brushes[j].polygons;
				
			while(polygon)
			{
				center.x = 0.0;
				center.y = 0.0;
				center.z = 0.0;
					
				for(i = 0; i < polygon->vert_count; i++)
				{
					center.x += polygon->vertices[i].position.x;
					center.y += polygon->vertices[i].position.y;
					center.z += polygon->vertices[i].position.z;
				}
				
				center.x /= polygon->vert_count;
				center.y /= polygon->vert_count;
				center.z /= polygon->vert_count;
				
				#define POLYGON_SCALE 0.99
				glColor4f(1.0, 0.5, 0.5, 0.1);
				glBegin(GL_LINE_LOOP);
				for(i = 0; i < polygon->vert_count; i++)
				{
					vert_vec.x = (polygon->vertices[i].position.x - center.x) * POLYGON_SCALE;
					vert_vec.y = (polygon->vertices[i].position.y - center.y) * POLYGON_SCALE;
					vert_vec.z = (polygon->vertices[i].position.z - center.z) * POLYGON_SCALE;
					
					glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
				}
				glEnd();
				
				glEnable(GL_BLEND);
				glBegin(GL_TRIANGLE_FAN);
				for(i = 0; i < polygon->vert_count; i++)
				{
					vert_vec.x = (polygon->vertices[i].position.x - center.x) * POLYGON_SCALE;
					vert_vec.y = (polygon->vertices[i].position.y - center.y) * POLYGON_SCALE;
					vert_vec.z = (polygon->vertices[i].position.z - center.z) * POLYGON_SCALE;
					
					glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
				}
				glEnd();
				glDisable(GL_BLEND);
				
				
				glBegin(GL_LINES);
				glColor3f(fabs(polygon->normal.x), fabs(polygon->normal.y), fabs(polygon->normal.z));
				glVertex3f(center.x, center.y, center.z);
				glVertex3f(center.x + polygon->normal.x, center.y + polygon->normal.y, center.z + polygon->normal.z);
				glEnd();
				polygon = polygon->next;
				
				#undef POLYGON_SCALE
				
				
				
				
			}	
		}
		
		glDepthMask(GL_TRUE);
	}
}
#endif

void renderer_DrawPortals()
{
	bsp_portal_t *portal;
	bsp_polygon_t *polygon;
	int i;
	int j;
	
	camera_t *active_camera = camera_GetActiveCamera();
	vec3_t center;
	vec3_t vert_vec;
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	//glUseProgram(0);
	renderer_SetShader(-1);
	
	glLineWidth(4.0);
	glDepthMask(GL_FALSE);
	//glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	//glColor4f(0.25, 0.25, 0.8, 0.1);
	
	
	if(!world_portals)
		return;
		
		
	portal = world_portals;
	
	
	while(portal)
	{
		polygon = portal->portal_polygon;
		
		while(polygon)
		{
			center.x = 0.0;
			center.y = 0.0;
			center.z = 0.0;
						
			for(j = 0; j < polygon->vert_count; j++)
			{
				center.x += polygon->vertices[j].position.x;
				center.y += polygon->vertices[j].position.y;
				center.z += polygon->vertices[j].position.z;
			}
					
			center.x /= polygon->vert_count;
			center.y /= polygon->vert_count;
			center.z /= polygon->vert_count;
					
			
			glEnable(GL_BLEND);
			glColor4f(0.45, 0.45, 0.8, 0.1);
			glBegin(GL_TRIANGLE_FAN);
			
			for(j = 0; j < 3; j++)
			{
				glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
			}

			for(; j < polygon->vert_count; j++)
			{
				glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
			}
			glEnd();
			
			
			#define POLYGON_SCALE 0.99
			glDisable(GL_BLEND);
			glBegin(GL_LINE_LOOP);
			glColor3f(1.0, 1.0, 1.0);
			for(j = 0; j < 3; j++)
			{
				vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
				vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
				vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;				
				glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);		
			}

			for(; j < polygon->vert_count; j++)
			{
				vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
				vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
				vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;		
				glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
			}
			glEnd();
			
			polygon = polygon->next;
			
			#undef POLYGON_SCALE
		}
		
		portal = portal->next;
	}
	//glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0);	
	
}


void renderer_DrawBrushPolygons()
{
	int i;
	int j;
	bsp_polygon_t *polygon;
	camera_t *active_camera = camera_GetActiveCamera();
	vec3_t center;
	vec3_t vert_vec;
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	//glUseProgram(0);
	renderer_SetShader(-1);
	
	glLineWidth(4.0);
	
	
	for(i = 0; i < brush_count; i++)
	{
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;
	
		polygon = brushes[i].polygons;
					
		while(polygon)
		{
			center.x = 0.0;
			center.y = 0.0;
			center.z = 0.0;
						
			for(j = 0; j < polygon->vert_count; j++)
			{
				center.x += polygon->vertices[j].position.x;
				center.y += polygon->vertices[j].position.y;
				center.z += polygon->vertices[j].position.z;
			}
					
			center.x /= polygon->vert_count;
			center.y /= polygon->vert_count;
			center.z /= polygon->vert_count;
					
			#define POLYGON_SCALE 0.99
			
			glBegin(GL_LINE_LOOP);
			glColor3f(1.0, 1.0, 1.0);
			for(j = 0; j < polygon->vert_count; j++)
			{
				vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
				vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
				vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;
						
				glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
			}
			glEnd();
				
			#undef POLYGON_SCALE
			
			if(b_draw_brush_polygons_normals)
			{
				glBegin(GL_LINES);
				glColor3f(fabs(polygon->normal.x), fabs(polygon->normal.y), fabs(polygon->normal.z));
				for(j = 0; j < polygon->vert_count; j++)
				{	
					glVertex3f(center.x, center.y, center.z);
					glVertex3f(center.x + polygon->normal.x, center.y + polygon->normal.y, center.z + polygon->normal.z);
				}
				glEnd();
			}
			
			
			polygon = polygon->next;
			
		}
				
	}
	
	glLineWidth(1.0);		
	
}


void renderer_DrawClippedPolygons()
{
	int i;
	int j;
	bsp_polygon_t *polygon;
	camera_t *active_camera = camera_GetActiveCamera();
	vec3_t center;
	vec3_t vert_vec;
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	//glUseProgram(0);
	renderer_SetShader(-1);
	
	glLineWidth(2.0);
	
	
	for(i = 0; i < brush_count; i++)
	{
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;
		
		polygon = brushes[i].clipped_polygons;
					
		while(polygon)
		{
			center.x = 0.0;
			center.y = 0.0;
			center.z = 0.0;
						
			for(j = 0; j < polygon->vert_count; j++)
			{
				center.x += polygon->vertices[j].position.x;
				center.y += polygon->vertices[j].position.y;
				center.z += polygon->vertices[j].position.z;
			}
					
			center.x /= polygon->vert_count;
			center.y /= polygon->vert_count;
			center.z /= polygon->vert_count;
					
			#define POLYGON_SCALE 1.0
			
			glBegin(GL_LINE_LOOP);
			glColor3f(0.0, 1.0, 0.0);
			for(j = 0; j < polygon->vert_count; j++)
			{
				vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
				vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
				vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;
						
				glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
			}
			glEnd();
				
			#undef POLYGON_SCALE
			
			if(b_draw_brush_polygons_normals)
			{
				glBegin(GL_LINES);
				glColor3f(fabs(polygon->normal.x), fabs(polygon->normal.y), fabs(polygon->normal.z));
				for(j = 0; j < polygon->vert_count; j++)
				{	
					glVertex3f(center.x, center.y, center.z);
					glVertex3f(center.x + polygon->normal.x, center.y + polygon->normal.y, center.z + polygon->normal.z);
				}
				glEnd();
			}
			
			
			polygon = polygon->next;
			
		}
				
	}
	
	glLineWidth(1.0);	
}



void renderer_DrawSourceLeaf()
{
	
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	int j;
	
	
	recursive_stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer;
	
	leaf = recursive_stack->src_leaf;
	polygon = leaf->polygons;
	
	glLineWidth(4.0);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glColor4f(0.0, 1.0, 0.0, 0.25);
	
	while(polygon)
	{
		glBegin(GL_TRIANGLE_FAN);
		for(j = 0; j < polygon->vert_count; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
		glEnd();		
		polygon = polygon->next;
	}

	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}


void renderer_DrawSourcePortal()
{
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	bsp_portal_t *portal;
	vec3_t center;
	vec3_t vert_vec;
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	int j;
	
	
	recursive_stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer;
	
	leaf = recursive_stack->src_leaf;
	polygon = leaf->polygons;

	portal = recursive_stack->src_portal;
	polygon = portal->portal_polygon;
	center.x = 0.0;
	center.y = 0.0;
	center.z = 0.0;
						
	for(j = 0; j < polygon->vert_count; j++)
	{
		center.x += polygon->vertices[j].position.x;
		center.y += polygon->vertices[j].position.y;
		center.z += polygon->vertices[j].position.z;
	}
					
	center.x /= polygon->vert_count;
	center.y /= polygon->vert_count;
	center.z /= polygon->vert_count;
					
			
	glEnable(GL_BLEND);
	glColor4f(0.45, 0.45, 0.8, 0.6);
	glBegin(GL_TRIANGLE_FAN);
			
	for(j = 0; j < 3; j++)
	{
		glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
	}

	for(; j < polygon->vert_count; j++)
	{
		glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
	}
	glEnd();
	
		
	#define POLYGON_SCALE 0.99
	glDisable(GL_BLEND);
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0, 1.0, 1.0);
	for(j = 0; j < polygon->vert_count; j++)
	{
		vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
		vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
		vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;				
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);		
	}
	glEnd();
			
	#undef POLYGON_SCALE
		
}

void renderer_DrawDestinationPortals()
{
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	bsp_portal_t *dst_portal;
	vec3_t center;
	vec3_t vert_vec;
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	recursive_pvs_for_leaf_stack_t *stack_base;
	int j;
	int i;
	
	
	recursive_stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer;
	stack_base = pvs_for_leaf_stack->recursive_stack;
	
	//leaf = recursive_stack->dst_leaf;
	//polygon = leaf->polygons;

	//portal = recursive_stack->dst_portal;
	//polygon = portal->portal_polygon;
	//center.x = 0.0;
	//center.y = 0.0;
	//center.z = 0.0;
	
	SDL_LockMutex(polygon_copy_mutex);
						
	for(i = 0; i < pvs_for_leaf_stack->recursive_stack_pointer + 1; i++)
	{
		dst_portal = stack_base[i].dst_portal;
		
		if(!dst_portal)
			break;
						
		polygon = dst_portal->portal_polygon;
		center.x = 0.0;
		center.y = 0.0;
		center.z = 0.0;
							
		for(j = 0; j < polygon->vert_count; j++)
		{
			center.x += polygon->vertices[j].position.x;
			center.y += polygon->vertices[j].position.y;
			center.z += polygon->vertices[j].position.z;
		}
						
		center.x /= polygon->vert_count;
		center.y /= polygon->vert_count;
		center.z /= polygon->vert_count;
		
		
		glEnable(GL_BLEND);
		glColor4f(0.8, 0.45, 0.45, 0.6);
		glBegin(GL_TRIANGLE_FAN);
				
		for(j = 0; j < 3; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
	
		for(; j < polygon->vert_count; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
		glEnd();
		
			
		#define POLYGON_SCALE 0.99
		glDisable(GL_BLEND);
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0, 1.0, 1.0);
		for(j = 0; j < polygon->vert_count; j++)
		{
			vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
			vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
			vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;				
			glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);		
		}
		glEnd();
				
		#undef POLYGON_SCALE
	}
	
	SDL_UnlockMutex(polygon_copy_mutex);
	
	
	
}


void renderer_DrawGeneratorPortals()
{
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	bsp_portal_t *dst_portal;
	vec3_t center;
	vec3_t vert_vec;
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	recursive_pvs_for_leaf_stack_t *stack_base;
	int j;
	int i;
	int portal_count;
	
	
	recursive_stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer;
	stack_base = pvs_for_leaf_stack->recursive_stack;
	
	
	
	
	leaf = recursive_stack->dst_dst_leaf;
	
	if(!leaf)
		return;

	SDL_LockMutex(polygon_copy_mutex);
	
	portal_count = leaf->portal_count;
	
	for(i = 0; i < portal_count; i++)
	{
		
		if(leaf->portals[i] == recursive_stack->dst_dst_portal || leaf->portals[i] == recursive_stack->dst_portal)
			continue;
		
		polygon = leaf->portals[i]->portal_polygon;
		center.x = 0.0;
		center.y = 0.0;
		center.z = 0.0;
							
		for(j = 0; j < polygon->vert_count; j++)
		{
			center.x += polygon->vertices[j].position.x;
			center.y += polygon->vertices[j].position.y;
			center.z += polygon->vertices[j].position.z;
		}
						
		center.x /= polygon->vert_count;
		center.y /= polygon->vert_count;
		center.z /= polygon->vert_count;
		
		
		
		glColor4f(0.45, 0.8, 0.45, 0.6);
		glEnable(GL_BLEND);
		glBegin(GL_TRIANGLE_FAN);
				
		for(j = 0; j < 3; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
	
		for(; j < polygon->vert_count; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
		glEnd();
		
			
		#define POLYGON_SCALE 0.99
		glDisable(GL_BLEND);
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0, 1.0, 1.0);
		for(j = 0; j < polygon->vert_count; j++)
		{
			vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
			vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
			vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;				
			glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);		
		}
		glEnd();
				
		#undef POLYGON_SCALE
	}
	
	SDL_UnlockMutex(polygon_copy_mutex);
}


void renderer_DrawValidPortals()
{
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	bsp_portal_t *dst_portal;
	vec3_t center;
	vec3_t vert_vec;
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	recursive_pvs_for_leaf_stack_t *stack_base;
	int j;
	int i;
	
	int portal_count;
	
	
	recursive_stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer;
	stack_base = pvs_for_leaf_stack->recursive_stack;
	
	
	SDL_LockMutex(polygon_copy_mutex);
	
	portal_count = recursive_stack->out_valid_dst_portal_count;
	
	for(i = 0; i < portal_count; i++)
	{
		if(!recursive_stack->out_valid_dst_portals[i])
			break;
			
		polygon = recursive_stack->out_valid_dst_portals[i]->portal_polygon;
		
		if(!polygon)
			break;
		
		center.x = 0.0;
		center.y = 0.0;
		center.z = 0.0;
							
		for(j = 0; j < polygon->vert_count; j++)
		{
			center.x += polygon->vertices[j].position.x;
			center.y += polygon->vertices[j].position.y;
			center.z += polygon->vertices[j].position.z;
		}
						
		center.x /= polygon->vert_count;
		center.y /= polygon->vert_count;
		center.z /= polygon->vert_count;
		
		
		
		glColor4f(0.8, 0.8, 0.45, 0.6);
		glEnable(GL_BLEND);
		glBegin(GL_TRIANGLE_FAN);
				
		for(j = 0; j < 3; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
	
		for(; j < polygon->vert_count; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
		glEnd();
		
			
		#define POLYGON_SCALE 0.99
		glDisable(GL_BLEND);
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0, 1.0, 1.0);
		for(j = 0; j < polygon->vert_count; j++)
		{
			vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
			vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
			vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;				
			glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);		
		}
		glEnd();
				
		#undef POLYGON_SCALE
	}
	
	SDL_UnlockMutex(polygon_copy_mutex);
}

void renderer_DrawClippingPlanes()
{		
	bsp_leaf_t *leaf;
	bsp_polygon_t *polygon;
	bsp_portal_t *portal;
	vec3_t center;
	vec3_t vert_vec;
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	int j;
	int k;
	float mult;
	
	
	SDL_LockMutex(polygon_copy_mutex);
	
	if(pvs_for_leaf_stack->recursive_stack_pointer < 0)
	{
		SDL_UnlockMutex(polygon_copy_mutex);
		return;
	}
		
	
	recursive_stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer;
		
	leaf = recursive_stack->dst_leaf;
	
	if(!leaf)
	{
		SDL_UnlockMutex(polygon_copy_mutex);
		return;
	}
		
	
	polygon = leaf->polygons;
	
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	for(k = 0; k < recursive_stack->clipplane_count; k++)
	{
		mult = 10.0;
		glColor4f(1.0, 1.0, 1.0, 0.1);
			
		center = recursive_stack->clipplanes[k].point;
			
		glVertex3f(center.x, center.y, center.z);
			
		vert_vec.x = recursive_stack->clipplanes[k].edge0.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge0.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge0.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
			
		vert_vec.x = recursive_stack->clipplanes[k].edge1.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge1.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge1.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
	}
	glEnd();
		
		
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_TRIANGLES);
	for(k = 0; k < recursive_stack->clipplane_count; k++)
	{
		glColor4f(1.0, 1.0, 1.0, 0.1);
		mult = 10.0;
			
		center = recursive_stack->clipplanes[k].point;
		glVertex3f(center.x, center.y, center.z);
		vert_vec.x = recursive_stack->clipplanes[k].edge0.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge0.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge0.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
			
		vert_vec.x = recursive_stack->clipplanes[k].edge1.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge1.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge1.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
	}
	glEnd();
	
	
	SDL_UnlockMutex(polygon_copy_mutex);
}


void renderer_DrawPvsSteps()
{
	#if 0
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	recursive_pvs_for_leaf_stack_t *stack_base;
	camera_t *active_camera;
	bsp_portal_t *portal;
	bsp_portal_t *dst_portal;
	bsp_polygon_t *polygon;
	bsp_leaf_t *leaf;
	bsp_leaf_t *dst_leaf;
	bsp_triangle_t *triangle;
	vec3_t center;
	vec3_t vert_vec;
	int j;
	int i;
	int k;
	
	float mult;
	
	
	//if(!b_step)
	//	return;
	
	if(!world_portals)
		return;	
	
	if(!b_calculating_pvs)
		return;
	
	if(pvs_for_leaf_stack->recursive_stack_pointer < 0)
		return;
		
	recursive_stack = pvs_for_leaf_stack->recursive_stack + pvs_for_leaf_stack->recursive_stack_pointer;
	stack_base = pvs_for_leaf_stack->recursive_stack;
	active_camera = camera_GetActiveCamera();
	
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	glUseProgram(0);
	

	renderer_DrawSourceLeaf();
	
	renderer_DrawSourcePortal();
		
	renderer_DrawDestinationPortals();	
	
	renderer_DrawClippingPlanes();
	
	renderer_DrawGeneratorPortals();
	
	renderer_DrawValidPortals();
	
	/*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	for(k = 0; k < recursive_stack->clipplane_count; k++)
	{
		mult = 10.0;
		glColor4f(1.0, 1.0, 1.0, 0.1);
			
		center = recursive_stack->clipplanes[k].point;
			
		glVertex3f(center.x, center.y, center.z);
			
		vert_vec.x = recursive_stack->clipplanes[k].edge0.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge0.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge0.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
			
		vert_vec.x = recursive_stack->clipplanes[k].edge1.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge1.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge1.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
	}
	glEnd();
		
		
	glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_TRIANGLES);
	for(k = 0; k < recursive_stack->clipplane_count; k++)
	{
		glColor4f(1.0, 1.0, 1.0, 0.1);
		mult = 10.0;
			
		center = recursive_stack->clipplanes[k].point;
		glVertex3f(center.x, center.y, center.z);
		vert_vec.x = recursive_stack->clipplanes[k].edge0.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge0.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge0.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
			
		vert_vec.x = recursive_stack->clipplanes[k].edge1.x * mult;
		vert_vec.y = recursive_stack->clipplanes[k].edge1.y * mult;
		vert_vec.z = recursive_stack->clipplanes[k].edge1.z * mult;
		glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);
	}
	glEnd();*/
	
	
	/*for(i = 0; i < pvs_for_leaf_stack.recursive_stack_pointer + 1; i++)
	{
		dst_portal = stack_base[i].dst_portal;
		
		if(!dst_portal)
			break;
						
		polygon = dst_portal->portal_polygon;
		center.x = 0.0;
		center.y = 0.0;
		center.z = 0.0;
							
		for(j = 0; j < polygon->vert_count; j++)
		{
			center.x += polygon->vertices[j].position.x;
			center.y += polygon->vertices[j].position.y;
			center.z += polygon->vertices[j].position.z;
		}
						
		center.x /= polygon->vert_count;
		center.y /= polygon->vert_count;
		center.z /= polygon->vert_count;
		
		
		glEnable(GL_BLEND);
		glColor4f(0.8, 0.45, 0.45, 0.6);
		glBegin(GL_TRIANGLE_FAN);
				
		for(j = 0; j < 3; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
	
		for(; j < polygon->vert_count; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
		glEnd();
		
			
		#define POLYGON_SCALE 0.99
		glDisable(GL_BLEND);
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0, 1.0, 1.0);
		for(j = 0; j < polygon->vert_count; j++)
		{
			vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
			vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
			vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;				
			glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);		
		}
		glEnd();
				
		#undef POLYGON_SCALE
	}
	
	
	for(i = 0; i < recursive_stack->out_valid_dst_portal_count; i++)
	{
		polygon = recursive_stack->out_valid_dst_portals[i]->portal_polygon;
		center.x = 0.0;
		center.y = 0.0;
		center.z = 0.0;
							
		for(j = 0; j < polygon->vert_count; j++)
		{
			center.x += polygon->vertices[j].position.x;
			center.y += polygon->vertices[j].position.y;
			center.z += polygon->vertices[j].position.z;
		}
						
		center.x /= polygon->vert_count;
		center.y /= polygon->vert_count;
		center.z /= polygon->vert_count;
		
		
		
		glColor4f(0.45, 0.8, 0.45, 0.6);
		glEnable(GL_BLEND);
		glBegin(GL_TRIANGLE_FAN);
				
		for(j = 0; j < 3; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
	
		for(; j < polygon->vert_count; j++)
		{
			glVertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
		glEnd();
		
			
		#define POLYGON_SCALE 0.99
		glDisable(GL_BLEND);
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0, 1.0, 1.0);
		for(j = 0; j < polygon->vert_count; j++)
		{
			vert_vec.x = (polygon->vertices[j].position.x - center.x) * POLYGON_SCALE;
			vert_vec.y = (polygon->vertices[j].position.y - center.y) * POLYGON_SCALE;
			vert_vec.z = (polygon->vertices[j].position.z - center.z) * POLYGON_SCALE;				
			glVertex3f(center.x + vert_vec.x, center.y + vert_vec.y, center.z + vert_vec.z);		
		}
		glEnd();
				
		#undef POLYGON_SCALE
	}*/
	
	
		
		
		
	//}
	//glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glLineWidth(1.0);	
	
	
	#endif
		
	
		
}










