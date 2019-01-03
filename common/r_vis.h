#ifndef R_VIS_H
#define R_VIS_H

#include "r_common.h"

void renderer_VisibleWorld();

void renderer_VisibleLightsBounds();

void renderer_VisibleLights();

void renderer_VisibleLightsOnClusters();

void renderer_UploadVisibleLights();

void renderer_VisibleEntities();





void renderer_WorldOnView(struct view_def_t *view_def);

void renderer_LightBoundsOnView(struct view_def_t *view_def);

void renderer_LightsOnView(struct view_def_t *view_def);

void renderer_LightsOnViewClusters(struct view_def_t *view_def);





float renderer_BoxScreenArea(struct view_def_t *view, vec3_t *center, vec3_t *extents);


#endif // R_VIS_H
