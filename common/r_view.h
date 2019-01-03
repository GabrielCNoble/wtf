#ifndef R_VIEW_H
#define R_VIEW_H


#include "r_common.h"
#include "matrix.h"
#include "vector.h"



struct view_handle_t renderer_CreateViewDef(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar, int flags);

void renderer_CreateViewData(struct view_data_t *view_data);

void renderer_DestroyViewDef(struct view_handle_t view);

void renderer_SetMainView(struct view_handle_t view);



struct view_def_t *renderer_GetMainViewPointer();

struct view_handle_t renderer_GetMainView();

struct view_def_t *renderer_GetViewPointer(struct view_handle_t view);

struct view_handle_t renderer_GetViewByName(char *view_name);

struct view_def_t *renderer_GetViewPointerByName(char *view_name);




void renderer_PitchYawView(struct view_handle_t view, float yaw, float pitch);

void renderer_TranslateView(struct view_handle_t view, vec3_t direction, float amount, int set);


void renderer_ComputeViewMatrix(struct view_handle_t view);

void renderer_ComputeMainViewMatrix();


#endif // R_VIEW_H
