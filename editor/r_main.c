#include <stdio.h>
#include <string.h>

#include "r_common.h"
#include "r_main.h"


#include "camera.h"
//#include "player.h"
#include "shader.h"
//#include "physics.h"
#include "mesh.h"
#include "world.h"
#include "material.h"
#include "texture.h"
#include "l_main.h"
#include "l_cache.h"
#include "brush.h"
#include "editor.h"
#include "gui.h"
#include "bsp.h"



SDL_Window *window;
SDL_GLContext context;


int window_width;
int window_height;
int window_flags;


int z_pre_pass_shader;
int forward_pass_shader;
int skybox_shader;
//unsigned int cursor_framebuffer_id;
//unsigned int cursor_color_texture_id;
//unsigned int cursor_depth_texture_id;


/* from gpu.c */
extern unsigned int gpu_heap;

/* from world.c */
extern unsigned int world_element_buffer;
extern int world_triangle_group_count;
extern triangle_group_t *world_triangle_groups;


/* from collision.c */
extern int collision_geometry_start;
extern int collision_geometry_count;
extern int collision_geometry_vertice_count;
extern vec3_t *collision_geometry_positions;
extern vec3_t *collision_geometry_normals;


/* from material.c */
extern material_t *materials;

/* from texture.c */
extern texture_t *textures;


/* from l_main.c */
//extern light_position_t *visible_light_positions;
//extern light_params_t *visible_light_params;
//extern light_position_t *light_positions;
extern int visible_lights[];
extern int visible_light_count;


/* from brush.c */
extern int brush_count;
extern brush_t *brushes;


/* from gui.c */
extern widget_t *widgets;
extern widget_t *last_widget;
extern widget_t *top_widget;
extern mat4_t gui_projection_matrix;

/* from bsp.c */
//extern bsp_node_t *world_bsp;




#define MAX_RENDER_FUNCTIONS 16
static int render_function_count = 0;
static void (*renderer_RegisteredFunction[MAX_RENDER_FUNCTIONS])(void);

unsigned int r_frame = 0;

void renderer_Init(int width, int height, int init_mode)
{
	char *ext_str;
	char *sub_str;
	
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("oh shit...\n");
		exit(1);
	}
	
	window_flags = SDL_WINDOW_OPENGL;
	SDL_DisplayMode display_mode;
	
	int r;
	
	if(init_mode == INIT_FULLSCREEN)
	{
		SDL_GetDisplayMode(0, 0, &display_mode);
		window_width = display_mode.w;
		window_height = display_mode.h;
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	else
	{
		window_width = width;
		window_height = height;	
	}
	
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	
	window = SDL_CreateWindow("wtf editor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, window_flags);
	context = SDL_GL_CreateContext(window);
	
	SDL_GL_MakeCurrent(window, context);

	SDL_GL_SetSwapInterval(1);
	
	if(glewInit() != GLEW_NO_ERROR)
	{
		printf("oh shit...\n");
		exit(2);
	}
	

	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearStencil(0);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glStencilMask(0xff);
	
	
	
	ext_str = (char *)glGetString(GL_EXTENSIONS);
	
	sub_str = strstr(ext_str, "GL_ARB_seamless_cube_map");
	if(sub_str)
	{
		//bm_extensions |= EXT_TEXTURE_CUBE_MAP_SEAMLESS;
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	}
	
	/*sub_str = strstr(ext_str, "GL_EXT_packed_depth_stencil");
	if(sub_str)
	{
		bm_extensions |= EXT_PACKED_DEPTH_STENCIL;
	}*/
	
	/*sub_str = strstr(ext_str, "GL_EXT_transform_feedback");
	if(sub_str)
	{
		bm_extensions |= EXT_TRANSFORM_FEEDBACK;
	}*/
	
	sub_str = strstr(ext_str, "GL_ARB_uniform_buffer_object");
	if(!sub_str)
	{
		printf("ERROR: extension GL_ARB_uniform_buffer_object not supported by current driver! Try to update it and then run the application again.\n");
		exit(3);
	}
	
	/*sub_str = strstr(ext_str, "GL_ARB_multi_draw");
	if(!sub_str)
	{
		printf("ERROR: extension GL_ARB_multi_draw not supported by current driver!\n Try to update it and then run the application again.\n");
		exit(3);
	}*/
	
	/*sub_str = strstr(ext_str, "GL_ARB_multi_draw_indirect");
	if(sub_str)
	{
		bm_extensions |= EXT_MULTI_DRAW_INDIRECT;
	}*/
	
	/*sub_str = strstr(ext_str, "GL_ARB_shader_draw_parameters");
	if(sub_str)
	{
		bm_extensions |= EXT_SHADER_DRAW_PARAMETERS;
	}*/
}

void renderer_Finish()
{	
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void renderer_RegisterFunction(void (*r_fn)(void))
{
	renderer_RegisteredFunction[render_function_count++] = r_fn;
}


void renderer_OpenFrame()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void renderer_DrawFrame()
{
	int i;
	light_BindCache();
	gpu_BindGpuHeap();
	renderer_DrawShadowMaps();
	renderer_ZPrePass();
	renderer_DrawSkyBox();
	renderer_DrawWorld();	
	
	for(i = 0; i < render_function_count; i++)
	{
		renderer_RegisteredFunction[i]();
	}
	
	renderer_DrawGUI();
	gpu_UnbindGpuHeap();
	light_UnbindCache();
}

void renderer_ZPrePass()
{
	
	//return;
	
	
	int i;
	int c;
	camera_t *active_camera = camera_GetActiveCamera();	
	//mat4_t transform;
	//mat4_t temp_transform;
	//mat4_t camera_transform;
	//mat4_t weapon_transform;
	//mat4_t model_view_matrix;	
	//mat3_t weapon_rot = mat3_t_id();
	//vec3_t weapon_position;
	triangle_group_t *triangle_group;

	
	//glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	shader_UseShader(z_pre_pass_shader);
	
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_TRUE);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	glStencilFunc(GL_ALWAYS, 0x01, 0xff);
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	
	c = world_triangle_group_count;
	
	for(i = 0; i < c; i++)
	{	
		triangle_group = &world_triangle_groups[i];
		glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
	}
	
	//glDisable(GL_STENCIL_TEST);
	
	
	/*if(active_player)
	{
		mat4_t_compose(&camera_transform, &active_camera->world_orientation, active_camera->world_position);
		weapon_position = vec3(1.0, -1.0, -1.0);
		weapon_position.z += active_player->weapon_z_shift;
		weapon_position.x += active_player->weapon_x_shift;
		weapon_position.y -= active_player->weapon_y_shift;
				
		mat4_t_compose(&transform, &weapon_rot, weapon_position);
		mat4_t_scale(&transform, vec3(0.0, 0.0, 1.0), 0.5);
		
		mat4_t_mult_fast(&weapon_transform, &transform, &camera_transform);
		mat4_t_mult_fast(&model_view_matrix, &weapon_transform, &active_camera->world_to_camera_matrix);
		
		
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glStencilOp(GL_INCR, GL_INCR, GL_INCR);
		
		glLoadMatrixf(&model_view_matrix.floats[0][0]);
		glDrawArrays(GL_TRIANGLES, active_player->weapon_start, active_player->weapon_count);
	}*/
	
	/*c = player_count;
	
	glStencilFunc(GL_NOTEQUAL, 1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	weapon_position = vec3(1.0, -1.0, -2.0);
	
	for(i = 0; i < c; i++)
	{
		if(&players[i] != active_player)
		{
			//mat4_t_compose(&transform, &players[i].player_orientation, players[i].player_position);		
			//mat4_t_mult_fast(&model_view_matrix, &transform, &active_camera->world_to_camera_matrix);
			
			
			//glLoadMatrixf(&model_view_matrix.floats[0][0]);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, body_color);
			
			glLoadMatrixf(&visible_players_body_transforms[i].floats[0][0]);
			glDrawArrays(GL_TRIANGLES, players[i].body_start, players[i].body_count);
			
			
			
			//mat4_t_compose(&transform, &players[i].player_camera->world_orientation, players[i].player_camera->world_position);
			
			//mat4_t_compose(&weapon_transform, &weapon_rot, weapon_position);
			//mat4_t_mult_fast(&temp_transform, &weapon_transform, &transform);
			//mat4_t_mult_fast(&model_view_matrix, &temp_transform, &active_camera->world_to_camera_matrix);
			
			
			//glLoadMatrixf(&model_view_matrix.floats[0][0]);
			//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, weapon_color);
			glLoadMatrixf(&visible_players_weapon_transforms[i].floats[0][0]);
			glDrawArrays(GL_TRIANGLES, players[i].weapon_start, players[i].weapon_count);
			
		}
	}*/
	

	
	
	
	
/*	c = triangle_group_count;
	
	for(i = 0; i < c; i++)
	{
		triangle_group = &triangle_groups[i];
		glDrawElements(GL_TRIANGLES, triangle_group->vertex_count, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
	}*/
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);
}


void renderer_DrawShadowMaps()
{
	
}



void renderer_DrawGUI()
{
	widget_t *w;
	
	int widget_stack_top = -1;
	int b_do_top = 0;
	widget_t *widget_stack[128];
	button_t *button;
	
	short x = 0;
	short y = 0;
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&gui_projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glUseProgram(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	
	w = last_widget;
	
	_do_top:
	
	while(w)
	{
		
		/* Ignore the top widget if it isn't its
		turn to be rendered... */
		if(w == top_widget)
		{
			if(!b_do_top)
			{
				w = w->prev;
				continue;
			}
		}
		
		switch(w->type)
		{
			case WIDGET_NONE:
				glBegin(GL_QUADS);
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					glColor3f(0.35, 0.35, 0.35);
				}
				else
				{
					glColor3f(0.3, 0.3, 0.3);
				}
						
				//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
				
				if(w->nestled)
				{
					
					x += w->x;
					y += w->y;
					
					widget_stack_top++;
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					continue;
				}
			break;
			
			case WIDGET_BUTTON:
				button = (button_t *)w;
				glBegin(GL_QUADS);
				
				if(button->bm_button_flags & BUTTON_PRESSED)
				{
					glColor3f(0.35, 0.35, 0.35);
				}
				else
				{
					if(w->bm_flags & WIDGET_MOUSE_OVER)
					{
						glColor3f(0.5, 0.5, 0.5);
					}
					else
					{
						glColor3f(0.4, 0.4, 0.4);
					}
				}
				
				//glRectf(w->x + w->w, w->y - w->h, w->x - w->w, w->y + w->h);
				glVertex3f(w->x - w->w + x, w->y + w->h + y, 0.0);
				glVertex3f(w->x - w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y - w->h + y, 0.0);
				glVertex3f(w->x + w->w + x, w->y + w->h + y, 0.0);
				glEnd();
			break;
		}
		
		
		/* If this is the top widget, and everything
		nestled in it was already processed, quit the loop
		to avoid rerendering anything on the list... */
		if(b_do_top && widget_stack_top < 0)
		{
			break;
		}
		
		_advance_widget:
		
		w = w->prev;
		
		if(!w)
		{
			if(widget_stack_top >= 0)
			{
				w = widget_stack[widget_stack_top];
				x -= w->x;
				y -= w->y;
				widget_stack_top--;
				goto _advance_widget;
			}
		}
		
	}
	
	
	/* All the list (excluding the top widget) have been
	drawn, so repeat the loop just for the top widget 
	(and any eventual nestled widget)... */
	if(!b_do_top)
	{
		b_do_top = 1;
		w = top_widget;
		goto _do_top;
	}
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
}

/*void renderer_DrawCollisionGeometry()
{
	
	int i;
	int c = collision_geometry_vertice_count;
	camera_t *active_camera = camera_GetActiveCamera();
	
	glUseProgram(0);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	glLineWidth(2.0);
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0);
	for(i = 0; i < c;)
	{
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		i++;
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		i++;
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		i++;
	}
	glEnd();
	
	
	glBegin(GL_LINES);
	glColor3f(0.0, 1.0, 0.0);
	for(i = 0; i < c;)
	{
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		glVertex3f(collision_geometry_positions[i].x + collision_geometry_normals[i].x, 
				   collision_geometry_positions[i].y + collision_geometry_normals[i].y, 
				   collision_geometry_positions[i].z + collision_geometry_normals[i].z);
		i++;
		
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		glVertex3f(collision_geometry_positions[i].x + collision_geometry_normals[i].x, 
				   collision_geometry_positions[i].y + collision_geometry_normals[i].y, 
				   collision_geometry_positions[i].z + collision_geometry_normals[i].z);
		i++;
		
		glVertex3f(collision_geometry_positions[i].x, collision_geometry_positions[i].y, collision_geometry_positions[i].z);
		glVertex3f(collision_geometry_positions[i].x + collision_geometry_normals[i].x, 
				   collision_geometry_positions[i].y + collision_geometry_normals[i].y, 
				   collision_geometry_positions[i].z + collision_geometry_normals[i].z);
		i++;
		
	}
	glEnd();
	
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glLineWidth(1.0);
	//gpu_BindGpuHeap();
	
	//glDrawArrays(GL_TRIANGLES, collision_geometry_start, collision_geometry_count);
	
	
	//gpu_UnbindGpuHeap();
}*/



/*void renderer_DrawPlayers()
{
	int i;
	int c = player_count;
	camera_t *active_camera = camera_GetActiveCamera();
	mat4_t transform;
	mat4_t temp_transform;
	mat4_t weapon_transform;
	mat4_t model_view_matrix;
	float body_color[] = {1.0, 0.0, 0.0, 1.0};
	float weapon_color[] = {0.0, 1.0, 0.0, 1.0};
	
	mat3_t weapon_rot = mat3_t_id();
	vec3_t weapon_position = vec3(1.0, -1.0, -2.0);
	
	glEnable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
	
	glStencilFunc(GL_NOTEQUAL, 1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	shader_UseShader(forward_pass_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);
	
	
	for(i = 0; i < c; i++)
	{
		if(&players[i] != active_player)
		{

			glLoadMatrixf(&visible_players_body_transforms[i].floats[0][0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, body_color);
			glDrawArrays(GL_TRIANGLES, players[i].body_start, players[i].body_count);

			glLoadMatrixf(&visible_players_weapon_transforms[i].floats[0][0]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, weapon_color);
			glDrawArrays(GL_TRIANGLES, players[i].weapon_start, players[i].weapon_count);
			
			
		}
	}
	
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_TRUE);
	
}*/

/*void renderer_DrawActivePlayer()
{
	int i;
	int c = player_count;
	camera_t *active_camera = camera_GetActiveCamera();
	float color[] = {0.0, 0.0, 1.0, 1.0};
	
	mat4_t transform;
	mat4_t camera_transform;
	mat4_t weapon_transform;
	mat4_t model_view_matrix;	
	mat3_t weapon_rot = mat3_t_id();
	vec3_t weapon_position;
	
	
	if(!active_player) return;
	
	glBindBuffer(GL_ARRAY_BUFFER, gpu_heap);
	shader_UseShader(forward_pass_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);

	
	mat4_t_compose(&camera_transform, &active_camera->world_orientation, active_camera->world_position);

		
	weapon_position = vec3(1.0, -1.0, -1.0);
	weapon_position.z += active_player->weapon_z_shift;
	weapon_position.x += active_player->weapon_x_shift;
	weapon_position.y -= active_player->weapon_y_shift;
			
	mat4_t_compose(&transform, &weapon_rot, weapon_position);
	mat4_t_scale(&transform, vec3(0.0, 0.0, 1.0), 0.5);
	
	glDepthMask(GL_FALSE);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	mat4_t_mult_fast(&weapon_transform, &transform, &camera_transform);
	mat4_t_mult_fast(&model_view_matrix, &weapon_transform, &active_camera->world_to_camera_matrix);
	glLoadMatrixf(&model_view_matrix.floats[0][0]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
	glDrawArrays(GL_TRIANGLES, active_player->weapon_start, active_player->weapon_count);
	
	glDisable(GL_STENCIL_TEST);
	glDepthMask(GL_TRUE);
			
}*/

/*void renderer_DrawBlocks()
{
	

}

void renderer_DrawBVH()
{
	
	
}*/

/*void renderer_CullWorld()
{
	
}*/

void renderer_DrawWorld()
{
	
	
	camera_t *active_camera = camera_GetActiveCamera();	
	triangle_group_t *triangle_group;
	material_t *material;
	int i;
	int c = brush_count;
	
	int j;
	int k;
	
	float color[] = {1.0, 1.0, 1.0, 1.0};

	//gpu_BindGpuHeap();
	shader_UseShader(forward_pass_shader);
	//shader_SetCurrentShaderUniform1i(UNIFORM_LIGHT_COUNT, visible_light_count);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_FLAGS, 0);
	
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);
		
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, world_element_buffer);
	c = world_triangle_group_count;
	
	k = visible_light_count;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 0x0, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glDepthMask(GL_TRUE);
	glBlendFunc(GL_ONE, GL_ONE);
	
	for(j = 0; j < k; j++)
	{
		light_SetLight(visible_lights[j]);
		
		for(i = 0; i < c; i++)
		{	
			triangle_group = &world_triangle_groups[i];
			
			material = &materials[triangle_group->material_index];
			
			color[0] = (float)material->r / 255.0;
			color[1] = (float)material->g / 255.0;
			color[2] = (float)material->b / 255.0;
			//color[3] = (float)material->a / 255.0;
			color[3] = 1.0;
				
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
			
			glDrawElements(GL_TRIANGLES, triangle_group->next, GL_UNSIGNED_INT, (void *)(triangle_group->start * sizeof(int)));
		
		}
	}
	
	glDisable(GL_BLEND);	
	glDisable(GL_STENCIL_TEST);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
}

void renderer_DrawSkyBox()
{
	mat4_t world_to_camera_rotation;
	camera_t *active_camera = camera_GetActiveCamera();
	int pos_x;
	int neg_x;
	int pos_y;
	int neg_y;
	int pos_z;
	int neg_z;
	int skybox_tex;
	memcpy(&world_to_camera_rotation, &active_camera->world_to_camera_matrix, sizeof(mat4_t));
	
	world_to_camera_rotation.floats[3][0] = 0.0;
	world_to_camera_rotation.floats[3][1] = 0.0;
	world_to_camera_rotation.floats[3][2] = 0.0;
	
	
	skybox_tex = texture_GetTexture("env");
	
	/*glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();*/
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&world_to_camera_rotation.floats[0][0]);
	shader_UseShader(skybox_shader);
	shader_SetCurrentShaderUniform1i(UNIFORM_TEXTURE_CUBE_SAMPLER0, 0);
	texture_BindTexture(skybox_tex, GL_TEXTURE0);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0x0, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glBegin(GL_QUADS);
	
	//#if 0
	/* +X */
	glVertex3f( 1.0, 1.0,-1.0);
	glVertex3f( 1.0,-1.0,-1.0);
	glVertex3f( 1.0,-1.0, 1.0);
	glVertex3f( 1.0, 1.0, 1.0);
	/* -X */
	glVertex3f(-1.0, 1.0, 1.0);
	glVertex3f(-1.0,-1.0, 1.0);
	glVertex3f(-1.0,-1.0,-1.0);
	glVertex3f(-1.0, 1.0,-1.0);
	
	/* +Y */
	glVertex3f(-1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0,-1.0);
	glVertex3f( 1.0, 1.0,-1.0);
	glVertex3f( 1.0, 1.0, 1.0);
	
	/* -Y */
	glVertex3f( 1.0,-1.0, 1.0);
	glVertex3f( 1.0,-1.0,-1.0);
	glVertex3f(-1.0,-1.0,-1.0);
	glVertex3f(-1.0,-1.0, 1.0);

	/* +Z */
	glVertex3f( 1.0, 1.0, 1.0);
	glVertex3f( 1.0,-1.0, 1.0);
	glVertex3f(-1.0,-1.0, 1.0);
	glVertex3f(-1.0, 1.0, 1.0);
	
	/* -Z */
	glVertex3f(-1.0, 1.0,-1.0);
	glVertex3f(-1.0,-1.0,-1.0);
	glVertex3f( 1.0,-1.0,-1.0);
	glVertex3f( 1.0, 1.0,-1.0);
	
	//#endif
	
	glEnd();
		
	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_TEXTURE_2D);
	//glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	
	/*glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);*/
	
}

void renderer_CloseFrame()
{
	SDL_GL_SwapWindow(window);
	r_frame++;
}

void renderer_GetWindowSize(int *w, int *h)
{
	*w = window_width;
	*h = window_height;
}





