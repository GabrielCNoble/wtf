#include "r_common.h"
#include "r_editor.h"
#include "editor.h"

#include "vector.h"
#include "matrix.h"


#include "world.h"
#include "mesh.h"
#include "camera.h"
#include "brush.h"
#include "l_main.h"
#include "l_cache.h"
#include "shader.h"
#include "material.h"
#include "player.h"
#include "bsp_common.h"

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
extern int handle_3d_mode;

/* from material.c */
extern material_t *materials;

/* from brush.c */
extern int brush_count;
extern brush_t *brushes;


/* from world.c */
extern int global_triangle_group_count;
extern triangle_group_t *global_triangle_groups;
extern int visible_leaves_count;
extern bsp_dleaf_t **visible_leaves;
extern bsp_lights_t *leaf_lights;
extern vertex_t *world_vertices;
extern bsp_dleaf_t *world_leaves;
extern int world_leaves_count;


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


/* from player.c */
extern int spawn_point_count;
extern spawn_point_t *spawn_points;


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

void renderer_EditorDraw()
{
	
	if(b_draw_editor && editor_state == EDITOR_EDITING)
	{
		if(b_draw_brushes)
		{
			renderer_DrawBrushes();
		}
		
		if(b_draw_lights)
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
		
		if(b_draw_cursors)
		{
			renderer_DrawCursors();	
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
		
		//bsp_DrawPolygons();
		//bsp_DrawPortals();
		
	}
	
	bsp_DrawExpandedBrushes();
	
	
}

void renderer_DrawBrushes()
{
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	int i;
	int c = brush_count;
	int light_index;
	int j;
	int k;
	
	float color[] = {1.0, 1.0, 1.0, 1.0};

	//gpu_BindGpuHeap();
	
	if(!b_draw_brushes) 
		return;
		
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	//glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	//glStencilFunc(GL_ALWAYS, 0x1, 0xff);
	//glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	//glClearStencil(0);
	glViewport(0, 0, r_window_width, r_window_height);
	//glClear(GL_STENCIL_BUFFER_BIT);
	
	/* do a z-prepass for brushes... */
	shader_UseShader(z_pre_pass_shader);
	for(i = 0; i < c; i++)
	{	
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;	
			
		glDrawArrays(GL_TRIANGLES, brushes[i].start, brushes[i].vertex_count);
	}
	
	
	//glStencilFunc(GL_ALWAYS, 0x1, 0xff);
	//glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
	//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	//glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	//glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR_WRAP, GL_KEEP);
	//glDepthMask(GL_FALSE);
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	
	
	
	//glBindBuffer(GL_ARRAY_BUFFER, stencil_meshes);
//	shader_UseShader(stencil_lights_pass_shader);
//	shader_SetCurrentShaderVertexAttribPointer(ATTRIB_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
//	c = visible_light_count;
	
	//glLoadIdentity();
	//glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);


	
//	for(i = 0; i < c; i++)
//	{
//		light_index = visible_lights[i];
//		light_SetLight(light_index);
//		glDrawArrays(GL_TRIANGLES, stencil_light_mesh_start, stencil_light_mesh_vert_count);		
//	}
	
	
	//shader_UseShader(geometry_pass_shader);
	//shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, light_count);
	//shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	
	shader_UseShader(forward_pass_brush_shader);
	
	
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, shared_shadow_map);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, cluster_texture);
	shader_SetCurrentShaderUniform1i(UNIFORM_CLUSTER_TEXTURE, 0);
	shader_SetCurrentShaderUniform4fv(UNIFORM_ACTIVE_CAMERA_POSITION, &active_camera->world_position.floats[0]);
	//shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_SAMPLER2, 1);
	shader_SetCurrentShaderUniform1i(UNIFORM_WIDTH, r_window_width);
	shader_SetCurrentShaderUniform1i(UNIFORM_HEIGHT, r_window_height);
	shader_SetCurrentShaderUniform1i(UNIFORM_FRAME, r_frame);
	
	glEnable(GL_CULL_FACE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	
	
	c = brush_count;
	
	for(i = 0; i < c; i++)
	{
		
		if(brushes[i].type == BRUSH_BOUNDS)
			continue;
		
		if(brushes[i].type == BRUSH_INVALID)
			continue;	
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brushes[i].element_buffer);
		k = brushes[i].triangle_group_count;
		
		triangle_group = brushes[i].triangle_groups;
				
		for(j = 0; j < k; j++)
		{
			material = &materials[triangle_group[j].material_index];
			
			color[0] = (float)material->r / 255.0;
			color[1] = (float)material->g / 255.0;
			color[2] = (float)material->b / 255.0;
			color[3] = (float)material->a / 255.0;
			
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
			glDrawElements(GL_TRIANGLES, triangle_group[j].next, GL_UNSIGNED_INT, (void *)(triangle_group[j].start * sizeof(int)));
		}
	}
	
	//glDepthMask(GL_TRUE);
	//glDisable(GL_STENCIL_TEST);
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
	
	glUseProgram(0);
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
	
	triangle_group_t *triangle_group;
	//brush_triangle_t *triangles;
	vertex_t *vertices;
	camera_t *active_camera = camera_GetActiveCamera();
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	glUseProgram(0);
	gpu_BindGpuHeap();
	
	
	
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
				vertices = brushes[index].vertices;
				k = brushes[index].vertex_count;
				
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
				
				//if(!i)
				/*if(i == c - 1)
				{
					glColor3f(0.2, 0.2, 1.0);
				}
				else
				{
					glColor3f(0.2, 0.2, 0.6);
				}*/
				
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
				
				/*if(i == c - 1)
				{
					glColor3f(0.2, 0.2, 1.0);
				}
				else
				{
					glColor3f(0.2, 0.2, 0.6);
				}*/
				
				glVertex3f(light_positions[index].position.x, light_positions[index].position.y, light_positions[index].position.z);
				glEnd();
				
				glDisable(GL_POINT_SMOOTH);
				glDisable(GL_STENCIL_TEST);
			break;
		}
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
	
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t forward_vector;
	vec3_t v;
		
	
	
	
	glUseProgram(0);
	
	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glLoadIdentity();
	//glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);
	
	//glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glStencilMask(0xff);
	
	glEnable(GL_POINT_SMOOTH);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cursor_framebuffer_id);
	//glEnable(GL_BLEND);
	
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, cursor_framebuffer_id);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 0xff, 0xff);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	
	if(selection_count)
	{
		cursor_position.vec3 = handle_3d_position;
		cursor_position.w = 1.0;
		
		mat4_t_vec4_t_mult(&active_camera->world_to_camera_matrix, &cursor_position);		
		
		if(cursor_position.z < nznear)
		{
			
			glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
			
			/*right_vector = active_camera->world_to_camera_matrix.r_axis;
			up_vector = active_camera->world_to_camera_matrix.u_axis;
			forward_vector = active_camera->world_to_camera_matrix.f_axis;*/
			
			d = sqrt(cursor_position.x * cursor_position.x + cursor_position.y * cursor_position.y + cursor_position.z * cursor_position.z);
			
			
			right_vector = vec3(1.0, 0.0, 0.0);
			up_vector = vec3(0.0, 1.0, 0.0);
			forward_vector = vec3(0.0, 0.0, 1.0);
			
			d *= 0.2;		
					
			right_vector.x *= d;
			up_vector.y *= d;
			forward_vector.z *= d;
			
				
			switch(handle_3d_mode)
			{
				case HANDLE_3D_TRANSLATION:
				case HANDLE_3D_SCALE:
					right_vector.x += handle_3d_position.x;
					right_vector.y += handle_3d_position.y;
					right_vector.z += handle_3d_position.z;
					
					up_vector.x += handle_3d_position.x;
					up_vector.y += handle_3d_position.y;
					up_vector.z += handle_3d_position.z;
					
					forward_vector.x += handle_3d_position.x;
					forward_vector.y += handle_3d_position.y;
					forward_vector.z += handle_3d_position.z;
					
					glDisable(GL_BLEND);
					glPointSize(16.0);
					glBegin(GL_POINTS);
					glColor4f(1.0, 1.0, 1.0, 1.0);
					glVertex3f(handle_3d_position.x, handle_3d_position.y, handle_3d_position.z);
					glEnd();
					
					if(handle_3d_mode == HANDLE_3D_TRANSLATION)
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
	
	glLineWidth(1.0);
	glPointSize(16.0);
	if(cursor_position.z < nznear)
	{	
		glBegin(GL_POINTS);
		glColor4f(1.0, 1.0, 1.0, 0.6);
		glVertex3f(cursor_position.x, cursor_position.y, -0.5);
		glEnd();
		
		glBegin(GL_LINES);
		glColor4f(0.0, 1.0, 0.0, 0.6);
		glVertex3f(cursor_position.x, cursor_position.y + 0.08 * qt, -0.5);
		glVertex3f(cursor_position.x, cursor_position.y - 0.08 * qt, -0.5);
		
		glColor4f(1.0, 0.0, 0.0, 0.6);
		glVertex3f(cursor_position.x - 0.08 * qr, cursor_position.y, -0.5);
		glVertex3f(cursor_position.x + 0.08 * qr, cursor_position.y, -0.5);
		
		glEnd();
	}
	
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, cursor_framebuffer_id);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glViewport(0, 0, r_width, r_height);
	
	//glReadBuffer(GL_STENCIL_ATTACHMENT);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cursor_color_texture_id);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0xff, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	glBegin(GL_QUADS);
	glColor4f(1.0, 1.0, 1.0, 0.6);
	
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
	glLineWidth(1.0);
	glPointSize(1.0);
	//glDisable(GL_BLEND);
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_STENCIL_TEST);
	
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
	glUseProgram(0);
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
	
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(4.0);
	
	glDisable(GL_CULL_FACE);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	
	for(i = 0; i < spawn_point_count; i++)
	{
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
	
	glUseProgram(0);
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
		//break;
		glColor3f(0.35, 0.35, 0.35);
		
	}
	glEnd();
	
	/*glPointSize(8.0);
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	
	for(i = 0; i < c; i++)
	{
		leaf = visible_leaves[i];
		
		glVertex3f(leaf->center.x, leaf->center.y, leaf->center.z);
		glColor3f(0.35, 0.35, 0.35);
		//break;
	}
	
	glEnd();
	glPointSize(1.0);*/
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
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
	
	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glLineWidth(2.0);
	
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 1.0, 1.0);
		
	for(i = 0; i < c; i++)
	{
		//leaf = visible_leaves[i];		
		
		/*for(k = 0; k < selection_count; k++)
		{
			if(selections[k].type == PICK_LIGHT)
			{
				light_index = selections[k].index0;
				if(leaf_lights[i].lights[light_index >> 5] & (1 << (light_index % 32)))
					break;
			}
		}
		
		if(k >= selection_count)
			continue;*/
			
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

















