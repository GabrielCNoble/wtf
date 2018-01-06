#ifndef BSP_H
#define BSP_H

#include "bsp_common.h"


void bsp_Init();

void bsp_Finish();

void bsp_LoadFile(char *file_name);

//void bsp_DeleteSolid(bsp_node_t *root);

//void bsp_DeleteSolidLeaf(bsp_node_t *root);

void bsp_DeleteBsp();

//void bsp_DrawBsp(bsp_node_t *root, bsp_node_t *parent, vec3_t camera_position, int level);

//void bsp_DrawSolidLeaf(bsp_node_t *root, vec3_t *camera_position, int level);

void bsp_Draw();

//int bsp_LineOfSight(bsp_node_t *root, vec3_t *start, vec3_t *end, vec3_t *normal, vec3_t *intersection, vec3_t *position, float *closest);

void bsp_ClipVelocityToPlane(vec3_t normal, vec3_t velocity, vec3_t *new_velocity, float overbounce);

bsp_pnode_t *bsp_HullForEntity(vec3_t mins, vec3_t max);

int bsp_SolidPoint(bsp_pnode_t *node, vec3_t point);

int bsp_FirstHit(bsp_pnode_t *bsp, vec3_t start, vec3_t end, trace_t *trace);

int bsp_TryStepUp(vec3_t *position, vec3_t *velocity, trace_t *trace);

void bsp_Move(vec3_t *position, vec3_t *velocity);

bsp_dleaf_t *bsp_GetCurrentLeaf(bsp_pnode_t *node, vec3_t camera_position);

bsp_dleaf_t **bsp_PotentiallyVisibleLeaves(int *leaf_count, vec3_t camera_position);



#endif
