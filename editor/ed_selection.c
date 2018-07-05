#include <string.h>

#include "ed_selection.h"
#include "GL/glew.h"
//#include "r_main.h"
//#include "r_imediate.h"
#include "model.h"
#include "camera.h"
#include "material.h"
#include "brush.h"
#include "entity.h"
#include "l_main.h"
#include "player.h"
#include "input.h"
#include "memory.h"
#include "physics.h"
#include "..\..\common\portal.h"
#include "..\..\common\r_main.h"
#include "..\..\common\r_imediate.h"
#include "..\..\common\r_debug.h"
#include "..\..\common\r_shader.h"
#include "..\..\common\navigation.h"

#include "ed_globals.h"

/* from r_main.c */
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;


/* from r_imediate.c */
extern int r_imediate_color_shader;

/* from brush.c */
extern brush_t *brushes;


/* from portal.c */
extern int ptl_portal_list_cursor;
extern portal_t *ptl_portals;


/* from entity.c */
extern int ent_entity_list_cursor;
extern struct entity_t *ent_entities;


/* from l_main.c */
extern int l_light_list_cursor;
extern light_params_t *l_light_params;
extern light_position_t *l_light_positions;


extern int nav_waypoint_count;
extern struct waypoint_t *nav_waypoints;


/* from player.c */
extern int spawn_point_count;
extern spawn_point_t *spawn_points;

/* from input.c */
extern int bm_mouse;



extern vec3_t level_editor_3d_handle_position;



unsigned int prev_fbo;
unsigned int prev_draw_buffer;
int prev_viewport[4];
void editor_EnablePicking()
{
	//renderer_PushFunctionName("editor_EnablePicking");
	R_DBG_PUSH_FUNCTION_NAME();
	
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_fbo);
	glGetIntegerv(GL_DRAW_BUFFER, &prev_draw_buffer);
	glGetIntegerv(GL_VIEWPORT, prev_viewport);
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ed_pick_framebuffer_id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ed_pick_framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, r_window_width, r_window_height);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	R_DBG_POP_FUNCTION_NAME();
	//renderer_PopFunctionName();	
}

void editor_SamplePickingBuffer(float mouse_x, float mouse_y, int *sample)
{
	R_DBG_PUSH_FUNCTION_NAME();
	
	int x = r_window_width * (mouse_x * 0.5 + 0.5);
	int y = r_window_height * (mouse_y * 0.5 + 0.5);
	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, sample);
	
	R_DBG_POP_FUNCTION_NAME();
}
 
void editor_DisablePicking()
{	
	//renderer_PushFunctionName("editor_DisablePicking");
	
	R_DBG_PUSH_FUNCTION_NAME();
	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_fbo);
	glDrawBuffer(prev_draw_buffer);
	glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
	
	R_DBG_POP_FUNCTION_NAME();
	//renderer_PopFunctionName();
}

/*
===============================================================
===============================================================
===============================================================
*/

pick_record_t editor_PickObject(float mouse_x, float mouse_y)
{
	int i;
	int c;
	//int j;
	//int k;
	//int x;
	//int y;
	
	int pick_type;
	int pick_index0;
	int pick_index1;
	int pick_index2;
	
	pick_record_t record;
	pick_record_t *records;
	struct model_t *model;
	struct waypoint_t *waypoint;
	mesh_t *mesh;
	portal_t *portal;
	
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t center;
	
	//int lshift;
	
	//int start;
	
	//vec3_t pos;
	mat4_t transform;
	
	//renderer_PushFunctionName("editor_PickObject");
	R_DBG_PUSH_FUNCTION_NAME();
	
	
	camera_t *active_camera = camera_GetActiveCamera();
	//triangle_group_t *triangle_group;
	//batch_t *batch;
	//material_t *material;
	//bsp_polygon_t *polygon;
	brush_t *brush;
	
	//unsigned int q[4];
	unsigned int pick_sample[4];
	
	int value;
	
	editor_EnablePicking();
	
	gpu_BindGpuHeap();
	
	renderer_SetShader(ed_pick_shader);
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_EnableImediateDrawing();
	
	value = PICK_BRUSH;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);
		
	brush = brushes;
	i = 1;
	while(brush)
	{	
		value = i;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);
		renderer_DrawVertsIndexed(GL_TRIANGLES, brush->clipped_polygons_index_count, 4, sizeof(vertex_t), brush->clipped_polygons_vertices, brush->clipped_polygons_indexes);
		brush = brush->next;
		i++;
	}
	
	c = ent_entity_list_cursor;
	
	value = PICK_ENTITY;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);
	
	for(i = 0; i < c; i++)
	{
		/*if(ent_entities[i].flags & ENTITY_INVALID)
			continue;
		
		if(ent_entities[i].flags & ENTITY_INVISIBLE)
			continue;	
			
		value = i + 1;	
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);	
		
		model = model_GetModelPointerIndex(ent_entities[i].model_index);
		mesh = model->mesh;
		mat4_t_compose(&transform, &ent_entities[i].orientation, ent_entities[i].position);
		mat4_t_scale_axis_aligned(&transform, ent_entities[i].scale);
		
		renderer_SetModelMatrix(&transform);
		renderer_DrawVerts(GL_TRIANGLES, mesh->vert_count, 4, sizeof(vertex_t), mesh->vertices);*/
	}

		
	c = l_light_list_cursor;
	glPointSize(24.0);
	glEnable(GL_POINT_SMOOTH);
	
	value = PICK_LIGHT;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);
		
	for(i = 0; i < c; i++)
	{
		if(l_light_params[i].bm_flags & LIGHT_INVALID)
			continue;
			
		value = i + 1;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);
		
		renderer_Begin(GL_POINTS);
		renderer_Vertex3f(l_light_positions[i].position.x, l_light_positions[i].position.y, l_light_positions[i].position.z);
		renderer_End();
	}
	
	glDisable(GL_POINT_SMOOTH);
	glPointSize(1.0);


	value = PICK_PORTAL;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);
	renderer_SetModelMatrix(NULL);
	
	glDisable(GL_CULL_FACE);
	
	for(i = 0; i < ptl_portal_list_cursor; i++)
	{		
		portal = &ptl_portals[i];
		
		right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
		right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
		right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;
		
		up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
		up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
		up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;
		
		center = portal->position;
		
		value = i + 1;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);

		renderer_Begin(GL_QUADS);
		renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);
		renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);
		renderer_End();
	}
	
	value = PICK_WAYPOINT;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);
	renderer_SetModelMatrix(NULL);
	
	for(i = 0; i < nav_waypoint_count; i++)
	{	
		waypoint = nav_waypoints + i;	
		
		value = i + 1;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);
		
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
		
		/*portal = &ptl_portals[i];
		
		right_vector.x = portal->orientation.floats[0][0] * portal->extents.x;
		right_vector.y = portal->orientation.floats[1][0] * portal->extents.x;
		right_vector.z = portal->orientation.floats[2][0] * portal->extents.x;
		
		up_vector.x = portal->orientation.floats[0][1] * portal->extents.y;
		up_vector.y = portal->orientation.floats[1][1] * portal->extents.y;
		up_vector.z = portal->orientation.floats[2][1] * portal->extents.y;
		
		center = portal->position;
		
		value = i + 1;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);

		renderer_Begin(GL_QUADS);
		renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);
		renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);
		renderer_End();*/
	}
	
	
	
	renderer_DisableImediateDrawing();
	glEnable(GL_CULL_FACE);	

	
	editor_SamplePickingBuffer(mouse_x, mouse_y, pick_sample);
						 
	editor_DisablePicking();
	
	record.type = pick_sample[0];
	
	switch(record.type)
	{
		case PICK_BRUSH:
			brush = brushes;
			i = 1;
			while(brush)
			{
				if(i == pick_sample[1])
				{
					break;
				}
				brush = brush->next;
				i++;
			}
			record.pointer = brush;
		break;
		
		
		case PICK_LIGHT:
		case PICK_SPAWN_POINT:
		case PICK_ENTITY:
		case PICK_PORTAL:
		case PICK_WAYPOINT:
			record.index0 = pick_sample[1] - 1;
		break;
	}
		
	gpu_UnbindGpuHeap();	
	R_DBG_POP_FUNCTION_NAME();
	
	return record;
}

pick_record_t editor_PickBrushFace(brush_t *brush, float mouse_x, float mouse_y)
{
	int i;
	int c;
	int j;
	//int j;
	//int k;
	//int x;
	//int y;
	
	int pick_type;
	int pick_index0;
	int pick_index1;
	int pick_index2;
	
	pick_record_t record;
	pick_record_t *records;
	//model_t *model;
	//mesh_t *mesh;
	//portal_t *portal;
	
	//vec3_t right_vector;
	//vec3_t up_vector;
	//vec3_t center;
	
	//int lshift;
	
	//int start;
	
	//vec3_t pos;
	//mat4_t transform;
	
	//renderer_PushFunctionName("editor_PickObject");
	R_DBG_PUSH_FUNCTION_NAME();
	
	
	camera_t *active_camera = camera_GetActiveCamera();
	//triangle_group_t *triangle_group;
	//batch_t *batch;
	//material_t *material;
	//bsp_polygon_t *polygon;
	//brush_t *brush;
	bsp_polygon_t *polygon;
	
	//unsigned int q[4];
	unsigned int pick_sample[4];
	
	int value;
	
	editor_EnablePicking();
	
	//gpu_BindGpuHeap();
	
	renderer_SetShader(ed_pick_shader);
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_EnableImediateDrawing();
	
	value = PICK_BRUSH_FACE;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);
		
	//brush = brushes;
	//i = 1;
	//while(brush)
	//{	
	
	for(c = 0; c < brush->base_polygons_count; c++)
	{
		value = c + 1;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);
			
		polygon = &brush->base_polygons[c];
		renderer_Begin(GL_TRIANGLE_FAN);
		for(j = 0; j < polygon->vert_count; j++)
		{
			renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
		}
		renderer_End();
	}
		
		
		
		
		//renderer_DrawVertsIndexed(GL_TRIANGLES, brush->clipped_polygons_index_count, 4, sizeof(vertex_t), brush->clipped_polygons_vertices, brush->clipped_polygons_indexes);
	//	brush = brush->next;
	//	i++;
	//}
	
	
	renderer_DisableImediateDrawing();
	glEnable(GL_CULL_FACE);	

	
	editor_SamplePickingBuffer(mouse_x, mouse_y, pick_sample);
						 
	editor_DisablePicking();
	
	record.type = pick_sample[0];
	
	//printf("%d %d\n", pick_sample[0], pick_sample[1]);
	switch(record.type)
	{
		case PICK_BRUSH_FACE:
			record.pointer = brush;
			record.index0 = pick_sample[1] - 1;
		break;
	}
		
	//gpu_UnbindGpuHeap();	
	R_DBG_POP_FUNCTION_NAME();
	
	return record;
}

/*
===============================================================
===============================================================
===============================================================
*/

int editor_Check3dHandle(double mouse_x, double mouse_y, vec3_t handle_position, int mode)
{	
	int x;
	int y;	
	float q[4];
	
	int flags = 0; 
	
	//renderer_PushFunctionName();
	
	R_DBG_PUSH_FUNCTION_NAME();
	
	editor_EnablePicking();
	editor_Draw3dHandle(handle_position, mode);	
	
	x = r_window_width * (mouse_x * 0.5 + 0.5);
	y = r_window_height * (mouse_y * 0.5 + 0.5);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, q);
	
	editor_DisablePicking();
			
	if(q[0])
	{
		flags |= ED_3D_HANDLE_X_AXIS_GRABBED;
	}
		
	if(q[1])
	{
		flags |= ED_3D_HANDLE_Y_AXIS_GRABBED;
	}
		
	if(q[2])
	{
		flags |= ED_3D_HANDLE_Z_AXIS_GRABBED;
	}
	
	//renderer_PopFunctionName();
	
	R_DBG_POP_FUNCTION_NAME();
	return flags;
	
}


float editor_GetMouseOffsetFrom3dHandle(float mouse_x, float mouse_y, vec3_t handle_position, vec3_t axis, int transform_mode, float linear_snap_value, float angular_snap_value)
{
	
	vec4_t p;
	vec4_t sp;
	vec3_t direction;
	float d;
	float z;
	float amount = 0.0;
	mat4_t model_view_projection_matrix;
	camera_t *active_camera = camera_GetActiveCamera();
	
	static float grab_screen_offset_x = 0.0;
	static float grab_screen_offset_y = 0.0;
	static float screen_dx = 0.0;
	static float screen_dy = 0.0;
	static float screen_x = 0.0;
	static float screen_y = 0.0;
	static float prev_dx = 0.0;
	static float prev_dy = 0.0;
	
	
	p.vec3 = handle_position;
	p.w = 1.0;
	mat4_t_mult_fast(&model_view_projection_matrix, &active_camera->view_data.view_matrix, &active_camera->view_data.projection_matrix);
	mat4_t_vec4_t_mult(&model_view_projection_matrix, &p);
				
					
	p.x /= p.w;
	p.y /= p.w;
	z = p.z;
					
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		if(transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE || transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
		{
			grab_screen_offset_x = mouse_x - p.x;
			grab_screen_offset_y = mouse_y - p.y;
		}
		else
		{
			grab_screen_offset_x = 0.0;
			grab_screen_offset_y = 0.0;
		}
						
	}
				
	screen_dx = mouse_x - p.x - grab_screen_offset_x;
	screen_dy = mouse_y - p.y - grab_screen_offset_y;
				
	/*if(entity_editor_3d_handle_flags & ED_3D_HANDLE_X_AXIS_GRABBED)
	{
		direction = vec3(1.0, 0.0, 0.0);
	}
	else if(entity_editor_3d_handle_flags & ED_3D_HANDLE_Y_AXIS_GRABBED)
	{
		direction = vec3(0.0, 1.0, 0.0);
	}
	else if(entity_editor_3d_handle_flags & ED_3D_HANDLE_Z_AXIS_GRABBED)
	{
		direction = vec3(0.0, 0.0, 1.0);
	}*/
	
	direction = axis;
				
	if(transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE || transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
	{
		sp = p;	
		p.vec3 = direction;
		p.w = 0.0;
				
		mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &p);
			
		screen_x = p.x;
		screen_y = p.y;
					
		amount = sqrt(screen_x * screen_x + screen_y * screen_y);
				
		screen_x /= amount;
		screen_y /= amount;
			
		amount = (screen_dx * screen_x + screen_dy * screen_y) * z;
					
					
		if(linear_snap_value > 0.0)
		{
			d = amount / linear_snap_value;
	 		amount = linear_snap_value * (int)d;
		}	
					
		if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
		{
			if(transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE)
			{
				if(fabs(amount) > 0.0)
				{
					grab_screen_offset_x = mouse_x - sp.x;
					grab_screen_offset_y = mouse_y - sp.y;
				}
					
			}
		}
				
	}
	else
	{
				
		amount = sqrt(screen_dx * screen_dx + screen_dy * screen_dy);
				
		screen_dx /= amount;
		screen_dy /= amount;
					
		if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
		{
			prev_dx = screen_dx;
			prev_dy = screen_dy;
		}
						
		amount = asin(prev_dx * screen_dy - prev_dy * screen_dx);
					
		d = dot3(direction, active_camera->world_orientation.f_axis);
					
		if(d < 0)
		{
			amount = -amount;
		}
		else if(d == 0)
		{
			d = dot3(direction, active_camera->world_orientation.r_axis);
					
			if(d < 0)
			{
				amount = -amount;
			}
		}
					
		if(angular_snap_value > 0.0)
		{
			d = amount / angular_snap_value;
			amount = angular_snap_value * (int)d;
		}
				
		if(amount != 0.0)
		{
			if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
			{
				prev_dx = screen_dx;
				prev_dy = screen_dy;
			}
		}
	}
	
	return amount;
}


/*
===============================================================
===============================================================
===============================================================
*/


void editor_AddSelection(pick_record_t *record, pick_list_t *pick_list)
{
	
	pick_record_t *records;
	
	
	if(record->type == PICK_NONE)
	{
		return;
	}
	
	/* try to drop this selection, to make sure
	there's just one selection per object... */
	editor_DropSelection(record, pick_list);
	
	
	if(pick_list->record_count >= pick_list->max_records)
	{
		records = memory_Malloc(sizeof(pick_record_t) * (pick_list->max_records + 256), "editor_AddSelection");
		memcpy(records, pick_list->records, sizeof(pick_record_t) * pick_list->max_records);
		memory_Free(pick_list->records);
		pick_list->records = records;
		pick_list->max_records += 256;
	}
	
	pick_list->records[pick_list->record_count] = *record;
	pick_list->record_count++;
	pick_list->last_selection_type = record->type;
}



void editor_DropSelection(pick_record_t *record, pick_list_t *pick_list)
{
	int i;
	int c;
	int j;
	int k;
	
	pick_record_t *records = pick_list->records;
	
	if(record->type == PICK_NONE)
	{
		return;
	}
	
	c = pick_list->record_count;
	
	for(i = 0; i < c; i++)
	{
		if(record->type == records[i].type)
		{
			if(record->type == PICK_BRUSH)
			{
				if(record->pointer == records[i].pointer)
				{
					goto _move_selections;
				}
				
				continue;
			}
			
			if(record->index0 == records[i].index0)
			{
				_move_selections:
					
				for(j = i; j < c - 1; j++)
				{
					records[j] = records[j + 1];
				}
				
				pick_list->record_count--;	
				break;
			}
		}
	}
	
	pick_list->last_selection_type = records[pick_list->record_count - 1].type;
	
	/*if(ed_selection_count)
	{
		switch(ed_selections[ed_selection_count - 1].type)
		{
			case PICK_LIGHT:
				ed_selected_light_params = &l_light_params[ed_selections[ed_selection_count - 1].index0];
				ed_selected_light_position = &l_light_positions[ed_selections[ed_selection_count - 1].index0];
				
				ed_selected_brush = NULL;
				ed_selected_brush_polygon_index = -1;
				ed_selected_brush_selection_index = -1;
				
			break;
						
			case PICK_BRUSH:
				ed_selected_brush = ed_selections[ed_selection_count - 1].pointer;				
			break;
		}
					
		ed_selection_type = ed_selections[ed_selection_count - 1].type;
	}
	else
	{
		ed_selection_type = PICK_NONE;
	}*/
}

void editor_ClearSelection(pick_list_t *pick_list)
{
	
	/*if(ed_editing_mode == EDITING_MODE_UV)
	{
		ed_selection_count = ed_selected_brush_selection_index + 1;
		ed_selection_type = PICK_BRUSH;
	}
	else
	{
		ed_selection_count = 0;
		ed_selection_type = PICK_NONE;
		
		ed_selected_light_params = NULL;
		ed_selected_light_position = NULL;
		
		ed_selected_brush = NULL;
		ed_selected_brush_polygon_index = -1;
		ed_selected_brush_polygon_vertex_index = -1;
		ed_selected_brush_selection_index = -1;
	}
	
	editor_CloseLightPropertiesWindow();
	editor_CloseBrushPropertiesWindow();*/
	
	
	pick_list->record_count = 0;
	pick_list->last_selection_type = PICK_NONE;
	
}

pick_record_t editor_GetLastSelection(pick_list_t *pick_list)
{
	if(pick_list->record_count)
	{
		return pick_list->records[pick_list->record_count - 1];
	}
	
	return (pick_record_t){PICK_NONE, -1, NULL, -1, -1};
}

mat3_t editor_GetLastSelectionOrientation()
{
	
}

vec3_t editor_GetLastSelectionPosition()
{
	
}

/*
===============================================================
===============================================================
===============================================================
*/

void editor_TranslateSelections(pick_list_t *pick_list, vec3_t direction, float amount)
{
	int i;
	int c;
	int k;
	
	vec3_t v;
	pick_record_t *records = pick_list->records;
	collision_shape_t *collision_shapes = NULL;
	brush_t *brush;
	bsp_polygon_t *polygon;
	c = pick_list->record_count;
	
	
	for(i = 0; i < c; i++)
	{
		switch(records[i].type)
		{
			case PICK_BRUSH:
				
				v.x = direction.x * amount;
				v.y = direction.y * amount;
				v.z = direction.z * amount;
				
				brush_TranslateBrush(records[i].pointer, v);
			break;
			
			case PICK_BRUSH_FACE:
				brush = records[i].pointer;
				polygon = &brush->base_polygons[records[i].index0];
				
				for(k = 0; k < polygon->vert_count; k++)
				{
					polygon->vertices[i].position.x += direction.x * amount;
					polygon->vertices[i].position.y += direction.y * amount;
					polygon->vertices[i].position.z += direction.z * amount;
				}
				
				brush->bm_flags |= BRUSH_MOVED;
			break;
			
			case PICK_BRUSH_EDGE:
			
			break;
			
			case  PICK_LIGHT:
				light_TranslateLight(records[i].index0, direction, amount);
			break;
			
			case PICK_SPAWN_POINT:
//				spawn_points[records[i].index0].position.x += direction.x * amount;
//				spawn_points[records[i].index0].position.y += direction.y * amount;
//				spawn_points[records[i].index0].position.z += direction.z * amount;
			break;
			
			case PICK_UV_VERTEX:
			
			break;
			
			case PICK_ENTITY:
				entity_TranslateEntity(records[i].index0, direction, amount);
			break;
			
			case PICK_PORTAL:
				portal_TranslatePortal(records[i].index0, direction, amount);
			break;
			
			case PICK_WAYPOINT:
				
				navigation_MoveWaypoint(records[i].index0, direction, amount);
				
				/*nav_waypoints[records[i].index0].position.x += direction.x * amount;
				nav_waypoints[records[i].index0].position.y += direction.y * amount;
				nav_waypoints[records[i].index0].position.z += direction.z * amount;*/
			break;	
		}
	}

}

void editor_RotateSelections(pick_list_t *pick_list, vec3_t axis, float amount, vec3_t handle_position)
{
	int i;
	int c;
	brush_t *brush;
	light_position_t *light;
	pick_record_t *records;
	
	vec3_t v;
	
	mat3_t rot;
	
	mat3_t_rotate(&rot, axis, amount, 1);
	
	records = pick_list->records;
	c = pick_list->record_count;
	
	for(i = 0; i < c; i++)
	{
		switch(records[i].type)
		{
			case PICK_BRUSH:
				
				//brush = &brushes[selections[i].index0];
				brush = (brush_t *)records[i].pointer;
				
				brush_RotateBrush(brush, axis, amount);
				v = brush->position;
				
				v.x -= handle_position.x;
				v.y -= handle_position.y;
				v.z -= handle_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				//v = MultiplyVector3(&rot, v);
				
				v.x += handle_position.x;
				v.y += handle_position.y;
				v.z += handle_position.z;
				
				
				v.x = v.x - brush->position.x;
				v.y = v.y - brush->position.y;
				v.z = v.z - brush->position.z;
				
				brush_TranslateBrush(brush, v);
				
				//brush->position = v;
			break;
			
			case PICK_LIGHT:
				light = &l_light_positions[records[i].index0];
				
				v = light->position;
				
				v.x -= handle_position.x;
				v.y -= handle_position.y;
				v.z -= handle_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				//v = MultiplyVector3(&rot, v);
				
				v.x += handle_position.x;
				v.y += handle_position.y;
				v.z += handle_position.z;
				
				
				v.x = v.x - light->position.x;
				v.y = v.y - light->position.y;
				v.z = v.z - light->position.z;
				
				
				light_TranslateLight(records[i].index0, v, 1.0);
			break;
			
			
			case PICK_ENTITY:
				/*v = ent_entities[records[i].index0].position;
				
				
				entity_RotateEntity(records[i].index0, axis, amount);
				
				v.x -= handle_position.x;
				v.y -= handle_position.y;
				v.z -= handle_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				
				
				v.x += handle_position.x;
				v.y += handle_position.y;
				v.z += handle_position.z;
				
				ent_entities[records[i].index0].position = v;*/
			break;
			
			case PICK_SPAWN_POINT:
//				v = spawn_points[records[i].index0].position;
				
				v.x -= handle_position.x;
				v.y -= handle_position.y;
				v.z -= handle_position.z;
				
				mat3_t_vec3_t_mult(&rot, &v);
				
				
				v.x += handle_position.x;
				v.y += handle_position.y;
				v.z += handle_position.z;
				
//				spawn_points[records[i].index0].position = v;
				
			break;
			
			case PICK_PORTAL:
				//ptl_portals[records[i].index0].orientation = rot;
				portal_RotatePortal(records[i].index0, axis, amount);
			break;
				
		}
	}

}

void editor_ScaleSelections(pick_list_t *pick_list, vec3_t axis, float amount)
{

	int i;
	int c;
	brush_t *brush;
	pick_record_t *records;
	
	records = pick_list->records;
	c = pick_list->record_count;
	vec2_t portal_scale_axis;
	
	for(i = 0; i < c; i++)
	{
		switch(records[i].type)
		{
			case PICK_BRUSH:
				brush = records[i].pointer;
				brush_ScaleBrush(brush, axis, amount);
			break;
			
			case PICK_ENTITY:
				entity_ScaleEntity(records[i].index0, axis, amount);
			break;
			
			case PICK_PORTAL:
				portal_scale_axis.x = axis.x;
				portal_scale_axis.y = axis.y;
				portal_ScalePortal(records[i].index0, portal_scale_axis, amount);
			break;
		}
	}
}

void editor_CopySelections(pick_list_t *pick_list)
{
	
	int i;
	int c;
	int new_index;
	
	light_position_t *light_pos;
	light_params_t *light_parms;
	struct entity_t *entity;
	pick_record_t *records;
	
	records = pick_list->records;
	c = pick_list->record_count;

	
	for(i = 0; i < c; i++)
	{
		switch(records[i].type)
		{
			case PICK_BRUSH:
				//new_index = brush_CopyBrush(&brushes[records[i].index0]);
				//records[i].index0 = new_index;
				records[i].pointer = brush_CopyBrush((brush_t *)records[i].pointer);
			break;
			
			case PICK_LIGHT:
				light_pos = &l_light_positions[records[i].index0];
				light_parms = &l_light_params[records[i].index0];
				new_index = light_CreateLight("copy_light", &light_pos->orientation, light_pos->position, vec3((float)light_parms->r / 255.0, (float)light_parms->g / 255.0, (float)light_parms->b / 255.0), (float)(LIGHT_ENERGY(light_parms->energy)), (float)(LIGHT_RADIUS(light_parms->radius)), light_parms->bm_flags);
				records[i].index0 = new_index;
			break;
			
			case PICK_ENTITY:
				//entity = entity_GetEntityPointerIndex(records[i].index0);
				//records[i].index0 = entity_CopyEntity(entity);
			break;
		}
	}
}

void editor_DestroySelection(pick_list_t *pick_list)
{
	int i;
	int c;
	pick_record_t *records;
	
	if(!pick_list->record_count)
		return;
		
	records = pick_list->records;
	c = pick_list->record_count;	
		
	
	for(i = 0; i < c; i++)
	{
		switch(records[i].type)
		{
			case PICK_LIGHT:
				light_DestroyLightIndex(records[i].index0);
			break;
			
			case PICK_BRUSH:
				brush_DestroyBrush(records[i].pointer);
			break;
			
			case PICK_SPAWN_POINT:
//				player_DestroySpawnPoint(records[i].index0);
			break;
			
			case PICK_ENTITY:
				//entity_DestroyEntityIndex(records[i].index0);
			break;
		}
	}	
}









