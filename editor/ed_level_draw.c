#include "ed_level_draw.h"
#include "ed_level_manip.h"
#include "ed_level.h"

#include "..\..\common\GLEW\include\GL\glew.h"
#include "..\..\common\camera.h"
#include "..\..\common\shader.h"
#include "..\..\common\l_main.h"
#include "..\..\common\r_imediate.h"
#include "..\..\common\r_shader.h"
#include "..\..\common\r_gl.h"
#include "..\..\common\r_main.h"
#include "..\..\common\r_view.h"
#include "..\..\common\r_debug.h"
#include "..\..\common\portal.h"
#include "..\..\common\navigation.h"
#include "..\..\common\containers\stack_list.h"
#include "..\..\common\material.h"
#include "ed_level_common.h"

#include "ed_draw.h"
#include "bsp_cmp.h"
#include "..\brush.h"

#include <math.h>

/* from editor.c */
/*extern unsigned int ed_cursors_framebuffer_id;
extern unsigned int ed_cursors_color_texture_id;
extern int ed_3d_handle_transform_mode;*/

extern struct framebuffer_t ed_cursors_framebuffer;


/* from ed_level.c */
extern int level_editor_draw_brushes;
extern int level_editor_draw_wireframe_brushes;
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
//extern pick_list_t level_editor_pick_list;
//extern pick_list_t level_editor_brush_face_pick_list;

//extern vec3_t level_editor_3d_handle_position;
//extern vec3_t level_editor_3d_cursor_position;
//extern float level_editor_linear_snap_value;
//extern int level_editor_3d_handle_transform_mode;
extern int level_editor_editing_mode;
//extern int level_editor_state;

//extern struct selection_state_t level_editor_selection_state;
extern struct editor_state_t level_editor_state;

/* from brush.c */
extern brush_t *brushes;


/* from r_main.c */
extern int r_frame;
extern struct renderer_t r_renderer;
//extern int r_width;
//extern int r_height;
extern int r_flat;
extern int r_z_pre_pass_shader;
extern int r_flat_pass_shader;
extern int r_forward_pass_shader;
extern int r_blit_texture_shader;
//extern struct framebuffer_t r_bbuffer;



/* from world.c */
extern struct bsp_dleaf_t **w_visible_leaves;
extern struct bsp_dleaf_t *w_world_leaves;
extern int w_visible_leaves_count;
extern vertex_t *w_world_vertices;


/* from l_main.c */
//extern int l_light_list_cursor;
//extern light_params_t *l_light_params;
//extern light_position_t *l_light_positions;
extern struct stack_list_t l_light_positions;
extern struct stack_list_t l_light_params;
extern struct stack_list_t l_light_clusters;


/* from portal.c */
extern int ptl_portal_list_cursor;
extern portal_t *ptl_portals;


/* from navigation.c */
extern struct stack_list_t nav_waypoints;
//extern int nav_waypoint_count;
//extern struct waypoint_t *nav_waypoints;


/* from r_imediate.c */
extern int r_imediate_color_shader;

void editor_LevelEditorPreDraw()
{
	if(level_editor_state.editor_state == EDITOR_PIE)
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
	if(level_editor_state.editor_state == EDITOR_PIE)
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

	/*if(level_editor_draw_world_polygons)
	{
		editor_LevelEditorDrawWorldPolygons();
	}*/

/*	if(level_editor_draw_clipped_polygons)
	{
		editor_LevelEditorDrawClippedPolygons();
	}*/

	if(level_editor_draw_cursors)
	{
		editor_LevelEditorDrawCursors();
	}

	if(level_editor_state.transforming_selection)
    {
        editor_LevelEditorDrawManipulationGuides();
    }
}

//mat4_t brush_draw_transform;

extern struct stack_list_t brush_transforms;

void editor_LevelEditorDrawBrushes()
{

	brush_t *brush;
	portal_t *portals;
	portal_t *portal;
	portal_recursive_view_data_t *recursive_view_data;
	portal_view_data_t *view_data;
//	camera_t *active_camera = camera_GetActiveCamera();
//	camera_t *main_view = camera_GetMainViewCamera();

    struct view_def_t *main_view = renderer_GetMainViewPointer();
	struct batch_t *batch;
	material_t *material;
	bsp_polygon_t *polygon;
	int transform_index;
	//brush_draw_transform = mat4_t_id();
	mat4_t *brush_draw_transform;
	int i;
	//int c = brush_count;
	int light_index;
	int j;
	int k;
	int l;

	float color[] = {10.0, 0.0, 0.0, 0.0};

	//gpu_BindGpuHeap();

	if(!level_editor_draw_brushes)
		return;

	brush = brushes;

	brush_transforms.element_count = 0;
	brush_transforms.free_stack_top = -1;

	while(brush)
	{
		if(!(brush->bm_flags & BRUSH_SUBTRACTIVE))
		{
			//SDL_LockMutex(brush->brush_mutex);

			//if(!SDL_TryLockMutex(brush->brush_mutex))
			{
				for(i = 0; i < brush->batch_count; i++)
				{

					transform_index = stack_list_add(&brush_transforms, NULL);

					brush_draw_transform = (mat4_t *)stack_list_get(&brush_transforms, transform_index);

					*brush_draw_transform = mat4_t_id();

					brush_draw_transform->floats[3][0] = brush->position.x;
					brush_draw_transform->floats[3][1] = brush->position.y;
					brush_draw_transform->floats[3][2] = brush->position.z;

					//renderer_SubmitDrawCommandToView(main_view, &brush_draw_transform, GL_TRIANGLES, brush->batches[i].start + brush->index_start, brush->batches[i].next, brush->batches[i].material_index, 1);
					renderer_SubmitDrawCommand(brush_draw_transform, GL_TRIANGLES, brush->batches[i].start + brush->index_start, brush->batches[i].next, brush->batches[i].material_index, 1);
				}

				//SDL_UnlockMutex(brush->brush_mutex);
			}


		}

		brush = brush->next;
	}

	renderer_EnableImediateDrawing();
	renderer_SetShader(r_renderer.r_shaders.r_imediate_color_shader);
	//renderer_SetMainView(active_camera);
	renderer_SetModelMatrix(NULL);
	renderer_SetViewMatrix(&main_view->view_data.view_matrix);
	renderer_SetProjectionMatrix(&main_view->view_data.projection_matrix);
	renderer_UpdateMatrices();
	//glPolygonOffset(58.0, 20.0);

	brush = brushes;
	/* subtractive... */
	while(brush)
	{

		if(!level_editor_draw_wireframe_brushes)
		{
			if(!(brush->bm_flags & BRUSH_SUBTRACTIVE))
			{
				brush = brush->next;
				continue;
			}
		}

		//SDL_LockMutex(brush->brush_mutex);

		if(brush->bm_flags & BRUSH_SUBTRACTIVE)
		{
			renderer_Color3f(10.0, 0.0, 0.0);
	//		glDisable(GL_POLYGON_OFFSET_LINE);
			glLineWidth(1.0);
		}
		else
		{
			renderer_Color3f(0.0, 10.0, 0.0);
	//		glEnable(GL_POLYGON_OFFSET_LINE);
			glLineWidth(1.0);

		}

		//if(brush->bm_flags & BRUSH_SUBTRACTIVE)
	//	{
		//renderer_Color3f(10.0, 0.0, 0.0);

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

		//SDL_UnlockMutex(brush->brush_mutex);
		//}

		brush = brush->next;
	}

	renderer_DisableImediateDrawing();
	//glDisable(GL_POLYGON_OFFSET_LINE);
}

void editor_LevelEditorDrawGrid()
{
	int i;
	int j;

	float snap_value;
	//camera_t *active_camera = camera_GetActiveCamera();
	struct view_def_t *main_view = renderer_GetMainViewPointer();
	mat4_t grid_transform = mat4_t_id();

	renderer_SetShader(r_renderer.r_shaders.r_imediate_color_shader);
	glLineWidth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	renderer_SetProjectionMatrix(&main_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&main_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);

    glLineWidth(2.0);

	renderer_EnableImediateDrawing();
	renderer_Begin(GL_QUADS);
	renderer_Color3f(0.3, 0.3, 0.3);


//    if(level_editor_selection_state.linear_snap_value != 0.0)
//    {
//        snap_value = level_editor_selection_state.linear_snap_value;
//    }
//    else
//    {
//        snap_value = 1.0;
//    }

    snap_value = 1.0;

	grid_transform.floats[3][0] = snap_value * (int)(main_view->world_position.floats[0] / snap_value);
	grid_transform.floats[3][2] = snap_value * (int)(main_view->world_position.floats[2] / snap_value);


	grid_transform.floats[0][0] = snap_value;
	grid_transform.floats[2][2] = snap_value;

	renderer_SetModelMatrix(&grid_transform);

	#define GRID_DIVS 50 / snap_value

	for(i = 0; i <= GRID_DIVS; i++)
	{
		renderer_Vertex3f(-i, 0.0,-GRID_DIVS);
		renderer_Vertex3f(-i, 0.0, GRID_DIVS);
		renderer_Vertex3f( i, 0.0, GRID_DIVS);
		renderer_Vertex3f( i, 0.0,-GRID_DIVS);
	}

	for(i = 0; i <= GRID_DIVS; i++)
	{
		renderer_Vertex3f(-GRID_DIVS, 0.0,-i);
		renderer_Vertex3f( GRID_DIVS, 0.0,-i);
		renderer_Vertex3f( GRID_DIVS, 0.0, i);
		renderer_Vertex3f(-GRID_DIVS, 0.0, i);
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
	struct pick_record_t *record;
//	camera_t *active_view;
	portal_t *portal;
	struct waypoint_t *waypoint;
	struct waypoint_t *waypoints;

	struct light_position_data_t *position;

	vec3_t portal_right_vector;
	vec3_t portal_up_vector;
	vec3_t portal_center;

	//active_view = camera_GetActiveCamera();
	struct view_def_t *main_view = renderer_GetMainViewPointer();


	//renderer_SetProjectionMatrix(&active_view)
	//renderer_SetMainView(active_view);
	renderer_SetModelMatrix(NULL);


	renderer_EnableImediateDrawing();

	renderer_PushStates(GL_STENCIL_TEST);
	glEnable(GL_STENCIL_TEST);
	//glCullFace(GL_FRONT);
	glDisable(GL_CULL_FACE);

	waypoints = (struct waypoint_t *)nav_waypoints.elements;

	if(level_editor_editing_mode == EDITING_MODE_OBJECT)
	{
		for(i = 0; i < level_editor_state.selection_state.pick_list.record_count; i++)
		{
			if(i == level_editor_state.selection_state.pick_list.record_count - 1)
			{
				renderer_Color3f(0.2, 0.2, 1.0);
			}
			else
			{
				renderer_Color3f(0.4, 0.2, 0.6);
			}

			record = level_editor_state.selection_state.pick_list.records + i;

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
					glLineWidth(1.0);
				}

				switch(record->type)
				{
					case PICK_BRUSH:

						brush = record->pointer;

						//SDL_LockMutex(brush->brush_mutex);

						//printf("%d\n", brush->bm_flags & BRUSH_MOVED);

						//polygon = brush->clipped_polygons;

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

						//SDL_UnlockMutex(brush->brush_mutex);
					break;

					case PICK_WAYPOINT:

						waypoint = waypoints + record->index0;

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

					#if 0

					case PICK_LIGHT:
                        position = (struct light_position_data_t *)l_light_positions.elements + record->index0;
                        //glPointSize(24.0);
                        renderer_Begin(GL_POINTS);
                        renderer_Vertex3f(position->position.x, position->position.y, position->position.z);
                        renderer_End();
                        //glPointSize(1.0);

                    break;

                    #endif

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

//		if(level_editor_pick_list.record_count)
//		{
//			record = &level_editor_pick_list.records[level_editor_pick_list.record_count - 1];
//
//			if(record->type == PICK_BRUSH)
//			{
//				renderer_Color3f(1.0, 0.5, 0.0);
//
//				for(r = 0; r < 2; r++)
//				{
//					if(r == 0)
//					{
//						glStencilFunc(GL_ALWAYS, 0xff, 0xff);
//						glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
//						glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
//						glDepthMask(GL_FALSE);
//						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//					}
//					else
//					{
//						glStencilFunc(GL_EQUAL, 0, 0xff);
//						glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
//						glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//						glDepthMask(GL_TRUE);
//						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//						glLineWidth(1.0);
//					}
//
//					brush = record->pointer;
//
//					if(!SDL_TryLockMutex(brush->brush_mutex))
//					{
//						polygon = brush->clipped_polygons;
//
//						while(polygon)
//						{
//							renderer_Begin(GL_TRIANGLE_FAN);
//							for(j = 0; j < polygon->vert_count; j++)
//							{
//								renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
//							}
//							renderer_End();
//
//							polygon = polygon->next;
//						}
//
//						SDL_UnlockMutex(brush->brush_mutex);
//					}
//
//				}
//
//
//				if(level_editor_brush_face_pick_list.record_count)
//				{
//					for(i = 0; i < level_editor_brush_face_pick_list.record_count; i++)
//					{
//						record = &level_editor_brush_face_pick_list.records[i];
//
//						switch(record->type)
//						{
//							case PICK_BRUSH_FACE:
//
//								brush = record->pointer;
//								polygon = &brush->base_polygons[record->index0];
//
//								renderer_Color4f(1.0, 0.0, 0.0, 0.25);
//								glEnable(GL_BLEND);
//								glDisable(GL_STENCIL_TEST);
//								glDisable(GL_DEPTH_TEST);
//								glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//
//								renderer_Begin(GL_TRIANGLE_FAN);
//								for(j = 0; j < polygon->vert_count; j++)
//								{
//									renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
//								}
//								renderer_End();
//
//								glDisable(GL_BLEND);
//								glEnable(GL_DEPTH_TEST);
//								glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//
//								renderer_Begin(GL_LINE_LOOP);
//								for(j = 0; j < polygon->vert_count; j++)
//								{
//									renderer_Vertex3f(polygon->vertices[j].position.x, polygon->vertices[j].position.y, polygon->vertices[j].position.z);
//								}
//								renderer_End();
//
//								glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//							break;
//						}
//					}
//
//				}
//			}
//
//
//
//		}



	}



	//glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);

	renderer_DisableImediateDrawing();

	renderer_PopStates(GL_STENCIL_TEST);

}

void editor_LevelEditorDrawCursors()
{
    renderer_BindFramebuffer(GL_DRAW_FRAMEBUFFER, &ed_cursors_framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(level_editor_draw_3d_handle)
	{
//		editor_Draw3dHandle(level_editor_3d_handle_position, level_editor_3d_handle_transform_mode);
	}

	editor_Draw3dCursor(level_editor_state.selection_state.cursor_position);

	renderer_BindFramebuffer(GL_DRAW_FRAMEBUFFER, &r_renderer.r_back_buffer);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_EQUAL, 0.0);

    renderer_EnableImediateDrawing();
    renderer_SetShader(r_renderer.r_shaders.r_blit_texture_shader);
    renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, ed_cursors_framebuffer.color_attachments[0].handle);
    renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
    renderer_Rectf(-1.0, -1.0, 1.0, 1.0);
    renderer_DisableImediateDrawing();

    glDisable(GL_ALPHA_TEST);
    glAlphaFunc(GL_ALWAYS, 1.0);


}


void editor_LevelEditorDrawLeaves()
{
	int i;
	int c;

	int j;

	struct bsp_dleaf_t *leaf;
	vertex_t *triangle;

	if(w_world_leaves)
	{

		if(w_visible_leaves_count)
		{
			glLineWidth(2.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_DEPTH_TEST);

			renderer_SetShader(r_renderer.r_shaders.r_imediate_color_shader);
			renderer_EnableImediateDrawing();

			renderer_Begin(GL_TRIANGLES);
			renderer_Color3f(0.2, 0.2, 0.2);

			for(i = w_visible_leaves_count - 1; i >= 0; i--)
			{
				leaf = w_visible_leaves[i];

				if(!i)
				{
					renderer_Color3f(0.0, 1.0, 0.0);
				}
				else
				{
					renderer_Color3f(0.2, 0.2, 0.2);
				}

				for(j = 0; j < leaf->tris_count; j++)
				{
					triangle = &w_world_vertices[leaf->tris[j].first_vertex];

					renderer_Vertex3f(triangle->position.x, triangle->position.y, triangle->position.z);
					triangle++;
					renderer_Vertex3f(triangle->position.x, triangle->position.y, triangle->position.z);
					triangle++;
					renderer_Vertex3f(triangle->position.x, triangle->position.y, triangle->position.z);
				}

			}

			renderer_End();


			leaf = w_visible_leaves[0];

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_BLEND);

			renderer_Begin(GL_TRIANGLES);
			renderer_Color4f(0.0, 1.0, 0.0, 0.2);

			for(j = 0; j < leaf->tris_count; j++)
			{
				triangle = &w_world_vertices[leaf->tris[j].first_vertex];

				renderer_Vertex3f(triangle->position.x, triangle->position.y, triangle->position.z);
				triangle++;
				renderer_Vertex3f(triangle->position.x, triangle->position.y, triangle->position.z);
				triangle++;
				renderer_Vertex3f(triangle->position.x, triangle->position.y, triangle->position.z);
			}

			renderer_End();


			renderer_DisableImediateDrawing();

			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			glLineWidth(1.0);

		}


	}


}

void editor_LevelEditorRecursiveDrawWorldPolygons(bsp_node_t *node)
{
	bsp_leaf_t *leaf;

	int i;

	bsp_polygon_t *polygon;

    if(node->type == BSP_LEAF)
	{
        leaf = (bsp_leaf_t *)node;

        if(!(leaf->bm_flags & BSP_SOLID))
		{
			polygon = leaf->polygons;

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
	}
	else
	{
		editor_LevelEditorRecursiveDrawWorldPolygons(node->front);
		editor_LevelEditorRecursiveDrawWorldPolygons(node->back);
	}

}

extern bsp_node_t *world_bsp;

void editor_LevelEditorDrawWorldPolygons()
{

	if(!world_bsp)
	{
		return;
	}

	glLineWidth(1.0);

	renderer_SetShader(r_renderer.r_shaders.r_imediate_color_shader);
	renderer_EnableImediateDrawing();

	editor_LevelEditorRecursiveDrawWorldPolygons(world_bsp);

	renderer_DisableImediateDrawing();
}

void editor_LevelEditorDrawClippedPolygons()
{
	brush_t *brush;
    int i;
    bsp_polygon_t *polygon;


	renderer_SetShader(r_renderer.r_shaders.r_imediate_color_shader);
    renderer_EnableImediateDrawing();

	brush = brushes;

	renderer_Color3f(1.0, 1.0, 1.0);

	while(brush)
	{
		if(!(brush->bm_flags & BRUSH_SUBTRACTIVE))
		{

			//SDL_LockMutex(brush->brush_mutex);

			if(!SDL_TryLockMutex(brush->brush_mutex))
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

				SDL_UnlockMutex(brush->brush_mutex);
			}
		}

		brush = brush->next;
	}



	renderer_DisableImediateDrawing();
}

void editor_LevelEditorDrawEntitiesAabbs()
{

}

void editor_LevelEditorDrawManipulationGuides()
{
    int i;
    int c;

    struct pick_record_t *records;

    vec3_t guide_a;
    vec3_t guide_b;
    vec3_t guide_color;

    if(level_editor_state.selection_state.axis_constraint)
    {

        records = level_editor_state.selection_state.pick_list.records;
        c = level_editor_state.selection_state.pick_list.record_count;

        switch(level_editor_state.selection_state.axis_constraint)
        {
            case SELECTION_TRANSFORM_CONSTRAINT_X_AXIS:
                guide_a = vec3_t_c(-99999.0, 0.0, 0.0);
                guide_b = vec3_t_c(99999.0, 0.0, 0.0);
                guide_color = vec3_t_c(1.0, 0.0, 0.0);
            break;

            case SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS:
                guide_a = vec3_t_c(0.0, -99999.0, 0.0);
                guide_b = vec3_t_c(0.0, 99999.0, 0.0);
                guide_color = vec3_t_c(0.0, 1.0, 0.0);
            break;

            case SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS:
                guide_a = vec3_t_c(0.0, 0.0, -99999.0);
                guide_b = vec3_t_c(0.0, 0.0, 99999.0);
                guide_color = vec3_t_c(0.0, 0.0, 1.0);
            break;
        }

        for(i = 0; i < c; i++)
        {
            renderer_DrawLine(add3(editor_LevelEditorGetSelectionPosition(records + i), guide_a),
                              add3(editor_LevelEditorGetSelectionPosition(records + i), guide_b), guide_color, 1.0, 0);
        }


    }
}















