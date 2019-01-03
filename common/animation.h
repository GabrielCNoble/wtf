#ifndef ANIMATION_H
#define ANIMATION_H

#include "anim_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

int animation_Init();

void animation_Finish();

/*
==================================================================
==================================================================
==================================================================
*/

struct skeleton_handle_t animation_CreateEmptySkeleton(int def);

struct skeleton_handle_t animation_SpawnSkeleton(struct skeleton_handle_t def);

void animation_DestroySkeleton(struct skeleton_handle_t skeleton);

void animation_DestroyAllSkeletons();

struct skeleton_t *animation_GetSkeletonPointerHandle(struct skeleton_handle_t skeleton);

struct skeleton_handle_t animation_LoadSkeleton(char *file_name);

void animation_PlayAnimation(struct skeleton_handle_t skeleton, struct animation_handle_t animation);


/*
==================================================================
==================================================================
==================================================================
*/

struct animation_handle_t animation_CreateEmtpyAnimation();

void animation_DestroyAnimation(struct animation_handle_t animation);

void animation_DestroyAllAnimations();

struct animation_t *animation_GetAnimationPointerHandle(struct animation_handle_t animation);

struct animation_handle_t animation_LoadAnimation(char *file_name);

void animation_UpdateAnimations(float delta_time);


#ifdef __cplusplus
}
#endif


#endif







