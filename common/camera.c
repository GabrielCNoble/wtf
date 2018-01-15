#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrin.h>

#include "matrix.h"
#include "camera.h"
#include "r_main.h"

#include "GL\glew.h"
//#include "draw.h"

//tern renderer_t renderer;
//extern camera_array camera_a;

//camera_list_t camera_list;

static int camera_list_size;
static int camera_count;
static camera_t *camera_list;

static int active_camera_index;

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
void camera_Init()
{	
	camera_list_size = 64;
	camera_count = 0;
	camera_list = malloc(sizeof(camera_t ) * camera_list_size);	
	
	renderer_RegisterCallback(camera_UpdateCamerasCallback, RENDERER_RESOLUTION_CHANGE_CALLBACK);
	
	return;
}


/*
=============
camera_Finish
=============
*/
void camera_Finish()
{
	int i;
	
	for(i = 0; i < camera_count; i++)
	{
		free(camera_list[i].name);
	}
	free(camera_list);
	return;
}

int camera_CreateCamera(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar, int bm_flags)
{
	int camera_index = camera_count++;
	int name_len;
	camera_t *camera;
	
	if(camera_index >= camera_list_size)
	{
		camera = malloc(sizeof(camera_t) * (camera_list_size + 16));
		memcpy(camera, camera_list, sizeof(camera_t) * camera_list_size);
		free(camera_list);
		camera_list = camera;
		camera_list_size += 16;
	}
	
	camera = &camera_list[camera_index];

	name_len = strlen(name) + 1;
	name_len = (name_len + 3) & (~3);
	camera->name = malloc(name_len);
	strcpy(camera->name, name);
	
	
	camera->local_position = position;
	memcpy(&camera->local_orientation, orientation, sizeof(mat3_t));
	
	camera->world_position = position;
	memcpy(&camera->world_orientation, orientation, sizeof(mat3_t));
	
	CreatePerspectiveMatrix(&camera->projection_matrix, fovy, width/height, znear, zfar, 0.0, 0.0, &camera->frustum);
	
	camera->width = width;
	camera->height = height;
	camera->zoom = 1.0;
	camera->fov_y = fovy;
	camera->camera_index = camera_index;
	camera->bm_flags = bm_flags;
	//camera->assigned_node=scenegraph_AddNode(NODE_CAMERA, camera_index, -1, camera->name);
	camera_ComputeWorldToCameraMatrix(camera);
	
	return camera_index;
}

void camera_SetCameraProjectionMatrix(camera_t *camera, float width, float height, float znear, float zfar, float fovy)
{
	CreatePerspectiveMatrix(&camera->projection_matrix, fovy, width/height, znear, zfar, 0.0, 0.0, &camera->frustum);
	camera->width = width;
	camera->height = height;
}


/*
=============
camera_SetCameraByIndex
=============
*/
void camera_SetCameraByIndex(int camera_index)
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
}


void camera_SetCamera(camera_t *camera)
{
	active_camera_index = camera->camera_index;
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&camera->projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
}


/*
=============
camera_ComputeWorldToCameraMatrix
=============
*/
void camera_ComputeWorldToCameraMatrix(camera_t *camera)
{	
	camera->world_to_camera_matrix.floats[0][0] = camera->world_orientation.floats[0][0];
	camera->world_to_camera_matrix.floats[1][0] = camera->world_orientation.floats[0][1];
	camera->world_to_camera_matrix.floats[2][0] = camera->world_orientation.floats[0][2];
	
	camera->world_to_camera_matrix.floats[3][0] = (-camera->world_position.floats[0]) * camera->world_to_camera_matrix.floats[0][0]	+	
										  		  (-camera->world_position.floats[1]) * camera->world_to_camera_matrix.floats[1][0]	+
										  		  (-camera->world_position.floats[2]) * camera->world_to_camera_matrix.floats[2][0];
										  		  
										  		  
	
	camera->world_to_camera_matrix.floats[0][1] = camera->world_orientation.floats[1][0];
	camera->world_to_camera_matrix.floats[1][1] = camera->world_orientation.floats[1][1];
	camera->world_to_camera_matrix.floats[2][1] = camera->world_orientation.floats[1][2];
	
	camera->world_to_camera_matrix.floats[3][1] = (-camera->world_position.floats[0]) * camera->world_to_camera_matrix.floats[0][1]	+	
										  		  (-camera->world_position.floats[1]) * camera->world_to_camera_matrix.floats[1][1]	+
										  		  (-camera->world_position.floats[2]) * camera->world_to_camera_matrix.floats[2][1];
										  		  
										  		  
										  		
	
	camera->world_to_camera_matrix.floats[0][2] = camera->world_orientation.floats[2][0];
	camera->world_to_camera_matrix.floats[1][2] = camera->world_orientation.floats[2][1];
	camera->world_to_camera_matrix.floats[2][2] = camera->world_orientation.floats[2][2];	
						
	camera->world_to_camera_matrix.floats[3][2] = (-camera->world_position.floats[0]) * camera->world_to_camera_matrix.floats[0][2]	+	
										 		  (-camera->world_position.floats[1]) * camera->world_to_camera_matrix.floats[1][2]	+
										  		  (-camera->world_position.floats[2]) * camera->world_to_camera_matrix.floats[2][2];
										  		  
	
	camera->world_to_camera_matrix.floats[0][3] = 0.0;
	camera->world_to_camera_matrix.floats[1][3] = 0.0;
	camera->world_to_camera_matrix.floats[2][3] = 0.0;
	camera->world_to_camera_matrix.floats[3][3] = 1.0;									  		  
									
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
	return &camera_list[active_camera_index];
}

camera_t *camera_GetCamera(char *name)
{
	int i;
	int c = camera_count;
	
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, camera_list[i].name))
		{
			return &camera_list[i];
		}
	}
	
	return NULL;
	
}

camera_t *camera_GetCameraByIndex(int camera_index)
{
	if(camera_index > -1 && camera_index < camera_list_size)
	{
		return &camera_list[camera_index];
	}
	
	return NULL;
}


void camera_UpdateCamerasCallback()
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
}



#ifdef __cplusplus
}
#endif


