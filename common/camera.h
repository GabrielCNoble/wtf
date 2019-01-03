#ifndef CAMERA_H
#define CAMERA_H
#include "camera_types.h"

#include "matrix.h"
#include "vector.h"
#include "frustum.h"
//#include "scenegraph.h"

#define CAMERA_MAX_NAME_LEN 32		/* including trailing null... */

#ifdef __cplusplus
extern "C"
{
#endif


//int camera_Init();

//void camera_Finish();

//void camera_ResizeCameraArray(int new_size);

//camera_t *camera_CreateCamera(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar, int bm_flags);

//void camera_InitViewData(struct view_data_t *view_data);

//void camera_DestroyCamera(camera_t *camera);

//void camera_DestroyAllCameras();

//void camera_SetCameraByIndex(int camera_index);

//void camera_SetCamera(camera_t *camera);

//void camera_SetMainViewCamera(camera_t *camera);

//void camera_Activate(camera_t *camera);

//void camera_Deactivate(camera_t *camera);

//void camera_ComputeWorldToCameraMatrix(camera_t *camera);

//void camera_TranslateCamera(camera_t *camera, vec3_t direction, float amount, int b_set);

//void camera_TranslateActiveCamera(vec3_t direction, float amount, int b_set);

//void camera_RotateCamera(camera_t *camera, vec3_t axis, float angle, int b_set);

//void camera_PitchYawCamera(camera_t *camera, float yaw, float pitch);

//camera_t *camera_GetActiveCamera();

//camera_t *camera_GetMainViewCamera();

//camera_t *camera_GetCamera(char *name);

//camera_t *camera_GetCameraByIndex(int camera_index);



//float camera_BoxScreenArea(camera_t *camera, vec3_t center, vec3_t extents);



//void camera_UpdateCamerasCallback();

#ifdef __cplusplus
}
#endif


#endif /* CAMERA_H */
