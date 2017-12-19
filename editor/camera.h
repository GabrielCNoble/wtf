#ifndef CAMERA_H
#define CAMERA_H
#include "camera_types.h"

#include "matrix.h"
#include "vector.h"
#include "frustum.h"
//#include "scenegraph.h"



void camera_Init();

void camera_Finish();

void camera_ResizeCameraArray(int new_size);

int camera_CreateCamera(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar);

void camera_SetCameraByIndex(int camera_index);

void camera_SetCamera(camera_t *camera);

void camera_ComputeWorldToCameraMatrix(camera_t *camera);

void camera_TranslateCamera(camera_t *camera, vec3_t direction, float amount, int b_set);

void camera_TranslateActiveCamera(vec3_t direction, float amount, int b_set);

void camera_RotateCamera(camera_t *camera, vec3_t axis, float angle, int b_set);

void camera_PitchYawCamera(camera_t *camera, float yaw, float pitch);

camera_t *camera_GetActiveCamera();

camera_t *camera_GetCamera(char *name);

camera_t *camera_GetCameraByIndex(int camera_index);



#endif /* CAMERA_H */
