#ifndef R_MAIN_H
#define R_MAIN_H

#include "r_common.h"

#include "vector.h"
#include "matrix.h"
//#include "w_common.h"
//#include "portal.h"
//#include "shader.h"
#include "camera.h"
//#include <stdint.h>





#ifdef __cplusplus
extern "C"
{
#endif


int renderer_Init(int width, int height, int init_mode);

void renderer_Finish();

void renderer_BindDiffuseTexture(int texture_index);

void renderer_BindNormalTexture(int texture_index);

void renderer_BindClusterTexture();

void renderer_BindShadowTexture();

void renderer_BindShadowMaskTexture();

void renderer_BindTexture(int texture_unit, int texture_target, int texture_index);

void renderer_BindTextureTexUnit(int texture_unit, int texture_target, int texture);

//int renderer_BindTexture(int texture_target, int texture);

void renderer_SetClearColor(float r, float g, float b);

void renderer_SetUniformBuffers();

/*
====================================================================
====================================================================
====================================================================
*/

void renderer_SetProjectionMatrix(mat4_t *matrix);

void renderer_SetViewMatrix(mat4_t *matrix);

void renderer_SetModelMatrix(mat4_t *matrix);

void renderer_UpdateMatrices();


/*
====================================================================
====================================================================
====================================================================
*/

void renderer_SetMaterial(int material_index);

void renderer_SetMaterialColor(vec4_t color);

/*
====================================================================
====================================================================
====================================================================
*/

//void renderer_UpdateDrawCommandGroups(view_data_t *view_data);

//void renderer_SubmitDrawCommand(view_data_t *view_data, mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw);

void renderer_SubmitDrawCommandToView(struct view_data_t *view, mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw);

void renderer_SubmitDrawCommand(mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw);

void renderer_SortViewDrawCommands(struct view_data_t *view);

void renderer_SortDrawCommands();

/*
====================================================================
====================================================================
====================================================================
*/

void renderer_SetWindowSize(int width, int height);

void renderer_SetRendererResolution(int width, int height, int samples);


/*
====================================================================
====================================================================
====================================================================
*/

int renderer_GetFullscreen();

void renderer_Fullscreen(int enable);

void renderer_ToggleFullscreen();



void renderer_Multisample(int enable);

void renderer_ToggleMultisample();



//void renderer_DeferredRenderer(int enable);

void renderer_Fullbright(int enable);


void renderer_SetShadowMapResolution(int resolution);

int renderer_GetShadowMapResolution();


void renderer_SetFrameRateClamping(int clamping);

int renderer_GetFrameRateClamping();

//void renderer_SetRenderer(int renderer);


/*
====================================================================
====================================================================
====================================================================
*/

void renderer_RegisterCallback(void (*r_fn)(void), int type);

void renderer_ClearRegisteredCallbacks();

/*
====================================================================
====================================================================
====================================================================
*/

void renderer_ResizeWorldTrianglesUniformBuffer(int triangle_count);

void renderer_FillWorldVerticesUniformBuffer(int vertice_count, void *verts, int stride);

/*
====================================================================
====================================================================
====================================================================
*/
//void renderer_SetActiveView(view_def_t *view);

//void renderer_SetMainView(struct view_handle_t view);

//struct view_def_t *renderer_GetMainView();

//struct view_def_t *renderer_GetView(struct view_handle_t view);

//void renderer_ComputeViewMatrix(view_def_t *view);

//void renderer_ComputeActiveViewMatrix();

//void renderer_SetMainView(camera_t *view);

//void renderer_SetupView(camera_t *view);

//void renderer_SetViewData(view_data_t *view_data);

//void renderer_SetViewLightData(view_data_t *view_data);

//void renderer_SetViewDrawCommands(view_data_t *view_data);

//void renderer_SetViewVisibleWorld(view_data_t *view_data);

/*
====================================================================
====================================================================
====================================================================
*/

void renderer_StartFrame();

void renderer_DrawFrame();

void renderer_EndFrame();


/*
====================================================================
====================================================================
====================================================================
*/

void renderer_ZPrePass();

void renderer_DrawRaytracedShadowMaps();

void renderer_DrawShadowMaps();

void renderer_GenerateShadowMask();

void renderer_DrawWorld();

void renderer_Tonemap();

//void renderer_DrawWorld();

//void renderer_DrawOpaque();

//void renderer_DrawTranslucent();

//void renderer_DrawShadowMaps();

void renderer_DrawParticles();

void renderer_DrawGUI();

void renderer_DrawBloom();

void renderer_DrawSkyBox();

void renderer_BlitColorbuffer();

void renderer_BlitBackbuffer();

/*
==============================================================
==============================================================
==============================================================
*/


void renderer_Enable(int cap);

void renderer_Disable(int cap);

int renderer_IsEnabled(int cap);

void renderer_SetIntegerv(int param_name, int *param);

void renderer_GetIntegerv(int param_name, int *param);


/*

void renderer_BeginTimeElapsedQuery();

void renderer_EndTimeElapsedQuery(int stage_index);

void renderer_ReportQueryResults();

void renderer_StartGpuTimer();

float renderer_StopGpuTimer();*/




#ifdef __cplusplus
}
#endif


#endif
