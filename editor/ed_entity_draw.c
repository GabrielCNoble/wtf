#include "ed_entity_draw.h"
#include "ed_entity.h"

#include "..\ed_common.h"
#include "ed_draw.h"

#include "..\..\common\r_imediate.h"
#include "..\..\common\r_main.h"
#include "..\..\common\r_debug.h"
#include "..\..\common\r_shader.h"
#include "..\..\common\r_view.h"
#include "..\..\common\camera.h"
#include "..\..\common\gmath\vector.h"
#include "..\..\common\gmath\matrix.h"
#include "..\..\common\model.h"
#include "..\..\common\entity.h"
#include "..\..\common\physics.h"

#include "..\..\common\GLEW\include\GL\glew.h"


extern struct entity_transform_t *ent_global_transform;


/* from ed_entity.c */
extern int entity_editor_draw_grid;
extern int entity_editor_draw_entity_def;
extern int entity_editor_draw_collider_def;
extern int entity_editor_draw_3d_handle;
extern struct entity_t *ed_entity_editor_current_entity_def;
extern vec3_t entity_editor_3d_handle_position;
extern vec3_t entity_editor_3d_cursor_position;
extern int entity_editor_3d_handle_transform_mode;

extern int ed_entity_editor_draw_collider_list_cursor;
extern struct collision_shape_t *ed_entity_editor_hovered_collision_shape;
extern struct entity_handle_t ed_entity_editor_draw_collider_list[1024];

/* from editor.c */
extern unsigned int ed_cursors_framebuffer_id;
extern unsigned int ed_cursors_color_texture_id;
extern int ed_brush_pick_shader;
extern int ed_pick_shader;


/* from r_main.c */
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;
extern int r_blit_texture_shader;



float cube_collision_shape_verts[] =
{
	-1.0, 1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0,-1.0, 1.0,
	-1.0, 1.0, 1.0,

	 1.0,-1.0,-1.0,
	 1.0, 1.0,-1.0,
	 1.0, 1.0, 1.0,
	 1.0,-1.0, 1.0,

	-1.0, 1.0,-1.0,
	-1.0, 1.0, 1.0,
	 1.0, 1.0, 1.0,
	 1.0, 1.0,-1.0,

	-1.0,-1.0, 1.0,
	-1.0,-1.0,-1.0,
	 1.0,-1.0,-1.0,
	 1.0,-1.0, 1.0,

	 1.0, 1.0,-1.0,
	 1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,
	-1.0, 1.0,-1.0,

	 1.0,-1.0, 1.0,
	 1.0, 1.0, 1.0,
	-1.0, 1.0, 1.0,
	-1.0,-1.0, 1.0,
};



/* from r_imediate.c */
extern int r_imediate_color_shader;



void editor_EntityEditorPreDraw()
{
	if(entity_editor_draw_entity_def)
	{
		editor_EntityEditorDrawEntityDef();
	}
}

void editor_EntityEditorPostDraw()
{
	if(entity_editor_draw_grid)
	{
		editor_EntityEditorDrawGrid();
	}

	if(entity_editor_draw_collider_def)
	{
		editor_EntityEditorDrawColliderDef(0);
	}

//	editor_EntityEditorDrawCursors();
}

/*
====================================================================
====================================================================
====================================================================
*/
mat4_t entity_editor_entity_def_transform;


void editor_EntityEditorRecursiveDrawEntity(struct entity_t *entity)
{

}

void editor_EntityEditorDrawEntityDef()
{
	int i;
	struct model_t *model;
	struct model_component_t *model_component;
	struct transform_component_t *transform_component;
	struct entity_transform_t *global_transform;

	entity_editor_entity_def_transform = mat4_t_id();
//	camera_t *main_view = camera_GetMainViewCamera();
/*
	if(ed_entity_editor_current_entity_def)
	{

		model_component = entity_GetComponentPointer(ed_entity_editor_current_entity_def->components[COMPONENT_TYPE_MODEL]);

		if(model_component)
		{
			model = model_GetModelPointerIndex(model_component->model_index);

			if(model)
			{
				for(i = 0; i < model->batch_count; i++)
				{
					renderer_SubmitDrawCommand(&entity_editor_entity_def_transform, model->draw_mode, model->vert_start + model->batches[i].start, model->batches[i].next, model->batches[i].material_index, 0);
				}
			}
		}


	}*/
}

void editor_EntityEditorDrawColliderDef(int pick)
{
//	camera_t *active_camera;
    struct view_def_t *main_view;
	int i;
	int j;

	struct collider_def_t *collider_def;
	struct collision_shape_t *collision_shape;
	struct entity_t *entity;
	struct transform_component_t *transform_component;
	struct physics_component_t *physics_component;

	mat4_t entity_transform;
	mat4_t shape_local_transform;
	mat4_t shape_world_transform;

	float color[4];

	vec4_t shape_color;


	//active_camera = camera_GetActiveCamera();
	main_view = renderer_GetMainViewPointer();
	renderer_EnableImediateDrawing();
	renderer_SetShader(r_imediate_color_shader);

	glEnable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

	renderer_SetProjectionMatrix(&main_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&main_view->view_data.view_matrix);





	for(i = 0; i < ed_entity_editor_draw_collider_list_cursor; i++)
	{
		entity = entity_GetEntityPointerHandle(ed_entity_editor_draw_collider_list[i]);

		if(entity)
		{
			transform_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);
			physics_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_PHYSICS]);

			collider_def = physics_GetColliderDefPointerHandle(physics_component->collider);

			if(collider_def)
			{
				mat4_t_compose2(&entity_transform, &transform_component->orientation, transform_component->position, transform_component->scale);

				for(j = 0; j < collider_def->collision_shape.element_count; j++)
				{
					//collision_shape = collider_def->collision_shape + j;

					collision_shape = physics_GetCollisionShapePointer(physics_component->collider, j);

					if(collision_shape == ed_entity_editor_hovered_collision_shape)
					{
						shape_color.r = 0.5;
						shape_color.g = 1.0;
						shape_color.b = 0.0;
						shape_color.a = 0.4;

						renderer_Begin(GL_LINES);

						renderer_Color3f(1.0, 0.0, 0.0);
                        renderer_Vertex3f(-1000.0 + collision_shape->position.x, collision_shape->position.y, collision_shape->position.z);
                        renderer_Vertex3f(1000.0 + collision_shape->position.x, collision_shape->position.y, collision_shape->position.z);

                        renderer_Color3f(0.0, 1.0, 0.0);
                        renderer_Vertex3f(collision_shape->position.x, -1000.0 - collision_shape->position.y, collision_shape->position.z);
                        renderer_Vertex3f(collision_shape->position.x, 1000.0 + collision_shape->position.y, collision_shape->position.z);

                        renderer_Color3f(0.0, 0.0, 1.0);
                        renderer_Vertex3f(collision_shape->position.x, collision_shape->position.y, -1000.0 - collision_shape->position.z);
                        renderer_Vertex3f(collision_shape->position.x, collision_shape->position.y, 1000.0 + collision_shape->position.z);

						renderer_End();

					}
					else
					{
						shape_color.r = 0.0;
						shape_color.g = 1.0;
						shape_color.b = 0.0;
						shape_color.a = 0.2;
					}


					mat4_t_compose2(&shape_local_transform, &collision_shape->orientation, collision_shape->position, collision_shape->scale);
					mat4_t_mult_fast(&shape_world_transform, &shape_local_transform, &entity_transform);
					renderer_SetModelMatrix(&shape_world_transform);



					switch(collision_shape->type)
					{
						case COLLISION_SHAPE_BOX:
							renderer_Color4f(shape_color.r, shape_color.g, shape_color.b, shape_color.a);
							glCullFace(GL_FRONT);
							renderer_DrawBox();
							glCullFace(GL_BACK);
							renderer_DrawBox();


							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							glDisable(GL_CULL_FACE);
							glDisable(GL_DEPTH_TEST);
							renderer_Color4f(1.0, 1.0, 1.0, 1.0);
							renderer_DrawBox();
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
							glEnable(GL_DEPTH_TEST);
							glEnable(GL_CULL_FACE);
						break;

						case COLLISION_SHAPE_CYLINDER:
							renderer_Color4f(shape_color.r, shape_color.g, shape_color.b, shape_color.a);
							glCullFace(GL_FRONT);
							renderer_DrawCylinder(16, 1.0, 1.0, 0);
							glCullFace(GL_BACK);
							renderer_DrawCylinder(16, 1.0, 1.0, 0);



							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							glDisable(GL_CULL_FACE);
							glDisable(GL_DEPTH_TEST);
							renderer_Color4f(1.0, 1.0, 1.0, 1.0);
							renderer_DrawCylinder(16, 1.0, 1.0, 1);
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
							glEnable(GL_DEPTH_TEST);
							glEnable(GL_CULL_FACE);
						break;
					}


				}


			}

		}
	}

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	renderer_DisableImediateDrawing();


//	if(ed_entity_editor_current_entity_def)
//	{
		/*if(entity_editor_current_entity_def->collider_def)
		{

			collider_def = entity_editor_current_entity_def->collider_def;

			active_camera = camera_GetActiveCamera();

			if(!pick)
			{
				renderer_SetShader(r_imediate_color_shader);
			}
			else
			{
				renderer_SetShader(ed_pick_shader);
			}

			renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
			renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
			renderer_EnableImediateDrawing();

			if(!pick)
			{
				glEnable(GL_BLEND);
				glDepthMask(GL_FALSE);
			}

			for(i = 0; i < collider_def->collider_data.generic_collider_data.collision_shape_count; i++)
			{
				collision_shape = &collider_def->collider_data.generic_collider_data.collision_shape[i];

				mat4_t_compose(&shape_transform, &collision_shape->orientation, collision_shape->position);

				shape_transform.floats[0][0] *= collision_shape->scale.x;
				shape_transform.floats[1][0] *= collision_shape->scale.x;
				shape_transform.floats[2][0] *= collision_shape->scale.x;

				shape_transform.floats[0][1] *= collision_shape->scale.y;
				shape_transform.floats[1][1] *= collision_shape->scale.y;
				shape_transform.floats[2][1] *= collision_shape->scale.y;

				shape_transform.floats[0][2] *= collision_shape->scale.z;
				shape_transform.floats[1][2] *= collision_shape->scale.z;
				shape_transform.floats[2][2] *= collision_shape->scale.z;

				renderer_SetModelMatrix(&shape_transform);

				switch(collision_shape->type)
				{
					case COLLISION_SHAPE_BOX:

						if(pick)
						{
							*(int *)&color[0] = PICK_COLLIDER_PRIMITIVE;
							*(int *)&color[1] = i + 1;
							glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
							renderer_DrawVerts(GL_QUADS, 24, 3, 0, cube_collision_shape_verts);
						}
						else
						{
							renderer_Color4f(0.0, 0.5, 0.0, 0.5);
							renderer_DrawVerts(GL_QUADS, 24, 3, 0, cube_collision_shape_verts);
							renderer_Color4f(1.0, 1.0, 1.0, 1.0);
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							glLineWidth(2.0);
							renderer_DrawVerts(GL_QUADS, 24, 3, 0, cube_collision_shape_verts);
							glLineWidth(1.0);
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
						}


					break;

					case COLLISION_SHAPE_CYLINDER:

					break;
				}
			}

			renderer_DisableImediateDrawing();


			if(!pick)
			{
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);
			}

		}*/
//	}
}

void editor_EntityEditorDrawGrid()
{
	int i;
	int j;
//	camera_t *active_camera = camera_GetActiveCamera();
    struct view_def_t *main_view = renderer_GetMainViewPointer();

	//glUseProgram(0);
	//renderer_SetShader(-1);
	renderer_SetShader(r_imediate_color_shader);
	glLineWidth(2.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	renderer_SetViewMatrix(&main_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);

	renderer_EnableImediateDrawing();
	renderer_Begin(GL_QUADS);
	renderer_Color3f(1.0, 1.0, 1.0);

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

#if 0

void editor_EntityEditorDrawCursors()
{
/*	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ed_cursors_framebuffer_id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, r_width, r_height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);*/

    renderer_Bindframebuffer(&ed_cursors_framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	if(entity_editor_draw_3d_handle)
	{
		editor_Draw3dHandle(entity_editor_3d_handle_position, entity_editor_3d_handle_transform_mode);
	}

	editor_Draw3dCursor(entity_editor_3d_cursor_position);

	//renderer_BindBackbuffer(0, 0);

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
}

#endif





