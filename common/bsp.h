#ifndef BSP_H
#define BSP_H

#include "bsp_common.h"

#define LEAF_ON_PVS(x, y) (x->pvs[y>>3]&(1<<(y%8)))


#ifdef __cplusplus
extern "C"
{
#endif

int bsp_Init();

void bsp_Finish();

void bsp_LoadFile(char *file_name);

void bsp_DeleteBsp();

void bsp_Draw();

void bsp_ClipVelocityToPlane(vec3_t normal, vec3_t velocity, vec3_t *new_velocity, float overbounce);

bsp_pnode_t *bsp_HullForEntity(vec3_t mins, vec3_t max);

int bsp_SolidPoint(bsp_pnode_t *node, vec3_t point);

int bsp_FirstHit(bsp_pnode_t *bsp, vec3_t start, vec3_t end, trace_t *trace);

int bsp_TryStepUp(vec3_t *position, vec3_t *velocity, trace_t *trace);

int bsp_TryStepDown(vec3_t *position, vec3_t *velocity, trace_t *trace);

void bsp_Move(vec3_t *position, vec3_t *velocity);

bsp_dleaf_t *bsp_GetCurrentLeaf(bsp_pnode_t *node, vec3_t position);

bsp_dleaf_t **bsp_PotentiallyVisibleLeaves(int *leaf_count, vec3_t position);


#ifdef __cplusplus
extern "C"
{
#endif

#endif
