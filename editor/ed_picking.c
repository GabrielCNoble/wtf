#include "ed_picking.h"

#include "..\common\r_gl.h"
#include "..\common\r_shader.h"
#include "..\common\r_imediate.h"
#include "..\common\r_debug.h"
#include "..\common\r_main.h"
#include "..\common\r_view.h"
#include "..\common\containers\stack_list.h"
#include "..\common\l_main.h"
#include "ed_picklist.h"
#include "brush.h"

extern struct framebuffer_t ed_pick_framebuffer;
extern struct renderer_t r_renderer;

void editor_EnablePicking()
{
	R_DBG_PUSH_FUNCTION_NAME();
	renderer_PushFramebuffer(&ed_pick_framebuffer);
	renderer_PushViewport(0, 0, r_renderer.r_window_width, r_renderer.r_window_height);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	R_DBG_POP_FUNCTION_NAME();
}

//    /* some gpus don't seem to bother working
//    with subnormal numbers, so they get rounded
//    to zero. To overcome that, we'll set the
//    leftmost bit of the mantissa. This leaves
//    us with 30 bits of value, which is... good
//    enough... */

#define SUBNORMAL_MASK 0x40000000

void editor_SamplePickingBuffer(float mouse_x, float mouse_y, int *sample)
{
    R_DBG_PUSH_FUNCTION_NAME();
	renderer_SampleFramebuffer(mouse_x, mouse_y, sample);
	sample[0] = sample[0] & (~SUBNORMAL_MASK);
	sample[1] = sample[1] & (~SUBNORMAL_MASK);
	R_DBG_POP_FUNCTION_NAME();
}

void editor_DisablePicking()
{
    R_DBG_PUSH_FUNCTION_NAME();
	renderer_PopViewport();
	renderer_PopFramebuffer();
	R_DBG_POP_FUNCTION_NAME();
}


/* from brush.c */
extern struct brush_t *brushes;

extern struct stack_list_t l_light_positions;

extern int ed_pick_shader;
extern int ed_brush_dist_shader;


vec3_t editor_3DCursorPosition(float mouse_x, float mouse_y)
{
    int i;
	int c;
	int j;
	int k;
	int x;
	int y;

//	camera_t *active_camera = camera_GetActiveCamera();
    struct view_def_t *main_view = renderer_GetMainViewPointer();
	brush_t *brush;

	mat4_t camera_to_world_matrix;

	vec4_t pos;

	float q[4];
	float z;
	float qr;
	float qt;



	editor_EnablePicking();
	glClearColor(main_view->frustum.zfar, main_view->frustum.zfar, main_view->frustum.zfar, main_view->frustum.zfar);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

    renderer_EnableVertexReads();
	renderer_SetShader(ed_brush_dist_shader);
	renderer_SetProjectionMatrix(&main_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&main_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_EnableImediateDrawing();

	brush = brushes;

	while(brush)
	{
		//renderer_DrawVertsIndexed(GL_TRIANGLES, brush->clipped_polygons_index_count, 4, sizeof(vertex_t), brush->clipped_polygons_vertices, brush->clipped_polygons_indexes);
		editor_DrawPickableBrush(brush, 0);
		brush = brush->next;
	}

	editor_SamplePickingBuffer(mouse_x, mouse_y, (int *)q);
	editor_DisablePicking();

	renderer_DisableImediateDrawing();
	renderer_DisableVertexReads();

	mat4_t_compose(&camera_to_world_matrix, &main_view->world_orientation, main_view->world_position);
	qr = main_view->frustum.znear / main_view->frustum.right;
	qt = main_view->frustum.znear / main_view->frustum.top;

	if(q[3] == main_view->frustum.zfar)
	{
		z = -10.0;
	}
	else
	{
		z = -q[3];
	}

	pos.x = (mouse_x / qr) * (-z);
	pos.y = (mouse_y / qt) * (-z);
	pos.z = z;
	pos.w = 1.0;

	mat4_t_vec4_t_mult(&camera_to_world_matrix, &pos);

	return pos.vec3;
}

struct pick_record_t editor_PickObject(float mouse_x, float mouse_y)
{
    int id;
    struct brush_t *brush;
    struct pick_record_t record;

    int i;
    int c;

    record.type = PICK_NONE;
    record.pointer = NULL;

    struct view_def_t *main_view = renderer_GetMainViewPointer();
    unsigned int pick_sample[4] = {0};


	renderer_SetShader(ed_pick_shader);
	renderer_SetProjectionMatrix(&main_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&main_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);

    editor_EnablePicking();
	renderer_EnableImediateDrawing();
	renderer_EnableVertexReads();

    brush = brushes;
    id = 1;

    while(brush)
	{
       editor_DrawPickableBrush(brush, id);
       id++;

       brush = brush->next;
	}


	c = l_light_positions.element_count;

	for(id = 0; id < c; id++)
    {
        editor_DrawPickableLight(id);
    }


    renderer_DisableVertexReads();
	renderer_DisableImediateDrawing();
	editor_SamplePickingBuffer(mouse_x, mouse_y, pick_sample);
	editor_DisablePicking();

    record.type = pick_sample[0];

	switch(record.type)
	{
		case PICK_BRUSH:
			brush = brushes;
			id = 1;
			while(brush)
			{
				if(id == pick_sample[1])
				{
					break;
				}
				brush = brush->next;
				id++;
			}
			record.pointer = brush;
		break;

		case PICK_LIGHT:
            record.index0 = pick_sample[1] - 1;
        break;
	}

	return record;
}

void editor_DrawPickableBrush(struct brush_t *brush, int brush_id)
{
    int value;
    int j;
    bsp_polygon_t *polygon;

    value = PICK_BRUSH | SUBNORMAL_MASK;
    renderer_SetNamedUniform1f("pick_type", *(float *)&value);

    value = brush_id | SUBNORMAL_MASK;
    renderer_SetNamedUniform1f("pick_index", *(float *)&value);

    polygon = brush->base_polygons;

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

void editor_DrawPickableLight(int light_index)
{
    int value;
    struct light_position_data_t *position;

	value = PICK_LIGHT | SUBNORMAL_MASK;
	renderer_SetNamedUniform1f("pick_type", *(float *)&value);


    position = (struct light_position_data_t *)l_light_positions.elements + light_index;

    if(position->flags & LIGHT_INVALID)
    {
        return;
    }

    value = (light_index + 1) | SUBNORMAL_MASK;
    renderer_SetNamedUniform1f("pick_index", *(float *)&value);

    glPointSize(24.0);
	glEnable(GL_POINT_SMOOTH);

    renderer_Begin(GL_POINTS);
    renderer_Vertex3f(position->position.x, position->position.y, position->position.z);
    renderer_End();

	glDisable(GL_POINT_SMOOTH);
	glPointSize(1.0);
}



