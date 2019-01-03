#ifndef ANIM_COMMON_H
#define ANIM_COMMON_H



#include "vector.h"
#include "matrix.h"


enum ANIM_SKELETON_FLAGS
{
    ANIM_SKELETON_FLAG_INVALID = 1,
    ANIM_SKELETON_FLAG_PLAYING = 1 << 1,
};

enum ANIM_ANIMATION_FLAGS
{
    ANIM_ANIMATION_FLAG_INVALID = 1,
};


struct skeleton_handle_t
{
    //unsigned int index;
    unsigned index : 31;
    unsigned def : 1;
};

#define INVALID_SKELETON_INDEX 0x7fffffff
#define INVALID_SKELETON_HANDLE (struct skeleton_handle_t){INVALID_SKELETON_INDEX, 1}
#define SKELETON_HANDLE(index, def) (struct skeleton_handle_t){index, def}



struct animation_handle_t
{
    unsigned int index;
};

#define INVALID_ANIMATION_INDEX 0xffffffff
#define INVALID_ANIMATION_HANDLE (struct animation_handle_t){INVALID_ANIMATION_INDEX}
#define ANIMATION_HANDLE(index) (struct animation_handle_t){index}


struct skeleton_joint_t
{
    mat4_t transform;
    int child_count;
    struct skeleton_joint_t **children;
    struct skeleton_joint_t *parent;

    char *name;
};

struct animation_t
{
    char *name;
    void *ozz_animation;
    int flags;
};

struct animation_track_t
{
    float time_scale;
};

struct skeleton_t
{
    void *ozz_skeleton;
    void *ozz_sampling_cache;
    void *ozz_local_buffer;
    void *ozz_model_buffer;

    unsigned int flags;
    int joint_count;
    struct skeleton_joint_t *joints;

    struct animation_handle_t animation;
    //struct skeleton_joint_t root;
};




#endif // ANIM_COMMON_H
