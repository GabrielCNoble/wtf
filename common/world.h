#ifndef WORLD_H
#define WORLD_H

#include "w_common.h"
#include "bsp_common.h"
#include "camera.h"
#include "portal.h"


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

void world_VisibleEntities();

void world_VisibleLights();

void world_VisibleLightTriangles();

void world_VisibleLeaves();

void world_VisibleWorld();



void world_AddLeafIndexes(int leaf_index);

void world_RemoveLeafIndexes(int leaf_index);

void world_Update();

void world_Clear();


#ifdef __cplusplus
}
#endif




#endif
