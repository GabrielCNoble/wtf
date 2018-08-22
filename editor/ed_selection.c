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
#include "c_memory.h"
#include "physics.h"
#include "..\..\common\portal.h"
#include "..\..\common\r_main.h"
#include "..\..\common\r_imediate.h"
#include "..\..\common\r_debug.h"
#include "..\..\common\r_shader.h"
#include "..\..\common\navigation.h"
#include "..\..\common\containers\stack_list.h"

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
//extern int ent_entity_list_cursor;
extern struct stack_list_t ent_entities[2];


/* from l_main.c */
extern int l_light_list_cursor;
extern light_params_t *l_light_params;
extern light_position_t *l_light_positions;


/* from navigation.c */
extern struct stack_list_t nav_waypoints;
//extern int nav_waypoint_count;
//extern struct waypoint_t *nav_waypoints;


/* from player.c */
extern int spawn_point_count;
extern spawn_point_t *spawn_points;

/* from input.c */
extern int bm_mouse;



extern vec3_t level_editor_3d_handle_position;



unsigned int prev_fbo;
unsigned int prev_draw_buffer;
int prev_viewport[4];

 void editor_EnablePicking(int width, int height)
{
	//renderer_PushFunctionName("editor_EnablePicking");
	R_DBG_PUSH_FUNCTION_NAME();

	/*glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_fbo);
	glGetIntegerv(GL_DRAW_BUFFER, &prev_draw_buffer);
	glGetIntegerv(GL_VIEWPORT, prev_viewport);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ed_pick_framebuffer_id);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ed_pick_framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, r_window_width, r_window_height);*/

	renderer_PushFramebuffer(&ed_pick_framebuffer);
	renderer_PushViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	R_DBG_POP_FUNCTION_NAME();
	//renderer_PopFunctionName();
}

void editor_SamplePickingBuffer(float mouse_x, float mouse_y, int *sample)
{
	R_DBG_PUSH_FUNCTION_NAME();

	renderer_SampleFramebuffer(mouse_x, mouse_y, sample);
	//int x = r_window_width * (mouse_x * 0.5 + 0.5);
	//int y = r_window_height * (mouse_y * 0.5 + 0.5);
	//glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, sample);

	R_DBG_POP_FUNCTION_NAME();
}

void editor_DisablePicking()
{
	//renderer_PushFunctionName("editor_DisablePicking");

	R_DBG_PUSH_FUNCTION_NAME();

	renderer_PopViewport();
	renderer_PopFramebuffer();

	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, prev_fbo);
	//glDrawBuffer(prev_draw_buffer);
	//glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);

	R_DBG_POP_FUNCTION_NAME();
	//renderer_PopFunctionName();
}

/*
===============================================================
===============================================================
===============================================================
*/

/* may be enough, or may not... */
struct entity_handle_t entity_stack[8192];

pick_record_t editor_PickObject(float mouse_x, float mouse_y)
{
	int i;
	int c;
	int j;
	int k;
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
	struct waypoint_t *waypoints;
	mesh_t *mesh;
	portal_t *portal;

	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t center;


	struct entity_t *entities;
	struct entity_t *entity;
	struct transform_component_t *transform_component;
	struct transform_component_t *other_transform;
	struct model_component_t *model_component;
	struct entity_transform_t *entity_transform;
	vec3_t entity_box_extents;
	vec3_t entity_box_scale;
	struct model_t *entity_model;
	//entity_transform_t entity_transform;

	int cur_top = -1;
	int next_top = -1;


	//int lshift;

	//int start;

	//vec3_t pos;
	mat4_t transform;

	//renderer_PushFunctionName("editor_PickObject");
	R_DBG_PUSH_FUNCTION_NAME();

	//vec3_t box[8];



	camera_t *active_camera = camera_GetActiveCamera();
	//triangle_group_t *triangle_group;
	//batch_t *batch;
	//material_t *material;
	//bsp_polygon_t *polygon;
	brush_t *brush;

	//unsigned int q[4];
	unsigned int pick_sample[4];

	int value;

	editor_EnablePicking(r_window_width, r_window_height);

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


	entities = (struct entity_t *)ent_entities[0].elements;
	c = ent_entities[0].element_count;

	value = PICK_ENTITY;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);

	for(i = 0; i < c; i++)
	{
		entity = entities + i;

		if(entity->flags & ENTITY_FLAG_INVALID)
		{
			continue;
		}

		transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

        if(entity_GetComponentPointer(transform_component->parent))
		{
			/* not the top of a hierarchy... */
			continue;
		}

		value = i + 1;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);

		next_top = 0;
		entity_stack[next_top].def = 0;
		entity_stack[next_top].entity_index = i;

		j = 0;

		do
		{
			cur_top = next_top;

            for(; j <= cur_top; j++)
			{
				entity = entity_GetEntityPointerHandle(entity_stack[j]);

				if(entity)
				{
					/* draw it... */

					model_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_MODEL]);

					if(!model_component)
					{
						entity_box_extents.x = 0.25;
						entity_box_extents.y = 0.25;
						entity_box_extents.z = 0.25;
					}
					else
					{
						entity_model = model_GetModelPointerIndex(model_component->model_index);
                        entity_box_extents = entity_model->aabb_max;
					}

					transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
					entity_transform = entity_GetWorldTransformPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

					transform = entity_transform->transform;

					transform.floats[0][0] *= entity_box_extents.x;
					transform.floats[0][1] *= entity_box_extents.x;
					transform.floats[0][2] *= entity_box_extents.x;

					transform.floats[1][0] *= entity_box_extents.y;
					transform.floats[1][1] *= entity_box_extents.y;
					transform.floats[1][2] *= entity_box_extents.y;

					transform.floats[2][0] *= entity_box_extents.z;
					transform.floats[2][1] *= entity_box_extents.z;
					transform.floats[2][2] *= entity_box_extents.z;

					//entity_box_scale.x = transform_component->scale.x * entity_box_extents.x;
					//entity_box_scale.y = transform_component->scale.y * entity_box_extents.y;
					//entity_box_scale.z = transform_component->scale.z * entity_box_extents.z;

					//mat4_t_compose2(&transform, &transform_component->orientation, transform_component->position, entity_box_scale);

                    renderer_SetModelMatrix(&transform);

					renderer_DrawBox();



					for(k = 0; k < transform_component->children_count; k++)
					{
						/* add whatever child entities it may have... */
						other_transform = entity_GetComponentPointer(transform_component->child_transforms[k]);

						if(other_transform->base.entity.entity_index != INVALID_ENTITY_INDEX)
						{
                            next_top++;
							entity_stack[next_top] = other_transform->base.entity;
						}
					}

				}
			}

		}
		while(cur_top != next_top);
	}

	renderer_SetModelMatrix(NULL);

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

/*
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
	}*/

	value = PICK_WAYPOINT;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);
	renderer_SetModelMatrix(NULL);

	waypoints = (struct waypoint_t *)nav_waypoints.elements;
	c = nav_waypoints.element_count;

	for(i = 0; i < c; i++)
	{
		waypoint = waypoints + i;

		value = i + 1;
		renderer_SetNamedUniform1f("pick_index", *(float *)&value);

		mat4_t_compose2(&transform, NULL, waypoint->position, vec3_t_c(0.2, 0.2, 0.2));

		renderer_SetModelMatrix(&transform);
		renderer_DrawBox();
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
			printf("pick!\n");
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

	pick_record_t record;
	pick_record_t *records;

	R_DBG_PUSH_FUNCTION_NAME();

	camera_t *active_camera = camera_GetActiveCamera();
	bsp_polygon_t *polygon;

	unsigned int pick_sample[4];

	int value;

	editor_EnablePicking(r_window_width, r_window_height);

	renderer_SetShader(ed_pick_shader);
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_EnableImediateDrawing();

	value = PICK_BRUSH_FACE;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);

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

	renderer_DisableImediateDrawing();
	glEnable(GL_CULL_FACE);

	editor_SamplePickingBuffer(mouse_x, mouse_y, pick_sample);

	editor_DisablePicking();

	record.type = pick_sample[0];
	switch(record.type)
	{
		case PICK_BRUSH_FACE:
			record.pointer = brush;
			record.index0 = pick_sample[1] - 1;
		break;
	}
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

	editor_EnablePicking(r_window_width, r_window_height);
	editor_Draw3dHandle(handle_position, mode);

	editor_SamplePickingBuffer(mouse_x, mouse_y, (int *)q);
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

vec3_t editor_3dCursorPosition(float mouse_x, float mouse_y)
{
	int i;
	int c;
	int j;
	int k;
	int x;
	int y;

	camera_t *active_camera = camera_GetActiveCamera();
	brush_t *brush;

	mat4_t camera_to_world_matrix;

	vec4_t pos;

	float q[4];
	float z;
	float qr;
	float qt;

	gpu_BindGpuHeap();

	editor_EnablePicking(r_window_width, r_window_height);
	glClearColor(active_camera->frustum.zfar, active_camera->frustum.zfar, active_camera->frustum.zfar, active_camera->frustum.zfar);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	renderer_SetShader(ed_brush_dist_shader);
	renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_EnableImediateDrawing();

	brush = brushes;
	while(brush)
	{
		renderer_DrawVertsIndexed(GL_TRIANGLES, brush->clipped_polygons_index_count, 4, sizeof(vertex_t), brush->clipped_polygons_vertices, brush->clipped_polygons_indexes);
		brush = brush->next;
	}

	editor_SamplePickingBuffer(mouse_x, mouse_y, (int *)q);

	editor_DisablePicking();
	mat4_t_compose(&camera_to_world_matrix, &active_camera->world_orientation, active_camera->world_position);
	qr = active_camera->frustum.znear / active_camera->frustum.right;
	qt = active_camera->frustum.znear / active_camera->frustum.top;

	if(q[0] == active_camera->frustum.zfar)
	{
		z = -10.0;
	}
	else
	{
		z = -q[0];
	}

	pos.x = (mouse_x / qr) * (-z);
	pos.y = (mouse_y / qt) * (-z);
	pos.z = z;
	pos.w = 1.0;

	mat4_t_vec4_t_mult(&camera_to_world_matrix, &pos);

	return pos.vec3;
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

	return amount * 0.9;
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
		records = memory_Malloc(sizeof(pick_record_t) * (pick_list->max_records + 256));
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
}

void editor_ClearSelection(pick_list_t *pick_list)
{
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
	struct collision_shape_t *collision_shapes = NULL;
	struct entity_handle_t entity_handle;
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

				entity_handle.def = 0;
				entity_handle.entity_index = records[i].index0;

				entity_TranslateEntity(entity_handle, direction, amount);
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
				new_index = light_CreateLight("copy_light", &light_pos->orientation, light_pos->position, vec3_t_c((float)light_parms->r / 255.0, (float)light_parms->g / 255.0, (float)light_parms->b / 255.0), (float)(UNPACK_LIGHT_ENERGY(light_parms->energy)), (float)(UNPACK_LIGHT_RADIUS(light_parms->radius)), light_parms->bm_flags);
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

	struct entity_handle_t entity_handle;

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
				entity_handle.entity_index = records[i].index0;
				entity_handle.def = 0;

				entity_RemoveEntity(entity_handle);
				//entity_DestroyEntityIndex(records[i].index0);
			break;
		}
	}

	pick_list->record_count = 0;
}

vec3_t editor_GetCenterOfSelections(pick_list_t *pick_list)
{
	int i;

	vec3_t center;

	center.x = 0.0;
	center.y = 0.0;
	center.z = 0.0;

	struct waypoint_t *waypoint;
	struct entity_t *entity;
	struct entity_handle_t entity_handle;

	struct entity_transform_t *world_transform;
	//vec3_t entity_position;


	//waypoints = (struct waypoint_t *)nav_waypoints.elements;

	brush_t *brush;

	for(i = 0; i < pick_list->record_count; i++)
	{
		switch(pick_list->records[i].type)
		{
			case PICK_LIGHT:
				center.x += l_light_positions[pick_list->records[i].index0].position.x;
				center.y += l_light_positions[pick_list->records[i].index0].position.y;
				center.z += l_light_positions[pick_list->records[i].index0].position.z;
			break;

			case PICK_BRUSH:
				brush = (brush_t *)pick_list->records[i].pointer;
				center.x += brush->position.x;
				center.y += brush->position.y;
				center.z += brush->position.z;
			break;

			case PICK_SPAWN_POINT:
//				level_editor_3d_handle_position.x += spawn_points[level_editor_pick_list.records[i].index0].position.x;
	//			level_editor_3d_handle_position.y += spawn_points[level_editor_pick_list.records[i].index0].position.y;
//				level_editor_3d_handle_position.z += spawn_points[level_editor_pick_list.records[i].index0].position.z;
			break;

			case PICK_ENTITY:

				entity_handle.entity_index = pick_list->records[i].index0;
				entity_handle.def = 0;

				entity = entity_GetEntityPointerHandle(entity_handle);

				world_transform = entity_GetWorldTransformPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

				center.x += world_transform->transform.floats[3][0];
				center.y += world_transform->transform.floats[3][1];
				center.z += world_transform->transform.floats[3][2];
			break;

			case PICK_PORTAL:
				center.x += ptl_portals[pick_list->records[i].index0].position.x;
				center.y += ptl_portals[pick_list->records[i].index0].position.y;
				center.z += ptl_portals[pick_list->records[i].index0].position.z;
			break;

			case PICK_WAYPOINT:

				waypoint = navigation_GetWaypointPointer(pick_list->records[i].index0);

				center.x += waypoint->position.x;
				center.y += waypoint->position.y;
				center.z += waypoint->position.z;
			break;
		}

	}

	center.x /= (float)pick_list->record_count;
	center.y /= (float)pick_list->record_count;
	center.z /= (float)pick_list->record_count;


	return center;
}










