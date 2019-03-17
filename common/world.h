#ifndef WORLD_H
#define WORLD_H

#include "w_common.h"
#include "bsp_common.h"
#include "scr_common.h"
#include "camera.h"
#include "portal.h"
#include "serializer.h"


#ifdef __cplusplus
extern "C"
{
#endif

int world_Init();

void world_Finish();

int world_LoadBsp(char *file_name);

/*
=================================================
=================================================
=================================================
*/

void world_MarkEntitiesOnLeaves();

void world_MarkLightsOnLeaves();

/*
=================================================
=================================================
=================================================
*/

//void world_WorldOnView(view_data_t *view_data);

//void world_WorldOnPortalView(portal_view_data_t *view_data);

//void world_WorldOnViews();

//void world_LightBounds();

//void world_LightBoundsOnView(view_data_t *view_data);

//void world_LightsOnView(view_data_t *view_data);

//void world_LightsOnViews();


//void world_EntitiesOnViews();


//void world_PortalOnView(view_data_t *view_data, portal_t *portal, vec3_t position);

//void world_PortalOnPortalView(portal_view_data_t *view_data, portal_t *portal);

//void world_PortalsOnViews();



/*
=================================================
=================================================
=================================================
*/

//void world_PortalsOnPortals(portal_t *portal, mat3_t *view_orientation, vec3_t view_position, int viewing_portal_index);

//void world_WorldOnPortals();

//void world_LightsOnPortal(portal_t *portal);

//void world_EntitiesOnPortals();

/*
=================================================
=================================================
=================================================
*/

//void world_VisibleEntities();

//void world_VisibleLights();

//void world_VisibleLightTriangles(int light_index);

//void world_VisibleLeaves();

//void world_VisibleWorld();








struct world_var_t *world_AddWorldVar(char *name, int size);

struct world_var_t *world_AddWorldArrayVar(char *name, int elem_size, int max_elements);

void world_RemoveWorldVar(char *name);



struct world_var_t *world_GetWorldVarPointer(char *name);



void world_WorldVarValue(char *name, void *value, int set);

void world_WorldArrayVarValue(char *name, void *value, int index, int set);



void world_SetWorldVarValue(char *name, void *value);

void world_GetWorldVarValue(char *name, void *value);

void world_SetWorldArrayVarValue(char *name, void *value, int index);

void world_GetWorldArrayVarValue(char *name, void *value, int index);

void world_AppendWorldArrayVarValue(char *name, void *value);

void world_ClearWorldArrayVar(char *name);




struct world_script_t *world_LoadScript(char *script_name);

void world_SetWorldScript(struct world_script_t *world_script);

struct world_script_t *world_GetWorldScript();

void world_ExecuteWorldScript();

struct world_event_t *world_GetEventPointer(char *event_name);

void world_CallEvent(char *event_name);

void world_CallEventIndex(int event_index);

void world_StopEvent(char *event_name);

void world_StopEventIndex(int event_index);

void world_StopAllEvents();

void world_FadeIn();

int world_HasFadedIn();

void world_FadeOut();

int world_HasFadedOut();

void world_ClearBsp();

void world_Update();

void world_Clear(int clear_flags);

struct world_level_t *world_CreateLevel(char *level_name, struct serializer_t serializer, struct world_script_t *world_script);

void world_DestroyLevel(char *level_name);

struct world_level_t *world_GetLevel(char *level_name);

struct world_level_t *world_GetCurrentLevel();

void world_ChangeLevel(char *level_name);

struct world_level_t *world_LoadLevel(char *level_name);

struct world_level_t *world_LoadLevelFromMemory(char *level_name, void **buffer);

void world_UnloadCurrentLevel();

void world_SerializeWorld(void **buffer, int *buffer_size);

struct serializer_t world_DeserializeWorld(void **buffer);


#ifdef __cplusplus
}
#endif




#endif
