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

struct bsp_pnode_t *bsp_HullForEntity(vec3_t mins, vec3_t max);

int bsp_SolidPoint(struct bsp_pnode_t *node, vec3_t point);

int bsp_FirstHit(struct bsp_pnode_t *bsp, vec3_t start, vec3_t end, struct trace_t *trace);

int bsp_TryStepUp(vec3_t *position, vec3_t *velocity, struct trace_t *trace);

int bsp_TryStepDown(vec3_t *position, vec3_t *velocity, struct trace_t *trace);

void bsp_Move(vec3_t *position, vec3_t *velocity);

struct bsp_dleaf_t *bsp_GetCurrentLeaf(struct bsp_pnode_t *node, vec3_t position);

unsigned char *bsp_CompressPvs(unsigned char *uncompressed_pvs, int uncompressed_pvs_size, int *compressed_pvs_size);

struct bsp_dleaf_t **bsp_DecompressPvs(struct bsp_dleaf_t *leaf, int *leaves_count);

struct bsp_dleaf_t **bsp_PotentiallyVisibleLeaves(int *leaf_count, vec3_t position);

struct bsp_dleaf_t **bsp_FrontToBackWalk(int *leaf_count, vec3_t position);




void bsp_SerializeBsp(void **buffer, int *buffer_size);

void bsp_DeserializeBsp(void **buffer);

void bsp_SaveBsp(char *output_name);

void *bsp_LoadBsp(char *file_name);


#ifdef __cplusplus
extern "C"
{
#endif

#endif
