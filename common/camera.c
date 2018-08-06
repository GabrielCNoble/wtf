#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <intrin.h>

#include "matrix.h"
#include "camera.h"
#include "r_main.h"
#include "r_debug.h"
#include "w_common.h"
#include "material.h"
#include "c_memory.h"

#include "GL\glew.h"
//#include "draw.h"

//tern renderer_t renderer;
//extern camera_array camera_a;

//camera_list_t camera_list;

static int camera_list_size = 0;
int camera_count = 0;
//camera_t *camera_list = NULL;

camera_t *cameras = NULL;
camera_t *last_camera = NULL;

camera_t *active_camera;
camera_t *main_view;

extern camera_t *r_active_view;
extern camera_t *r_main_view;

static int active_camera_index;
static unsigned int camera_indexes[2048];

extern int r_window_width;
extern int r_window_height;

#define CLUSTER_WIDTH 32

#ifdef __cplusplus
extern "C"
{
#endif


/*
=============
camera_Init
=============
*/
int camera_Init()
{
	int i;

	camera_list_size = 64;
	camera_count = 0;

	for(i = 0; i < 2048; i++)
	{
		camera_indexes[i] = 0;
	}
	//camera_list = malloc(sizeof(camera_t ) * camera_list_size);

	/* space for name strings are prealocated... */
	/*for(i = 0; i < camera_list_size; i++)
	{
		camera_list[i].name = malloc(CAMERA_MAX_NAME_LEN);
	}*/


	//renderer_RegisterCallback(camera_UpdateCamerasCallback, RENDERER_RESOLUTION_CHANGE_CALLBACK);

	return 1;
}


/*
=============
camera_Finish
=============
*/
void camera_Finish()
{
	int i;

	camera_DestroyAllCameras();
	/*for(i = 0; i < camera_list_size; i++)
	{
		free(camera_list[i].name);
	}
	free(camera_list);*/
	return;
}

camera_t *camera_CreateCamera(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar, int bm_flags)
{
	//int camera_index = camera_count++;
	int name_len;
	camera_t *camera;
	int i;
	int bit_index;
	int int_index;

	camera = memory_Malloc(sizeof(camera_t), "camera_CreateCamera");
	camera->next = NULL;
	camera->prev = NULL;

	//name_len = strlen(name) + 1;
	//name_len = (name_len + 3) & (~3);
	//camera->name = malloc(name_len);
	//strcpy(camera->name, name);
	camera->name = memory_Strdup(name, "camera_CreateCamera");


	//camera->local_position = position;
	//memcpy(&camera->local_orientation, orientation, sizeof(mat3_t));

	camera->world_position = position;

	if(!orientation)
	{
		camera->world_orientation = mat3_t_id();
	}
	else
	{
		camera->world_orientation = *orientation;
	}


	//memcpy(&camera->world_orientation, orientation, sizeof(mat3_t));

	CreatePerspectiveMatrix(&camera->view_data.projection_matrix, fovy, width/height, znear, zfar, 0.0, 0.0, &camera->frustum);

	camera->width = width;
	camera->height = height;
	camera->zoom = 1.0;
	camera->fov_y = fovy;
	//camera->view_clusters = memory_Malloc(sizeof(cluster_t) * CLUSTERS_PER_ROW * CLUSTER_ROWS * CLUSTER_LAYERS, "camera_CreateCamera");
	//camera->visible_lights = memory_Malloc((MAX_WORLD_LIGHTS >> 5) * sizeof(int), "camera_CreateCamera");

	camera_InitViewData(&camera->view_data);

	/*camera->view_data.view_lights_list_cursor = 0;
	camera->view_data.view_lights_list_size = 32;
	camera->view_data.view_lights = memory_Malloc(sizeof(view_light_t) * camera->view_data.view_lights_list_size, "camera_CreateCamera");

	camera->view_data.view_portals_list_cursor = 0;
	camera->view_data.view_portals_list_size = 32;
	camera->view_data.view_portals = memory_Malloc(sizeof(unsigned short) * camera->view_data.view_portals_list_size, "camera_CreateCamera");

	camera->view_data.visible_entities_list_size = 0;
	camera->view_data.visible_entities_list_cursor = 0;
	camera->view_data.visible_entities = NULL;

	camera->view_data.view_draw_command_frame = 0;
	camera->view_data.view_draw_command_list_size = 512;
	camera->view_data.view_draw_command_list_cursor = 0;
	camera->view_data.view_draw_commands = memory_Malloc(sizeof(draw_command_t) * camera->view_data.view_draw_command_list_size, "camera_CreateCamera");

	camera->view_data.view_material_refs = memory_Malloc(sizeof(view_material_ref_record_t) * (MAX_MATERIALS + 1), "camera_CreateCamera");
	camera->view_data.view_material_refs++;

	camera->view_data.view_leaves_list_cursor = 0;
	camera->view_data.view_leaves_list_size = 256;
	camera->view_data.view_leaves = memory_Malloc(sizeof(bsp_dleaf_t *) * camera->view_data.view_leaves_list_size, "camera_CreateCamera");

	for(i = -1; i < MAX_MATERIALS; i++)
	{
		camera->view_data.view_material_refs[i].last_touched = 0;
		camera->view_data.view_material_refs[i].frame_ref_count = 0;
	}*/

	camera->x_shift = 0.0;
	camera->y_shift = 0.0;


	/*for(i = 0; i < 2048; i++)
	{
		int_index = i >> 5;
		bit_index = i % 32;

		if(!(camera_indexes[int_index] & (1 << bit_index)))
		{
			camera_indexes[int_index] |= (1 << bit_index);
			break;
		}
	}*/

	//camera->camera_index = i;

	//camera->camera_index = camera_index;
	camera->bm_flags = 0;
	//camera->assigned_node=scenegraph_AddNode(NODE_CAMERA, camera_index, -1, camera->name);
	camera_ComputeWorldToCameraMatrix(camera);

	if(!cameras)
	{
		cameras = camera;
	}
	else
	{
		last_camera->next = camera;
		camera->prev = last_camera;
	}
	last_camera = camera;

	camera_count++;

	return camera;
}

void camera_InitViewData(view_data_t *view_data)
{
	int i;

	/*view_data->view_lights_list_cursor = 0;
	view_data->view_lights_list_size = 32;
	view_data->view_lights = memory_Malloc(sizeof(view_light_t) * view_data->view_lights_list_size, "camera_InitViewData");

	view_data->view_portals_frame = 0;
	view_data->view_portals_list_cursor = 0;
	view_data->view_portals_list_size = 32;
	view_data->view_portals = memory_Malloc(sizeof(unsigned short) * view_data->view_portals_list_size, "camera_InitViewData");

	view_data->view_entities_list_size = 1024;
	view_data->view_entities_list_cursor = 0;
	view_data->view_entities = memory_Malloc(sizeof(unsigned short ) * view_data->view_entities_list_size, "camera_InitViewData");

	view_data->view_draw_command_frame = 0;
	view_data->view_draw_command_list_size = 512;
	view_data->view_draw_command_list_cursor = 0;
	view_data->view_draw_commands = memory_Malloc(sizeof(draw_command_t) * view_data->view_draw_command_list_size, "camera_InitViewData");

	view_data->view_leaves_list_cursor = 0;
	view_data->view_leaves_list_size = 256;
	view_data->view_leaves = memory_Malloc(sizeof(bsp_dleaf_t *) * view_data->view_leaves_list_size, "camera_InitViewData");

	view_data->view_triangles_size = 1024;
	view_data->view_triangles_cursor = 0;
	view_data->view_triangles = memory_Malloc(sizeof(bsp_striangle_t) * view_data->view_triangles_size, "camera_InitViewData");*/
}

void camera_DeleteViewData(view_data_t *view_data)
{
	/*memory_Free(view_data->view_lights);
	memory_Free(view_data->view_portals);
	memory_Free(view_data->view_entities);
	memory_Free(view_data->view_draw_commands);
	memory_Free(view_data->view_triangles);
	memory_Free(view_data->view_leaves);*/
}

void camera_DestroyCamera(camera_t *camera)
{
	if(camera)
	{
		if(camera == cameras)
		{
			cameras = cameras->next;

			if(cameras)
			{
				cameras->prev = NULL;
			}
		}
		else
		{
			camera->prev->next = camera->next;

			if(camera->next)
			{
				camera->next->prev = camera->prev;
			}
			else
			{
				last_camera = last_camera->prev;
			}
		}

		camera_DeleteViewData(&camera->view_data);
		memory_Free(camera->name);
		memory_Free(camera);

		camera_count--;
	}
}

void camera_DestroyAllCameras()
{
	while(cameras)
	{
		camera_DestroyCamera(cameras);
	}

	camera_count = 0;
}

void camera_SetCameraProjectionMatrix(camera_t *camera, float width, float height, float znear, float zfar, float fovy)
{
	CreatePerspectiveMatrix(&camera->view_data.projection_matrix, fovy, width/height, znear, zfar, 0.0, 0.0, &camera->frustum);
	camera->width = width;
	camera->height = height;
}


/*
=============
camera_SetCameraByIndex
=============
*/
/*void camera_SetCameraByIndex(int camera_index)
{
	camera_t *camera;
	if(camera_index > -1 && camera_index < camera_list_size)
	{
		active_camera_index = camera_index;
		camera = &camera_list[camera_index];

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&camera->projection_matrix.floats[0][0]);
		glMatrixMode(GL_MODELVIEW);
	}
}*/


void camera_SetCamera(camera_t *camera)
{
	//active_camera_index = camera->camera_index;
	if(camera)
	{
		active_camera = camera;
		r_active_view = camera;

		camera_Activate(camera);
		//camera->bm_flags &= ~CAMERA_INACTIVE;

		//glMatrixMode(GL_PROJECTION);
		//glLoadMatrixf(&camera->projection_matrix.floats[0][0]);
		//glMatrixMode(GL_MODELVIEW);
	}
}

void camera_SetMainViewCamera(camera_t *camera)
{
	main_view = camera;
	r_main_view = camera;
}

void camera_Activate(camera_t *camera)
{
	if(camera)
	{
		camera->bm_flags &= ~CAMERA_INACTIVE;
	}
}

void camera_Deactivate(camera_t *camera)
{
	if(camera)
	{
		camera->bm_flags |= CAMERA_INACTIVE;
	}
}


/*
=============
camera_ComputeWorldToCameraMatrix
=============
*/
void camera_ComputeWorldToCameraMatrix(camera_t *camera)
{
	camera->view_data.view_matrix.floats[0][0] = camera->world_orientation.floats[0][0];
	camera->view_data.view_matrix.floats[1][0] = camera->world_orientation.floats[0][1];
	camera->view_data.view_matrix.floats[2][0] = camera->world_orientation.floats[0][2];

	camera->view_data.view_matrix.floats[3][0] =  (-camera->world_position.floats[0]) * camera->view_data.view_matrix.floats[0][0]	+
										  		  (-camera->world_position.floats[1]) * camera->view_data.view_matrix.floats[1][0]	+
										  		  (-camera->world_position.floats[2]) * camera->view_data.view_matrix.floats[2][0];



	camera->view_data.view_matrix.floats[0][1] = camera->world_orientation.floats[1][0];
	camera->view_data.view_matrix.floats[1][1] = camera->world_orientation.floats[1][1];
	camera->view_data.view_matrix.floats[2][1] = camera->world_orientation.floats[1][2];

	camera->view_data.view_matrix.floats[3][1] =  (-camera->world_position.floats[0]) * camera->view_data.view_matrix.floats[0][1]	+
										  		  (-camera->world_position.floats[1]) * camera->view_data.view_matrix.floats[1][1]	+
										  		  (-camera->world_position.floats[2]) * camera->view_data.view_matrix.floats[2][1];




	camera->view_data.view_matrix.floats[0][2] = camera->world_orientation.floats[2][0];
	camera->view_data.view_matrix.floats[1][2] = camera->world_orientation.floats[2][1];
	camera->view_data.view_matrix.floats[2][2] = camera->world_orientation.floats[2][2];

	camera->view_data.view_matrix.floats[3][2] =  (-camera->world_position.floats[0]) * camera->view_data.view_matrix.floats[0][2]	+
										 		  (-camera->world_position.floats[1]) * camera->view_data.view_matrix.floats[1][2]	+
										  		  (-camera->world_position.floats[2]) * camera->view_data.view_matrix.floats[2][2];


	camera->view_data.view_matrix.floats[0][3] = 0.0;
	camera->view_data.view_matrix.floats[1][3] = 0.0;
	camera->view_data.view_matrix.floats[2][3] = 0.0;
	camera->view_data.view_matrix.floats[3][3] = 1.0;

}


/*
=============
camera_TranslateCamera
=============
*/
void camera_TranslateCamera(camera_t *camera, vec3_t direction, float amount, int b_set)
{
	if(b_set)
	{
		camera->world_position.floats[0]=direction.floats[0]*amount;
		camera->world_position.floats[1]=direction.floats[1]*amount;
		camera->world_position.floats[2]=direction.floats[2]*amount;
	}
	else
	{
		camera->world_position.floats[0] += direction.floats[0]*amount;
		camera->world_position.floats[1] += direction.floats[1]*amount;
		camera->world_position.floats[2] += direction.floats[2]*amount;
	}

	camera_ComputeWorldToCameraMatrix(camera);
	return;
}


/*
=============
camera_RotateCamera
=============
*/
void camera_RotateCamera(camera_t *camera, vec3_t axis, float angle, int b_set)
{
	//mat3_t_rotate(&camera->local_orientation, axis, -angle, b_set);
	mat3_t_rotate(&camera->world_orientation, axis, -angle, b_set);
	camera_ComputeWorldToCameraMatrix(camera);
	return;
}


void camera_PitchYawCamera(camera_t *camera, float yaw, float pitch)
{

	mat3_t p;
	mat3_t y;

	float s;
	float c;

	//unsigned long long start;
	//unsigned long long end;
	//mat3_t temp;

	//start = _rdtsc();

	//mat3_t_rotate(&y, vec3(0.0, 1.0, 0.0), yaw, 1);
	//mat3_t_rotate(&p, vec3(1.0, 0.0, 0.0), pitch, 1);

	/* First apply pitch, THEN yaw. This is necesary
	because the camera is using the world vector
	(1, 0, 0) as its right vector, which is not
	modified when the camera turns. So, if applying
	the yaw first, the camera's right vector might
	become its forward vector at times, so it will
	roll instead of pitch. To apply the yaw before
	the pitch, the world right vector must be rotated
	by the yaw matrix, so it will always be pointing
	to the right relative to the camera.*/
	//mat3_t_mult(&camera->local_orientation, &p, &y);

	s = sin(pitch * 3.14159265);
	c = cos(pitch * 3.14159265);

	p.floats[0][0] = 1.0;
	p.floats[0][1] = 0.0;
	p.floats[0][2] = 0.0;

	p.floats[1][0] = 0.0;
	p.floats[1][1] = c;
	p.floats[1][2] = s;

	p.floats[2][0] = 0.0;
	p.floats[2][1] = -s;
	p.floats[2][2] = c;



	s = sin(yaw * 3.14159265);
	c = cos(yaw * 3.14159265);

	y.floats[0][0] = c;
	y.floats[0][1] = 0.0;
	y.floats[0][2] = -s;

	y.floats[1][0] = 0.0;
	y.floats[1][1] = 1.0;
	y.floats[1][2] = 0.0;

	y.floats[2][0] = s;
	y.floats[2][1] = 0.0;
	y.floats[2][2] = c;


	//end = _rdtsc();

	//printf("%llu\n", end - start);

	mat3_t_mult(&camera->world_orientation, &p, &y);



	//camera_ComputeWorldToCameraMatrix(camera);

	//mat3_t_mult(&camera->local_orientation, &y, &p);

}


/*
=============
camera_GetActiveCamera
=============
*/
camera_t *camera_GetActiveCamera()
{
	//return &camera_list[active_camera_index];
	return r_active_view;
}

camera_t *camera_GetMainViewCamera()
{
	return r_main_view;
}

camera_t *camera_GetCamera(char *name)
{
	camera_t *c;
	c = cameras;

	while(c)
	{
		if(!strcmp(name, c->name))
		{
			return c;
		}
		c = c->next;
	}

	return NULL;
}



float camera_BoxScreenArea(camera_t *camera, vec3_t center, vec3_t extents)
{
	int i;

	float x_max;
	float x_min;

	float y_max;
	float y_min;

	float qt;
	float qr;
	float nznear;

	vec4_t corners[8];

	int positive_z;

	float screen_area;

	mat4_t view_projection_matrix;

	nznear = -camera->frustum.znear;

	qr = nznear / camera->frustum.right;
	qt = nznear / camera->frustum.top;


	for(i = 0; i < 8; i++)
	{
		corners[i].vec3 = center;
		corners[i].w = 1.0;
	}

	corners[0].x -= extents.x;
	corners[0].y += extents.y;
	corners[0].z -= extents.z;

	corners[1].x -= extents.x;
	corners[1].y += extents.y;
	corners[1].z += extents.z;

	corners[2].x += extents.x;
	corners[2].y += extents.y;
	corners[2].z += extents.z;

	corners[3].x += extents.x;
	corners[3].y += extents.y;
	corners[3].z -= extents.z;



	corners[4].x -= extents.x;
	corners[4].y -= extents.y;
	corners[4].z -= extents.z;

	corners[5].x -= extents.x;
	corners[5].y -= extents.y;
	corners[5].z += extents.z;

	corners[6].x += extents.x;
	corners[6].y -= extents.y;
	corners[6].z += extents.z;

	corners[7].x += extents.x;
	corners[7].y -= extents.y;
	corners[7].z -= extents.z;

	x_max = -10.0;
	y_max = -10.0;

	x_min = 10.0;
	y_min = 10.0;

	positive_z = 0;

	for(i = 0; i < 8; i++)
	{
		mat4_t_vec4_t_mult(&camera->view_data.view_matrix, &corners[i]);

		if(corners[i].z > nznear)
		{
			corners[i].z = nznear;
			positive_z++;
		}

		corners[i].x = (corners[i].x * qr) / corners[i].z;
		corners[i].y = (corners[i].y * qt) / corners[i].z;

		if(corners[i].x > x_max) x_max = corners[i].x;
		if(corners[i].x < x_min) x_min = corners[i].x;

		if(corners[i].y > y_max) y_max = corners[i].y;
		if(corners[i].y < y_min) y_min = corners[i].y;
	}

	if(x_max > 1.0) x_max = 1.0;
	else if(x_max < -1.0) x_max = -1.0;

	if(x_min < -1.0) x_min = -1.0;
	else if(x_min > 1.0) x_min = 1.0;

	if(y_max > 1.0) y_max = 1.0;
	else if(y_max < -1.0) y_max = -1.0;

	if(y_min < -1.0) y_min = -1.0;
	else if(y_min > 1.0) y_min = 1.0;

/*	renderer_Draw2dLine(vec2(x_min, y_max), vec2(x_min, y_min), vec3_t_c(0.0, 1.0, 0.0), 1.0, 0);
	renderer_Draw2dLine(vec2(x_min, y_min), vec2(x_max, y_min), vec3_t_c(0.0, 1.0, 0.0), 1.0, 0);
	renderer_Draw2dLine(vec2(x_max, y_min), vec2(x_max, y_max), vec3_t_c(0.0, 1.0, 0.0), 1.0, 0);
	renderer_Draw2dLine(vec2(x_max, y_max), vec2(x_min, y_max), vec3_t_c(0.0, 1.0, 0.0), 1.0, 0);*/

	screen_area = (x_max - x_min) * (y_max - y_min);

	if(positive_z < 8 && screen_area > 0.0)
	{
		return screen_area;
	}

	return 0.0;
}

/*camera_t *camera_GetCameraByIndex(int camera_index)
{
	if(camera_index > -1 && camera_index < camera_list_size)
	{
		return &camera_list[camera_index];
	}

	return NULL;
}*/


/*void camera_UpdateCamerasCallback()
{
	int i;
	camera_t *camera;

	for(i = 0; i < camera_count; i++)
	{
		camera = &camera_list[i];

		if(camera->bm_flags & CAMERA_UPDATE_ON_RESIZE)
		{
			CreatePerspectiveMatrix(&camera->projection_matrix, camera->fov_y, (float)r_window_width/(float)r_window_height, camera->frustum.znear, camera->frustum.zfar, 0.0, 0.0, &camera->frustum);
		}

	}
}*/



#ifdef __cplusplus
}
#endif


