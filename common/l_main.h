#ifndef L_MAIN_H
#define L_MAIN_H

#include "matrix_types.h"
#include "vector_types.h"

#include "l_common.h"
//#include "bsp_common.h"
//#include "camera_types.h"





/*
===================================================================
===================================================================
===================================================================
*/



int light_Init();

void light_Finish();

int light_CreateLight(char *name, mat3_t *orientation, vec3_t position, vec3_t color, float radius, float energy, int bm_flags);

int light_DestroyLight(char *name);

int light_DestroyLightIndex(int light_index);

void light_DestroyAllLights();

int light_Getlight(char *name);

light_ptr_t light_GetLightPointer(char *name);

light_ptr_t light_GetLightPointerIndex(int light_index);



/* updates which leaves contains which lights... */
//void light_MarkLightsOnLeaves();

/* find out which clusters each visible light affect
(and eliminate lights outside the frustum)... */
//void light_LightBounds();

/* update the cluster texture (very time consuming!)... */
//void light_UpdateClusters();

/* create a list of visible lights from the visible
leaves... */
//void light_VisibleLights();

/* update the index list each light keeps to
render its shadow map... */
//void light_VisibleTriangles(int light_index);


void light_ClearLightLeaves();

void light_EntitiesOnLights();

void light_TranslateLight(int light_index, vec3_t direction, float amount);

//void light_SetLight(int light_index);

void light_AllocShadowMap(int light_index);

void light_FreeShadowMap(int light_index);

void light_AllocShadowMaps();


void light_SerializeLights(void **buffer, int *buffer_size);

void light_DeserializeLights(void **buffer);



#endif








