#include "ed_draw.h"
#include "ed_common.h"
#include "camera.h"
#include "r_imediate.h"
#include "r_shader.h"
#include "r_main.h"

#include "GL/glew.h"

#include "ed_globals.h"

extern int r_imediate_color_shader;


void editor_Draw3dHandle(vec3_t position, int transform_mode)
{
//	camera_t *active_camera = camera_GetActiveCamera();
    camera_t *active_camera = (camera_t *)renderer_GetActiveView();
	vec4_t cursor_position;
	int i;
	float d;

	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t forward_vector;
	vec3_t v;

	float verts[] = {-1.0, 0.0,-1.0,
					-1.0, 0.0, 1.0,
					 1.0, 0.0, 1.0,
					 1.0, 0.0,-1.0,};

	float color[] = {1.0, 0.0, 0.0,
					 1.0, 0.0, 0.0,
					 1.0, 0.0, 0.0,
					 1.0, 0.0, 0.0,};

	//renderer_SetShader(-1);

	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&active_camera->projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);*/


	glEnable(GL_POINT_SMOOTH);

	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ed_cursors_framebuffer_id);
	//glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glViewport(0, 0, r_width, r_height);

	//glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClearStencil(0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	/*glDisable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexPointer(3, GL_FLOAT, 0, verts);
	glColorPointer(3, GL_FLOAT, 0, color);

	glDrawArrays(GL_QUADS, 0, 4);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_CULL_FACE);*/

	//glEnable(GL_STENCIL_TEST);
	//glStencilFunc(GL_ALWAYS, 0xff, 0xff);
	//glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);




	//if(ed_selection_count && ed_editing_mode != EDITING_MODE_UV)
	{
		//cursor_position.vec3 = ed_3d_handle_position;
		cursor_position.vec3 = position;
		cursor_position.w = 1.0;

		mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &cursor_position);

		//if(cursor_position.z < nznear)
		{

			//glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
			d = sqrt(cursor_position.x * cursor_position.x + cursor_position.y * cursor_position.y + cursor_position.z * cursor_position.z);

			right_vector = vec3_t_c(1.0, 0.0, 0.0);
			up_vector = vec3_t_c(0.0, 1.0, 0.0);
			forward_vector = vec3_t_c(0.0, 0.0, 1.0);

			d *= 0.2;
			//d = 1.0;

			right_vector.x *= d;
			right_vector.y *= d;
			right_vector.z *= d;

			up_vector.x *= d;
			up_vector.y *= d;
			up_vector.z *= d;

			forward_vector.x *= d;
			forward_vector.y *= d;
			forward_vector.z *= d;

			renderer_SetShader(r_imediate_color_shader);
			renderer_SetProjectionMatrix(&active_camera->view_data.projection_matrix);
			renderer_SetViewMatrix(&active_camera->view_data.view_matrix);
			renderer_SetModelMatrix(NULL);
			renderer_EnableImediateDrawing();

			//switch(ed_3d_handle_transform_mode)
			switch(transform_mode)
			{
				case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
				case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:

					glDisable(GL_BLEND);
					glPointSize(16.0);
					renderer_Begin(GL_POINTS);
					renderer_Color4f(1.0, 1.0, 1.0, 0.0);
					renderer_Vertex3f(position.x, position.y, position.z);
					renderer_End();
					//glBegin(GL_POINTS);
					//glColor4f(1.0, 1.0, 1.0, 1.0);
					//glVertex3f(position.x, position.y, position.z);
					//glEnd();


					glLineWidth(6.0);

					renderer_Begin(GL_LINES);
					renderer_Color4f(1.0, 0.0, 0.0, 0.0);
					renderer_Vertex3f(position.x, position.y, position.z);
					renderer_Vertex3f(position.x + right_vector.x, position.y + right_vector.y, position.z + right_vector.z);

					renderer_Color4f(0.0, 1.0, 0.0, 0.0);
					renderer_Vertex3f(position.x, position.y, position.z);
					renderer_Vertex3f(position.x + up_vector.x, position.y + up_vector.y, position.z + up_vector.z);

					renderer_Color4f(0.0, 0.0, 1.0, 0.0);
					renderer_Vertex3f(position.x, position.y, position.z);
					renderer_Vertex3f(position.x + forward_vector.x, position.y + forward_vector.y, position.z + forward_vector.z);
					renderer_End();

					/*glBegin(GL_LINES);
					glColor4f(1.0, 0.0, 0.0, 1.0);
					glVertex3f(position.x, position.y, position.z);
					glVertex3f(position.x + right_vector.x, position.y + right_vector.y, position.z + right_vector.z);

					glColor4f(0.0, 1.0, 0.0, 1.0);
					glVertex3f(position.x, position.y, position.z);
					glVertex3f(position.x + up_vector.x, position.y + up_vector.y, position.z + up_vector.z);

					glColor4f(0.0, 0.0, 1.0, 1.0);
					glVertex3f(position.x, position.y, position.z);
					glVertex3f(position.x + forward_vector.x, position.y + forward_vector.y, position.z + forward_vector.z);
					glEnd();*/


					right_vector.x += position.x;
					right_vector.y += position.y;
					right_vector.z += position.z;

					up_vector.x += position.x;
					up_vector.y += position.y;
					up_vector.z += position.z;

					forward_vector.x += position.x;
					forward_vector.y += position.y;
					forward_vector.z += position.z;

					//if(ed_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
					if(transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
					{
						glPointSize(6.0);
						renderer_Begin(GL_POINTS);
						renderer_Color4f(1.0, 0.0, 0.0, 0.0);
						renderer_Vertex3f(right_vector.x, right_vector.y, right_vector.z);
						renderer_Color4f(0.0, 1.0, 0.0, 0.0);
						renderer_Vertex3f(up_vector.x, up_vector.y, up_vector.z);
						renderer_Color4f(0.0, 0.0, 1.0, 0.0);
						renderer_Vertex3f(forward_vector.x, forward_vector.y, forward_vector.z);
						renderer_End();
					}
					/*else
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

						glEnd();
						glEnable(GL_CULL_FACE);
					}*/





				break;

				case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:

					glLineWidth(6.0);
					renderer_Begin(GL_LINE_LOOP);
					renderer_Color4f(1.0, 0.0, 0.0, 0.0);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						renderer_Vertex3f(position.x, position.y + ed_3d_rotation_handle_angles_lut[i][0] * d, position.z + ed_3d_rotation_handle_angles_lut[i][1] * d);
					}
					renderer_End();

					/*glPointSize(4.0);
					glBegin(GL_POINTS);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(ed_3d_handle_position.x, ed_3d_handle_position.y + ed_3d_rotation_handle_angles_lut[i][0], ed_3d_handle_position.z + ed_3d_rotation_handle_angles_lut[i][1]);
					}
					glEnd();*/


					renderer_Begin(GL_LINE_LOOP);
					renderer_Color4f(0.0, 1.0, 0.0, 0.0);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						renderer_Vertex3f(position.x + ed_3d_rotation_handle_angles_lut[i][1] * d, position.y, position.z + ed_3d_rotation_handle_angles_lut[i][0] * d);
					}
					renderer_End();
					/*glBegin(GL_POINTS);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(ed_3d_handle_position.x + angles_lut[i][1], ed_3d_handle_position.y, ed_3d_handle_position.z + angles_lut[i][0]);
					}
					glEnd();*/


					renderer_Begin(GL_LINE_LOOP);
					renderer_Color4f(0.0, 0.0, 1.0, 0.0);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						renderer_Vertex3f(position.x + ed_3d_rotation_handle_angles_lut[i][1] * d, position.y + ed_3d_rotation_handle_angles_lut[i][0] * d, position.z);
					}
					renderer_End();
					/*glBegin(GL_POINTS);
					for(i = 0; i < ROTATION_HANDLE_DIVS; i++)
					{
						glVertex3f(ed_3d_handle_position.x + angles_lut[i][1], ed_3d_handle_position.y + angles_lut[i][0], ed_3d_handle_position.z);
					}
					glEnd();*/
				break;
			}

			renderer_DisableImediateDrawing();
		}
	}

	glDisable(GL_POINT_SMOOTH);
	//glDisable(GL_STENCIL_TEST);


	glLineWidth(1.0);
	glPointSize(1.0);
	glEnable(GL_DEPTH_TEST);


	/*glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);*/
}


void editor_Draw3dCursor(vec3_t position)
{
	camera_t *active_camera;
//	active_camera = camera_GetActiveCamera();
    active_camera = (camera_t *)renderer_GetActiveView();
	vec4_t cursor_position;
	float ratio;
	float qt;
	float qr;

	cursor_position.vec3 = position;
	cursor_position.w = 1.0;

	mat4_t_vec4_t_mult(&active_camera->view_data.view_matrix, &cursor_position);
	qt = -active_camera->frustum.znear / active_camera->frustum.top;
	qr = -active_camera->frustum.znear / active_camera->frustum.right;

	if(cursor_position.z < 0.0)
	{
		cursor_position.x = (cursor_position.x * qr) / cursor_position.z;
		cursor_position.y = (cursor_position.y * qt) / cursor_position.z;

		renderer_SetShader(r_imediate_color_shader);
		renderer_EnableImediateDrawing();
		renderer_SetProjectionMatrix(NULL);
		renderer_SetViewMatrix(NULL);
		renderer_SetModelMatrix(NULL);

		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0xff, 0xff);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

		glLineWidth(1.0);
		glPointSize(16.0);

		renderer_Begin(GL_POINTS);
		renderer_Color4f(1.0, 1.0, 1.0, 0.0);
		renderer_Vertex3f(cursor_position.x, cursor_position.y, 0.0);
		renderer_End();

		renderer_Begin(GL_LINES);
		renderer_Color4f(0.0, 1.0, 0.0, 0.0);
		renderer_Vertex3f(cursor_position.x, cursor_position.y + 0.08 * qt, 0.0);
		renderer_Vertex3f(cursor_position.x, cursor_position.y - 0.08 * qt, 0.0);
		renderer_Color4f(1.0, 0.0, 0.0, 0.0);
		renderer_Vertex3f(cursor_position.x - 0.08 * qr, cursor_position.y, 0.0);
		renderer_Vertex3f(cursor_position.x + 0.08 * qr, cursor_position.y, 0.0);
		renderer_End();

		glPointSize(1.0);

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_POINT_SMOOTH);

		renderer_DisableImediateDrawing();
	}
}


