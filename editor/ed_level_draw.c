#include "ed_level_draw.h"
#include "ed_level.h"

#include "..\..\common\GLEW\include\GL\glew.h"
#include "..\..\common\camera.h"
#include "..\..\common\shader.h"
#include "..\..\common\l_main.h"
#include "..\..\common\r_imediate.h"
#include "..\..\common\r_gl.h"
#include "..\..\common\r_main.h"
#include "..\..\common\portal.h"
#include "..\..\common\navigation.h"
#include "..\brush.h"
 
/* from editor.c */
extern unsigned int ed_cursors_framebuffer_id;
extern unsigned int ed_cursors_color_texture_id;
extern int ed_3d_handle_transform_mode;


/* from ed_level.c */
extern int level_editor_draw_brushes;
extern int level_editor_draw_grid;
extern int level_editor_draw_selected;
extern int level_editor_draw_cursors;
extern int level_editor_draw_lights;
extern int level_editor_draw_spawn_points;
extern int level_editor_draw_leaves;
extern int level_editor_draw_world_polygons;
extern int level_editor_draw_clipped_polygons;
extern int level_editor_draw_entities_aabbs;
extern int level_editor_draw_3d_handle;
extern pick_list_t level_editor_pick_list;
extern pick_list_t level_editor_brush_face_pick_list;

extern vec3_t level_editor_3d_handle_position;
extern vec3_t level_editor_3d_cursor_position;
extern int level_editor_3d_handle_transform_mode;
extern int level_editor_editing_mode;
extern int level_editor_state;


/* from brush.c */
extern brush_t *brushes;


/* from r_main.c */
extern int r_frame;
extern int r_width;
extern int r_height;
extern int r_flat;
extern int r_z_pre_pass_shader;
extern int r_flat_pass_shader;
extern int r_forward_pass_shader;
extern int r_blit_texture_shader;
extern unsigned int r_bbuffer_id;


/* from l_main.c */
extern int l_light_list_cursor;
extern light_params_t *l_light_params;
extern light_position_t *l_light_positions;


/* from portal.c */
extern int ptl_portal_list_cursor;
extern portal_t *ptl_portals;


/* from navigation.c */
extern int nav_waypoint_count;
extern struct waypoint_t *nav_waypoints;


/* from r_imediate.c */
extern int r_imediate_color_shader;

void editor_LevelEditorPreDraw()
{
	if(level_editor_state == EDITOR_PIE)
	{
		return;
	}
	
	if(level_editor_draw_brushes)
	{
		editor_LevelEditorDrawBrushes();
	}
}

void editor_LevelEditorPostDraw()
{
	if(level_editor_state == EDITOR_PIE)
	{
		return;
	}
	
	if(level_editor_draw_grid)
	{
		editor_LevelEditorDrawGrid();
	}
	
	if(level_editor_draw_selected)
	{
		editor_LevelEditorDrawSelected();
	}
	
	if(level_editor_draw_lights)
	{
		editor_LevelEditorDrawLights();
	}
	
	if(level_editor_draw_spawn_points)
	{
		editor_LevelEditorDrawSpawnPoints();
	}
	
	if(level_editor_draw_leaves)
	{
		editor_LevelEditorDrawLeaves();
	}
	
	if(level_editor_draw_world_polygons)
	{
		editor_LevelEditorDrawWorldPolygons();
	}
	
	if(level_editor_draw_clipped_polygons)
	{
		editor_LevelEditorDrawClippedPolygons();
	}
	
	if(level_editor_draw_entities_aabbs)
	{
		editor_LevelEditorDrawEntitiesAabbs();
	}
	
	if(level_editor_draw_cursors)
	{
		editor_LevelEditorDrawCursors();
	}
}

mat4_t brush_draw_transform;

void editor_LevelEditorDrawBrushes()
{
	
	brush_t *brush;
	portal_t *portals;
	portal_t *portal;
	portal_recursive_view_data_t *recursive_view_data;
	portal_view_data_t *view_data;
	camera_t *active_camera = camera_GetActiveCamera();	
	camera_t *main_view = camera_GetMainViewCamera();
	batch_t *batch;
	material_t *material;
	bsp_polygon_t *polygon;
	brush_draw_transform = mat4_t_id();
	int i;
	//int c = brush_count;
	int light_index;
	int j;
	int k;
	int l;
	
	float color[] = {10.0, 0.0, 0.0, 0.0};
	
	gpu_BindGpuHeap();
	
	if(!level_editor_draw_brushes) 
		return;
	
	brush = brushes;
	
	while(brush)
	{
		if(!(brush->bm_flags & BRUSH_SUBTRACTIVE))
		{
			for(i = 0; i < brush->batch_count; i++)
			{
				//renderer_SubmitDrawCommandToView(main_view, &brush_draw_transform, GL_TRIANGLES, brush->batches[i].start + brush->index_start, brush->batches[i].next, brush->batches[i].material_index, 1);
				renderer_SubmitDrawCommand(&brush_draw_transform, GL_TRIANGLES, brush->batches[i].start + brush->index_start, brush->batches[i].next, brush->batches[i].material_index, 1);
			}
		}
		brush = brush->next;
	}
	
	/*brush_draw_transform.floats[0][0] *= -1.0;
	brush_draw_transform.floats[1][0] *= -1.0;
	brush_draw_transform.floats[2][0] *= -1.0;*/
	
	//for(j = 0; j < ptl_portal_list_cursor; j++)
	//{
	//	portal = &ptl_portals[j];
		
	//	brush = brushes;
	
	//	while(brush)
	//	{
	//		if(!(brush->bm_flags & BRUSH_SUBTRACTIVE))
	//		{
	//			for(i = 0; i < brush->batch_count; i++)
	//			{
					//renderer_SubmitDrawCommandToView(ptl_portals[j].view, &brush_draw_transform, GL_TRIANGLES, brush->batches[i].start + brush->index_start, brush->batches[i].next, brush->batches[i].material_index, 1);
					
	//				for(k = 0; k < portal->max_recursion; k++)
	//				{
						//recursive_view_data = &ptl_portals[j].portal_recursive_views[k];
	//					recursive_view_data = &portal->portal_recursive_views[k];
						
	//					for(l = 0; l < recursive_view_data->views_count; l++)
	//					{
	//						renderer_SubmitDrawCommand(&recursive_view_data->views[l].view_data, &brush_draw_transform, GL_TRIANGLES, brush->batches[i].start + brush->index_start, brush->batches[i].next, brush->batches[i].material_index, 1);
	//					}
						
	//				}
					//renderer_SubmitDrawCommand(&ptl_portals[j].portal_recursive_views[0].views[0].view_data, &brush_draw_transform, GL_TRIANGLES, brush->batches[i].start + brush->index_start, brush->batches[i].next, brush->batches[i].material_index, 1);
//				}
//			}
			
			
//			brush = brush->next;
//		}
//	}
	
	/*renderer_SetShader(r_flat_pass_shader);
	renderer_UpdateMatrices();
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->position);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->normal);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->tangent);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->tex_coord);
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);*/
	
	renderer_EnableImediateDrawing();
	renderer_SetShader(r_imediate_color_shader);
	renderer_SetActiveView(main_view);
	renderer_SetModelMatrix(NULL);
	renderer_Color3f(10.0, 0.0, 0.0);
	 
	brush = brushes;
	/* subtractive... */
	while(brush)
	{
		if(brush->bm_flags & BRUSH_SUBTRACTIVE)
		{
			polygon = brush->clipped_polygons;
			
			while(polygon)
			{
				renderer_Begin(GL_LINE_LOOP);
				for(i = 0; i < polygon->vert_count; i++)
				{
					renderer_Vertex3f(polygon->vertices[i].position.x, polygon->vertices[i].position.y, polygon->vertices[i].position.z);
				}
				renderer_End();
				
				polygon = polygon->next;
			}
			
		}

		brush = brush->next;
	}
	
	renderer_DisableImediateDrawing();
	
	
	#if 0
	
	glDisable(GL_BLEND);
	
	renderer_SetProjectionMatrix(&active_camera->projection_matrix);
	renderer_SetViewMatrix(&active_camera->world_to_camera_matrix);
	renderer_SetModelMatrix(NULL);
		
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	
		
	renderer_SetShader(r_z_pre_pass_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->position);
	renderer_UpdateMatrices();
			
	brush = brushes;
	
	/* brush z-prepass... */	
	while(brush)
	{				
		if(brush->bm_flags & BRUSH_INVISIBLE)
		{
			brush = brush->next;
			continue;
		}
				
		if(brush->bm_flags & BRUSH_SUBTRACTIVE)
		{
			brush = brush->next;
			continue;
		}
			
		if(!brush->clipped_polygons_index_count)
		{
			brush = brush->next;
			continue;
		}
					
					
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);	
		renderer_DrawElements(GL_TRIANGLES, brush->clipped_polygons_index_count, GL_UNSIGNED_INT, 0);
		
		brush = brush->next;
	}
		
	if(r_flat)
	{
		renderer_SetShader(r_flat_pass_shader);
	}
	else
	{
		renderer_SetShader(r_forward_pass_shader);
	}
		
	glEnable(GL_CULL_FACE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		
	renderer_SetProjectionMatrix(&active_camera->projection_matrix);
	renderer_SetViewMatrix(&active_camera->world_to_camera_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();
	
	renderer_SetUniform1i(UNIFORM_r_width, r_width);
	renderer_SetUniform1i(UNIFORM_r_height, r_height);
	renderer_SetUniform1i(UNIFORM_r_frame, r_frame);
		
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->position);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->normal);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->tangent);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->tex_coord);
		
	brush = brushes;
	
	/* non-subtractive brushes first... */
	while(brush)
	{
		if(brush->bm_flags & BRUSH_INVISIBLE)
		{
			brush = brush->next;
			continue;
		}
		
		if(brush->bm_flags & BRUSH_SUBTRACTIVE)
		{
			brush = brush->next;
			continue;
		}
	
		if(!brush->clipped_polygons_index_count)
		{
			brush = brush->next;
			continue;
		}
					
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, brush->element_buffer);


		k = brush->batch_count;	
		batch = brush->batches;			
		for(j = 0; j < k; j++)
		{	
			renderer_SetMaterial(batch[j].material_index);		
			renderer_DrawElements(GL_TRIANGLES, batch[j].next, GL_UNSIGNED_INT, (void *)(batch[j].start * sizeof(int)));
		}
			
		brush = brush->next;
	}
		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
	renderer_SetShader(r_flat_pass_shader);
	renderer_UpdateMatrices();
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->position);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->normal);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->tangent);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), &((vertex_t *)0)->tex_coord);
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	
	brush = brushes;
	/* subtractive... */
	while(brush)
	{
		if(!(brush->bm_flags & BRUSH_SUBTRACTIVE))
		{
			brush = brush->next;
			continue;
		}
		k = brush->clipped_polygons_vert_count;
		
		polygon = brush->clipped_polygons;
		
		k = brush->start;
		while(polygon)
		{
			//glDrawArrays(GL_LINE_LOOP, k, polygon->vert_count);
			renderer_DrawArrays(GL_LINE_LOOP, k, polygon->vert_count);
			k += polygon->vert_count;
			polygon = polygon->next;
		}		
		brush = brush->next;
	}
	
	#endif 
}

void editor_LevelEditorDrawGrid()
{
	int i;
	int j;
	camera_t *active_camera = camera_GetActiveCamera();
	
	renderer_SetShader(r_imediate_color_shader);
	glLineWidth(2.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix); 
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	 
	renderer_EnableImediateDrawing();
	renderer_Begin(GL_QUADS);
	renderer_Color3f(0.3, 0.3, 0.3);
	
	for(i = 0; i <= 10; i++)
	{
		renderer_Vertex3f(-i, 0.0,-10.0);
		renderer_Vertex3f(-i, 0.0, 10.0);
		renderer_Vertex3f( i, 0.0, 10.0);
		renderer_Vertex3f( i, 0.0,-10.0);
	}
	
	for(i = 0; i <= 10; i++)
	{
		renderer_Vertex3f(-10.0, 0.0,-i);
		renderer_Vertex3f( 10.0, 0.0,-i);
		renderer_Vertex3f( 10.0, 0.0, i);
		renderer_Vertex3f(-10.0, 0.0, i);
	}
	
	renderer_End();
	renderer_DisableImediateDrawing();
	
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1.0);
	
}

void editor_LevelEditorDrawSelected()
{
	int i;
	int j;
	int r;
	
	brush_t *brush;
	bsp_polygon_t *polygon;
	pick_record_t *record;
	camera_t *active_view;
	portal_t *portal;
	struct waypoint_t *waypoint;
	
	vec3_t portal_right_vector;
	vec3_t portal_up_vector;
	vec3_t portal_center;
	
	active_view = camera_GetActiveCamera();
	
	
	//renderer_SetProjectionMatrix(&active_view)
	renderer_SetActiveView(active_view);
	renderer_SetModelMatrix(NULL);
	
	
	renderer_EnableImediateDrawing();
	
	renderer_PushStates(GL_STENCIL_TEST);
	glEnable(GL_STENCIL_TEST);
	//glCullFace(GL_FRONT);
	glDisable(GL_CULL_FACE);
	
	if(level_editor_editing_mode == EDITING_MODE_OBJECT)
	{
		for(i = 0; i < level_editor_pick_list.record_count; i++)
		{
			if(i == level_editor_pick_list.record_count - 1)
			{
				renderer_Color3f(0.2, 0.2, 1.0);
			}
			else
			{
				renderer_Color3f(0.4, 0.2, 0.6);
			}
			
			record = level_editor_pick_list.records + i;

			glClearStencil(0);
			glClear(GL_STENCIL_BUFFER_BIT);
			
			for(r = 0; r < 2; r++)
			{
				if(r == 0)
				{
					glStencilFunc(GL_ALWAYS, 0xff, 0xff);
					glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
					glDepthMask(GL_FALSE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
				else
				{
					glStencilFunc(GL_EQUAL, 0, 0xff);
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
					glDepthMask(GL_TRUE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glLineWidth(4.0);
				}
				
				switch(record->type)
				{
					case PICK_BRUSH:
						
							
						brush = record->pointer;
						polygon = brush->clipped_polygons;
						
						while(polygon)
						{
							renderer_Begin(GL_TRIANGLE_FAN);
							for(j = 0; j < polygon->vert_count; j++)
							{
								renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
							}
							renderer_End();
							
							polygon = polygon->next;
						}
					break;
					
					case PICK_WAYPOINT:
						
						waypoint = nav_waypoints + record->index0;
						
						renderer_Begin(GL_QUADS);
		
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y + 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y - 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y - 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y + 0.2, waypoint->position.z - 0.2);
						
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y + 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y - 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y - 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y + 0.2, waypoint->position.z + 0.2);
						
						
						
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y + 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y - 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y - 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y + 0.2, waypoint->position.z + 0.2);
						
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y + 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y - 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y - 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y + 0.2, waypoint->position.z + 0.2);
						
						
						
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y + 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y + 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y + 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y + 0.2, waypoint->position.z - 0.2);
						
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y - 0.2, waypoint->position.z - 0.2);
						renderer_Vertex3f(waypoint->position.x - 0.2, waypoint->position.y - 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y - 0.2, waypoint->position.z + 0.2);
						renderer_Vertex3f(waypoint->position.x + 0.2, waypoint->position.y - 0.2, waypoint->position.z - 0.2);
						
						renderer_End();
					break;
									
					case PICK_PORTAL:
						portal = &ptl_portals[record->index0];
						
						portal_center = portal->position;
						
						portal_right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
						portal_right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
						portal_right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;
						
						portal_up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
						portal_up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
						portal_up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;
						
						
						renderer_Begin(GL_QUADS);
						
						renderer_Vertex3f(portal_center.x - portal_right_vector.x + portal_up_vector.x, 
										  portal_center.y - portal_right_vector.y + portal_up_vector.y, 
										  portal_center.z - portal_right_vector.z + portal_up_vector.z);
						
						renderer_Vertex3f(portal_center.x - portal_right_vector.x - portal_up_vector.x, 
										  portal_center.y - portal_right_vector.y - portal_up_vector.y, 
										  portal_center.z - portal_right_vector.z - portal_up_vector.z);
										  
						renderer_Vertex3f(portal_center.x + portal_right_vector.x - portal_up_vector.x, 
										  portal_center.y + portal_right_vector.y - portal_up_vector.y, 
										  portal_center.z + portal_right_vector.z - portal_up_vector.z);				  
						
						renderer_Vertex3f(portal_center.x + portal_right_vector.x + portal_up_vector.x, 
										  portal_center.y + portal_right_vector.y + portal_up_vector.y, 
										  portal_center.z + portal_right_vector.z + portal_up_vector.z);
										  
						renderer_End();
					break;
				}
			}
				
			glLineWidth(1.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		
		}
	}
	else
	{
		
		if(level_editor_pick_list.record_count)
		{
			record = &level_editor_pick_list.records[level_editor_pick_list.record_count - 1];
			
			if(record->type == PICK_BRUSH)
			{
				renderer_Color3f(1.0, 0.5, 0.0);
			
				for(r = 0; r < 2; r++)
				{
					if(r == 0)
					{
						glStencilFunc(GL_ALWAYS, 0xff, 0xff);
						glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
						glDepthMask(GL_FALSE);
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}
					else
					{
						glStencilFunc(GL_EQUAL, 0, 0xff);
						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
						glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
						glDepthMask(GL_TRUE);
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
						glLineWidth(4.0);
					}
					
					brush = record->pointer;
					polygon = brush->clipped_polygons;
								
					while(polygon)
					{
						renderer_Begin(GL_TRIANGLE_FAN);
						for(j = 0; j < polygon->vert_count; j++)
						{
							renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
						}
						renderer_End();
							
						polygon = polygon->next;
					}
				}
				
				
							
				
				
				if(level_editor_brush_face_pick_list.record_count)
				{
					record = &level_editor_brush_face_pick_list.records[level_editor_brush_face_pick_list.record_count - 1];	
	
					switch(record->type)
					{
						case PICK_BRUSH_FACE:
							
							brush = record->pointer;
							polygon = &brush->base_polygons[record->index0];
							
							renderer_Color4f(1.0, 0.0, 0.0, 0.25);
							glEnable(GL_BLEND);
							glDisable(GL_STENCIL_TEST);
							glDisable(GL_DEPTH_TEST);
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
							
							renderer_Begin(GL_TRIANGLE_FAN);
							for(j = 0; j < polygon->vert_count; j++)
							{
								renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
							}
							renderer_End();
							
							glDisable(GL_BLEND);
							glEnable(GL_DEPTH_TEST);
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
							
							renderer_Begin(GL_LINE_LOOP);
							for(j = 0; j < polygon->vert_count; j++)
							{
								renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
							}
							renderer_End();
							
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						break;	
					}
					
				}	
			}
				
			
				
		}
		
		
		
	}
	
	
	
	//glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	
	renderer_DisableImediateDrawing();
	
	renderer_PopStates(GL_STENCIL_TEST);
		
}

void editor_LevelEditorDrawCursors()
{	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ed_cursors_framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, r_width, r_height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	//renderer_SaveCapability(GL_STENCIL_TEST);
	//renderer_SaveStencilFunc();
	//renderer_SaveStencilOp();
	
	renderer_PushStates(GL_STENCIL_TEST);
	
	if(level_editor_draw_3d_handle)
	{
		editor_Draw3dHandle(level_editor_3d_handle_position, level_editor_3d_handle_transform_mode);
	}
	
	editor_Draw3dCursor(level_editor_3d_cursor_position);
	
	renderer_BindBackbuffer(0, 0);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ed_cursors_framebuffer_id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, r_width, r_height, 0, 0, r_width, r_height, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0xff, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	renderer_SetShader(r_blit_texture_shader);
	renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, ed_cursors_color_texture_id);
	renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
	renderer_EnableImediateDrawing();
	renderer_Rectf(-1.0, -1.0, 1.0, 1.0);
	renderer_DisableImediateDrawing();
	glDisable(GL_STENCIL_TEST);
	
	/*renderer_RestoreCapability(GL_STENCIL_TEST);
	renderer_RestoreStencilFunc();
	renderer_RestoreStencilOp();*/
	
	renderer_PopStates(GL_STENCIL_TEST);
}
 
void editor_LevelEditorDrawLights()
{
	int i;
	int c = l_light_list_cursor;
	camera_t *active_camera = camera_GetActiveCamera();
	
	glPointSize(12.0);
	glEnable(GL_POINT_SMOOTH);

	renderer_SetShader(r_imediate_color_shader);
	renderer_EnableImediateDrawing();
	renderer_Begin(GL_POINTS);
	renderer_Color3f(0.5, 0.5, 1.0);
	for(i = 0; i < c; i++)
	{
		if(l_light_params[i].bm_flags & LIGHT_INVALID)
			continue;
		renderer_Vertex3f(l_light_positions[i].position.x, l_light_positions[i].position.y, l_light_positions[i].position.z);
	}
	
	renderer_End();
	renderer_DisableImediateDrawing();
	glPointSize(1.0);
	glDisable(GL_POINT_SMOOTH);
} 

void editor_LevelEditorDrawSpawnPoints()
{
	
}
 
void editor_LevelEditorDrawLeaves()
{
	
}

void editor_LevelEditorDrawWorldPolygons()
{
	
}

void editor_LevelEditorDrawClippedPolygons()
{
	
}

void editor_LevelEditorDrawEntitiesAabbs()
{
	
}

void editor_LevelEditorDrawBrush(brush_t *brush)
{
	
}













