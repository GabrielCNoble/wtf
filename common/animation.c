#include "animation.h"

#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/skeleton_utils.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/blending_job.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/base/io/stream.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/memory/allocator.h"
#include "ozz/base/platform.h"
#include "ozz/base/maths/soa_transform.h"

#include "containers/stack_list.h"
#include "containers/list.h"

#include "c_memory.h"



struct stack_list_t anim_skeletons[2];
struct list_t anim_playing_skeletons;
struct stack_list_t anim_animations;

#ifdef __cplusplus
extern "C"
{
#endif

int animation_Init()
{
    anim_skeletons[0] = stack_list_create(sizeof(struct skeleton_t), 32, NULL);
    anim_skeletons[1] = stack_list_create(sizeof(struct skeleton_t), 32, NULL);

    anim_animations = stack_list_create(sizeof(struct animation_t), 32, NULL);

    anim_playing_skeletons = list_create(sizeof(struct skeleton_handle_t), 32, NULL);

    return 1;
}

void animation_Finish()
{
    animation_DestroyAllSkeletons();
    animation_DestroyAllAnimations();

    stack_list_destroy(&anim_skeletons[0]);
    stack_list_destroy(&anim_skeletons[1]);
    stack_list_destroy(&anim_animations);

    list_destroy(&anim_playing_skeletons);
}


/*
==================================================================
==================================================================
==================================================================
*/

struct skeleton_handle_t animation_CreateEmptySkeleton(int def)
{
    struct skeleton_handle_t handle;
    struct skeleton_t *skeleton;

    def = def && 1;

    handle.index = stack_list_add(&anim_skeletons[def], NULL);
    handle.def = def;
    skeleton = (struct skeleton_t *)stack_list_get(&anim_skeletons[def], handle.index);

    skeleton->flags = 0;
    skeleton->ozz_skeleton = NULL;

    return handle;
}

struct skeleton_handle_t animation_SpawnSkeleton(struct skeleton_handle_t def)
{
    struct skeleton_handle_t handle;
    struct skeleton_t *skeleton;
    struct skeleton_t *skeleton_def;

    handle = INVALID_SKELETON_HANDLE;

    if(def.def)
    {
        skeleton_def = animation_GetSkeletonPointerHandle(def);

        if(skeleton_def)
        {
            handle = animation_CreateEmptySkeleton(0);

            skeleton = animation_GetSkeletonPointerHandle(handle);

            skeleton->joints = skeleton_def->joints;
            skeleton->joint_count = skeleton_def->joint_count;
            skeleton->ozz_skeleton = skeleton_def->ozz_skeleton;
            skeleton->ozz_local_buffer = skeleton_def->ozz_local_buffer;
            skeleton->ozz_model_buffer = skeleton_def->ozz_model_buffer;
            skeleton->ozz_sampling_cache = skeleton_def->ozz_sampling_cache;
        }
    }

    return handle;
}

void animation_DestroySkeleton(struct skeleton_handle_t skeleton)
{
    struct skeleton_t *skeleton_ptr;


    skeleton_ptr = animation_GetSkeletonPointerHandle(skeleton);

    if(skeleton_ptr)
    {
        skeleton_ptr->flags |= ANIM_SKELETON_FLAG_INVALID;

        if(skeleton_ptr->ozz_skeleton)
        {
            ozz::memory::default_allocator()->Delete((ozz::animation::Skeleton *)skeleton_ptr->ozz_skeleton);
            ozz::memory::default_allocator()->Delete((ozz::animation::SamplingCache *)skeleton_ptr->ozz_sampling_cache);

            memory_Free(skeleton_ptr->ozz_local_buffer);
            memory_Free(skeleton_ptr->ozz_model_buffer);
        }

        stack_list_remove(&anim_skeletons[skeleton.def], skeleton.index);
    }
}

void animation_DestroyAllSkeletons()
{
    int i;

    for(i = 0; i < anim_skeletons[1].element_count; i++)
    {
        animation_DestroySkeleton(SKELETON_HANDLE(i, 1));
    }
}


struct skeleton_t *animation_GetSkeletonPointerHandle(struct skeleton_handle_t skeleton)
{
    struct skeleton_t *skeleton_ptr;

    skeleton_ptr = (struct skeleton_t *)stack_list_get(&anim_skeletons[skeleton.def], skeleton.index);

    if(skeleton_ptr)
    {
        if(skeleton_ptr->flags & ANIM_SKELETON_FLAG_INVALID)
        {
            skeleton_ptr = NULL;
        }
    }

    return skeleton_ptr;
}


void test_joint_hierarchy(struct skeleton_joint_t *joint)
{
    static int level = -1;
    int i;

    level++;

    for(i = 0; i < level; i++)
    {
        printf("-");
    }

    printf("%s\n", joint->name);

    for(i = 0; i < joint->child_count; i++)
    {
        test_joint_hierarchy(joint->children[i]);
    }


    level--;
}

void animation_BuildJointHierachy(struct skeleton_handle_t skeleton)
{
    struct skeleton_t *skeleton_ptr;
    ozz::animation::Skeleton *ozz_skeleton;
    ozz::animation::Skeleton::JointProperties *ozz_joints;
    struct skeleton_joint_t *joints;
    struct skeleton_joint_t *parent_joint;
    struct skeleton_joint_t **joint_children;

    int i;
    int j;


    skeleton_ptr = animation_GetSkeletonPointerHandle(skeleton);

    if(skeleton_ptr)
    {
        ozz_skeleton = (ozz::animation::Skeleton *)skeleton_ptr->ozz_skeleton;

        if(ozz_skeleton)
        {
            j = ozz_skeleton->num_joints();

            joints = (struct skeleton_joint_t *)memory_Calloc(j, sizeof(struct skeleton_joint_t));
            joint_children = (struct skeleton_joint_t **)memory_Calloc(j, sizeof(struct skeleton_joint_t *));
            ozz_joints = ozz_skeleton->joint_properties_.begin;

            joints[0].children = joint_children;
            joints[0].parent = NULL;
            joints[0].name = ozz_skeleton->joint_names_.begin[0];

            for(i = 1; i < j; i++)
            {
                parent_joint = joints + ozz_joints[i].parent;
                parent_joint->child_count++;

                joints[i].parent = parent_joint;
                joints[i].child_count = 0;
                joints[i].name = ozz_skeleton->joint_names_.begin[i];
            }

            for(i = 1; i < j; i++)
            {
                joints[i].children = joints[i - 1].children + joints[i - 1].child_count;
                joints[i - 1].child_count = 0;
            }

            joints[i - 1].child_count = 0;

            for(j--; j > 0; j--)
            {
                parent_joint = joints[j].parent;
                parent_joint->children[parent_joint->child_count] = joints + j;
                parent_joint->child_count++;
            }

            skeleton_ptr->joints = joints;
            skeleton_ptr->joint_count = ozz_skeleton->num_joints();

            //test_joint_hierarchy(skeleton_ptr->joints);
        }
    }
}

void animation_RecursiveFillPose(struct skeleton_joint_t *joint, ozz::math::Float4x4 **pose)
{
    int j;

    ozz::math::Float4x4 *transform = *pose;

    for(j = 0; j < 4; j++)
    {
        joint->transform.floats[j][0] = transform->cols[j].x;
        joint->transform.floats[j][1] = transform->cols[j].y;
        joint->transform.floats[j][2] = transform->cols[j].z;
        joint->transform.floats[j][3] = transform->cols[j].w;
    }

    (*pose)++;

    for(j = 0; j < joint->child_count; j++)
    {
        animation_RecursiveFillPose(joint->children[j], pose);
    }
}

void animation_FillPose(struct skeleton_handle_t skeleton, ozz::math::Float4x4 *pose)
{
    struct skeleton_t *skeleton_ptr;
    int i;

    skeleton_ptr = animation_GetSkeletonPointerHandle(skeleton);

    if(skeleton_ptr && pose)
    {
        animation_RecursiveFillPose(skeleton_ptr->joints, &pose);
    }
}


struct skeleton_handle_t animation_LoadSkeleton(char *file_name)
{
    struct skeleton_t *skeleton;
    struct skeleton_handle_t handle;
    ozz::io::File file(file_name, "rb");
    ozz::io::IArchive archive(&file);
    ozz::animation::Skeleton *ozz_skeleton;
    //ozz::animation::SamplingJob *sampling_job;
    //ozz::math::SoaTransform *ttimeransforms;
    ozz::math::Float4x4 *transforms;
    ozz::animation::LocalToModelJob job;
    ozz::math::Float4x4 transform;

    unsigned int i;
    unsigned int j;
    unsigned int c;

    ozz_skeleton = ozz::memory::default_allocator()->New<ozz::animation::Skeleton>();

    archive >> *ozz_skeleton;

    handle = animation_CreateEmptySkeleton(1);
    skeleton = animation_GetSkeletonPointerHandle(handle);

    skeleton->ozz_skeleton = ozz_skeleton;

    animation_BuildJointHierachy(handle);

    transforms = (ozz::math::Float4x4 *) memory_Malloc(sizeof(ozz::math::Float4x4) * skeleton->joint_count);



    c = ozz_skeleton->num_soa_joints();

    job.skeleton = ozz_skeleton;
    job.output = ozz::Range<ozz::math::Float4x4>(transforms, skeleton->joint_count);
    job.input = ozz_skeleton->bind_pose_;

    job.Run();

    c = job.output.count();
    transforms = job.output.begin;

    animation_FillPose(handle, transforms);

    skeleton->ozz_local_buffer = memory_Calloc(sizeof(ozz::math::SoaTransform), ozz_skeleton->num_soa_joints() * 2);
    skeleton->ozz_model_buffer = memory_Calloc(sizeof(ozz::math::Float4x4), ozz_skeleton->num_joints());
    skeleton->ozz_sampling_cache = ozz::memory::default_allocator()->New<ozz::animation::SamplingCache>(skeleton->joint_count * 2);


    return handle;
}

void animation_PlayAnimation(struct skeleton_handle_t skeleton, struct animation_handle_t animation)
{
    struct skeleton_t *skeleton_ptr;
    struct animation_t *animation_ptr;
    //ozz::animation::SamplingJob *sampler;


    skeleton_ptr = animation_GetSkeletonPointerHandle(skeleton);
    animation_ptr = animation_GetAnimationPointerHandle(animation);

    if(skeleton_ptr && animation_ptr)
    {
        skeleton_ptr->animation = animation;
        skeleton_ptr->flags |= ANIM_SKELETON_FLAG_PLAYING;

        //sampler = (ozz::animation::SamplingJob *)skeleton_ptr->ozz_sampler;

        //sampler->animation = (ozz::animation::Animation) = animation_ptr->ozz_animation;
    }
}

/*
==================================================================
==================================================================
==================================================================
*/


struct animation_handle_t animation_CreateEmtpyAnimation()
{
    struct animation_handle_t handle;
    struct animation_t *animation;


    handle.index = stack_list_add(&anim_animations, NULL);
    animation = (struct animation_t *)stack_list_get(&anim_animations, handle.index);

    animation->flags = 0;
    animation->ozz_animation = NULL;

    return handle;
}

void animation_DestroyAnimation(struct animation_handle_t animation)
{
    struct animation_t *animation_ptr;

    animation_ptr = animation_GetAnimationPointerHandle(animation);

    if(animation_ptr)
    {
        if(!(animation_ptr->flags & ANIM_ANIMATION_FLAG_INVALID))
        {
            if(animation_ptr->ozz_animation)
            {
                ozz::memory::default_allocator()->Delete((ozz::animation::Animation *)animation_ptr->ozz_animation);
            }

            animation_ptr->flags |= ANIM_ANIMATION_FLAG_INVALID;

            stack_list_remove(&anim_animations, animation.index);
        }
    }
}

void animation_DestroyAllAnimations()
{
    int i;

    for(i = 0; i < anim_animations.element_count; i++)
    {
        animation_DestroyAnimation(ANIMATION_HANDLE(i));
    }
}

struct animation_t *animation_GetAnimationPointerHandle(struct animation_handle_t animation)
{
    struct animation_t *animation_ptr;

    animation_ptr = (struct animation_t *)stack_list_get(&anim_animations, animation.index);

    if(animation_ptr)
    {
        if(animation_ptr->flags & ANIM_ANIMATION_FLAG_INVALID)
        {
            animation_ptr = NULL;
        }
    }

    return animation_ptr;
}

struct animation_handle_t animation_LoadAnimation(char *file_name)
{
    struct animation_handle_t handle;
    struct animation_t *animation;
    ozz::animation::Animation *ozz_animation;

    int i;

    ozz::io::File file(file_name, "rb");
    ozz::io::IArchive archive(&file);


    ozz_animation = ozz::memory::default_allocator()->New<ozz::animation::Animation>();

    archive >> *ozz_animation;


    handle = animation_CreateEmtpyAnimation();
    animation = animation_GetAnimationPointerHandle(handle);

    animation->ozz_animation = (void *)ozz_animation;


    //printf("%d\n", ozz_animation->num_tracks());
    //printf("%d\n", ozz_animation->translations_.count());

    return handle;
}

void animation_UpdateAnimations(float delta_time)
{
    struct skeleton_t *skeletons;
    struct skeleton_t *skeleton;
    ozz::animation::Skeleton *ozz_skeleton;
    struct animation_t *animation;

    static float time = 0.0;
    int i;

    ozz::animation::LocalToModelJob transformer;
    ozz::animation::SamplingJob sampler;

    skeletons = (struct skeleton_t *)anim_skeletons[0].elements;

    for(i = 0; i < anim_skeletons[0].element_count; i++)
    {
        if(skeletons[i].flags & ANIM_SKELETON_FLAG_PLAYING)
        {
            skeleton = &skeletons[i];
            animation = animation_GetAnimationPointerHandle(skeleton->animation);

            ozz_skeleton = (ozz::animation::Skeleton *)skeleton->ozz_skeleton;

            sampler.animation = (ozz::animation::Animation *)animation->ozz_animation;
            sampler.ratio = time;
            sampler.cache = (ozz::animation::SamplingCache *)skeleton->ozz_sampling_cache;
            sampler.output = ozz::Range<ozz::math::SoaTransform>((ozz::math::SoaTransform *)skeleton->ozz_local_buffer, ozz_skeleton->num_soa_joints() * 2);

            time += 0.001;

            if(time >= 1.0)
            {
                time = 0.0;
            }

            if(sampler.Run())
            {
                transformer.input = sampler.output;
                //transformer.input = ozz_skeleton->bind_pose_;
                transformer.output = ozz::Range<ozz::math::Float4x4>((ozz::math::Float4x4 *)skeleton->ozz_model_buffer, ozz_skeleton->num_joints());
                transformer.skeleton = (ozz::animation::Skeleton*)skeleton->ozz_skeleton;

                if(transformer.Run())
                {
                    animation_FillPose(SKELETON_HANDLE(i, 0), transformer.output.begin);
                }
                else
                {
                    printf("animation_UpdateAnimations: transformation job is not valid!\n");
                }
            }
            else
            {
                printf("animation_UpdateAnimations: sampling job not valid!\n");
            }
        }
    }
}


#ifdef __cplusplus
}
#endif
