#ifndef R_MAIN_H
#define R_MAIN_H

#include "r_common.h"

#include "vector.h"
#include "matrix.h"
#include "w_common.h"
#include "portal.h"
#include "shader.h"
#include "camera.h"
#include <stdint.h>




int renderer_Init(int width, int height, int init_mode);

void renderer_Finish();

void renderer_SetDiffuseTexture(int texture_index);

void renderer_SetNormalTexture(int texture_index);

void renderer_SetClusterTexture();

void renderer_SetShadowTexture();

void renderer_SetTexture(int texture_unit, int texture_target, int texture_index);

void renderer_BindTextureTexUnit(int texture_unit, int texture_target, int texture);

int renderer_BindTexture(int texture_target, int texture);

void renderer_SetClearColor(float r, float g, float b);

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

/*
====================================================================
====================================================================
====================================================================
*/

//void renderer_UpdateDrawCommandGroups(view_data_t *view_data);

//void renderer_SubmitDrawCommand(view_data_t *view_data, mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw);

void renderer_SubmitDrawCommand(mat4_t *transform, unsigned short draw_mode, unsigned int start, unsigned int count, int material_index, int indexed_draw);

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

//void renderer_UpdateGeometryBuffer();

void renderer_UpdateColorbuffer();

void renderer_UpdatePortalbuffer();

//void renderer_BindGeometryBuffer(int clear, int read);

void renderer_BindColorbuffer(int clear, int read);

void renderer_BindBackbuffer(int clear, int read);

void renderer_BindWindowBuffer(int clear, int read);

/*
====================================================================
====================================================================
====================================================================
*/

void renderer_Fullscreen(int enable);

void renderer_ToggleFullscreen();

void renderer_Multisample(int enable);

void renderer_ToggleMultisample();

void renderer_DeferredRenderer(int enable);

void renderer_Fullbright(int enable);

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

void renderer_SetActiveView(camera_t *view);

void renderer_SetMainView(camera_t *view);

void renderer_SetupView(camera_t *view);

void renderer_SetViewData(view_data_t *view_data);

void renderer_SetViewLightData(view_data_t *view_data);

void renderer_SetViewDrawCommands(view_data_t *view_data);

void renderer_SetViewVisibleWorld(view_data_t *view_data);

/*
====================================================================
====================================================================
====================================================================
*/

void renderer_OpenFrame();

void renderer_DrawFrame();

void renderer_CloseFrame();


/*
====================================================================
====================================================================
====================================================================
*/


void renderer_ZPrePass();

void renderer_DrawWorld();

void renderer_ExecuteDrawCmds();

void renderer_DrawOpaque();

void renderer_DrawTranslucent();

void renderer_DrawShadowMaps();

void renderer_DrawParticles();

void renderer_DrawGUI();

void renderer_DrawBloom();

void renderer_DrawSkyBox();

//void renderer_DrawPlayers();

//void renderer_DrawActivePlayer();


/*
====================================================================
====================================================================
====================================================================
*/


void renderer_BlitColorbuffer();

void renderer_BlitBackbuffer();







void renderer_ExecuteDrawCommands();





//void renderer_DrawPortals();

//void renderer_RecursiveDrawPortals(portal_t *portal, int viewing_portal_index);

//void renderer_DrawPortal(portal_t *portal, int viewing_portal_index);

//void renderer_RecursiveDrawPortalsStencil(portal_t *portal, int viewing_portal_index);

//void renderer_RecursiveDrawPortalsViews(portal_t *portal, int viewing_portal_index);



void renderer_Tonemap();

//void renderer_Shade();







void renderer_BeginTimeElapsedQuery();

void renderer_EndTimeElapsedQuery(int stage_index);

void renderer_ReportQueryResults();

void renderer_StartGpuTimer();

float renderer_StopGpuTimer();







#endif
