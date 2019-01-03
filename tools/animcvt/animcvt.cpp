#include "animcvt.h"

using namespace ozz::animation;

struct node_anim_node_t
{
    struct node_anim_node_t *parent;
    int children_count;
    struct node_anim_node_t **children;

    int node_index;

    aiNode *node;
    aiNodeAnim *node_anim;
};



int count_nodes(aiNode *node)
{
    unsigned int i;
    int count = 1;

    for(i = 0; i < node->mNumChildren; i++)
    {
        count += count_nodes(node->mChildren[i]);
    }

    return count;
}



void get_nodes_by_prop(aiNode *node, aiNode **buffer, int *cursor, char *prop)
{
    unsigned int i;

    aiMetadata *meta_data;

    meta_data = node->mMetaData;

    if(meta_data)
    {
        for(i = 0; i < meta_data->mNumProperties; i++)
        {
            if(!strcmp(meta_data->mKeys[i].C_Str(), prop))
            {
                buffer[*cursor] = node;
                (*cursor)++;
            }
        }
    }

    for(i = 0; i < node->mNumChildren; i++)
    {
        get_nodes_by_prop(node->mChildren[i], buffer, cursor, prop);
    }
}

int node_index = 0;

struct node_anim_node_t *build_node_tree_recursive(aiNode *node)
{
    aiMetadata *meta_data;

    struct node_anim_node_t *child_node;
    struct node_anim_node_t *current_node;

    unsigned int i;
    unsigned int j;

    current_node = (struct node_anim_node_t *)calloc(sizeof(struct node_anim_node_t), 1);

    current_node->node = node;

    if(node->mNumChildren)
    {
        current_node->children = (struct node_anim_node_t **)calloc(sizeof(struct node_anim_node_t *), node->mNumChildren);
        current_node->node_index = node_index;
        node_index++;

        for(i = 0; i < node->mNumChildren; i++)
        {
            meta_data = node->mChildren[i]->mMetaData;

            for(j = 0; j < meta_data->mNumProperties; j++)
            {
                if(!strcmp(meta_data->mKeys[j].C_Str(), "bone"))
                {
                    child_node = build_node_tree_recursive(node->mChildren[i]);
                    child_node->parent = current_node;
                    current_node->children[current_node->children_count] = child_node;
                    current_node->children_count++;
                    break;
                }
            }
        }
    }

    return current_node;
}



struct node_anim_node_t *build_node_tree(const aiScene *scene)
{
    struct node_anim_node_t *tree = NULL;
    aiNode **skeletons = NULL;
    int skeleton_count = 0;

    node_index = -1;

    skeleton_count = count_nodes(scene->mRootNode);

    skeletons = (aiNode **)calloc(sizeof(aiNode *), skeleton_count);

    skeleton_count = 0;

    get_nodes_by_prop(scene->mRootNode, skeletons, &skeleton_count, "skeleton");


    if(skeleton_count)
    {
        tree = build_node_tree_recursive(skeletons[0]);
    }

    free(skeletons);

    return tree;
}





void fill_node_tree_node_anim(aiAnimation *anim, struct node_anim_node_t *node_tree)
{
    int i;
    unsigned int j;

    for(j = 0; j < anim->mNumChannels; j++)
    {
        if(!strcmp(anim->mChannels[j]->mNodeName.C_Str(), node_tree->node->mName.C_Str()))
        {
            node_tree->node_anim = anim->mChannels[j];
            break;
        }
    }

    for(i = 0; i < node_tree->children_count; i++)
    {
        fill_node_tree_node_anim(anim, node_tree->children[i]);
    }
}

char *fix_stupid_name(char *stupid_name)
{
    static char fixed_name[512];
    int i;

    for(i = 0; stupid_name[i]; i++)
    {
        if(stupid_name[i] == '|')
        {
            /* make sure this stupid vertical pipe doesn't get
            added to the output name. Whose idea was to fiddle
            with the animation names? It's the job of the DCC
            tool to ensure unique names, not the importer. It
            could at least have a flag to allow switching this
            annoying behavior off... */

            i++;
            break;
        }
    }

    strcpy(fixed_name, stupid_name + i);
    return fixed_name;
}

void traverse_nodes2(aiNode *node)
{
    unsigned int i;
    unsigned int j;
    static unsigned int level = -1;

    level++;

    aiMetadata *meta_data;
    char *meta_data_type;

    char value_buffer[512];

    char bool_value;
    int32_t int_value;
    int64_t long_value;
    float float_value;
    aiString *string_value;
    aiVector3D vector_value;

    if(node)
    {
        for(i = 0; i < level; i++)
        {
            printf("-");
        }
        printf("%s\n", node->mName.C_Str());

        if(node->mMetaData)
        {
            meta_data = node->mMetaData;

            for(j = 0; j < meta_data->mNumProperties; j++)
            {
                for(i = 0; i < level; i++)
                {
                    printf(" ");
                }
                /*
                AI_BOOL       = 0,
                AI_INT32      = 1,
                AI_UINT64     = 2,
                AI_FLOAT      = 3,
                AI_DOUBLE     = 4,
                AI_AISTRING   = 5,
                AI_AIVECTOR3D = 6,
                AI_META_MAX   = 7,
                */

                switch(meta_data->mValues[j].mType)
                {
                    case AI_BOOL:
                        meta_data_type = "AI_BOOL";
                        bool_value = *(char *)meta_data->mValues[j].mData;
                        sprintf(value_buffer, "%s", bool_value ? "true" : "false");
                    break;

                    case AI_INT32:
                        meta_data_type = "AI_INT32";
                        int_value = *(int32_t *)meta_data->mValues[j].mData;
                        sprintf(value_buffer, "%d", int_value);
                    break;

                    case AI_UINT64:
                        meta_data_type = "AI_UINT64";
                        continue;
                        //long_value = *(int64_t *)meta_data->mValues[j].mData;
                        //sprintf(value_buffer, "%d", long_value);
                    break;

                    case AI_FLOAT:
                        meta_data_type = "AI_FLOAT";
                        float_value = *(float *)meta_data->mValues[j].mData;
                        sprintf(value_buffer, "%f", float_value);
                    break;

                    case AI_DOUBLE:
                        meta_data_type = "AI_DOUBLE";
                        continue;
                    break;

                    case AI_AISTRING:
                        meta_data_type = "AI_AISTRING";
                        string_value = (aiString *)meta_data->mValues[j].mData;
                        sprintf(value_buffer, "%s", string_value->C_Str());
                    break;

                    case AI_AIVECTOR3D:
                        meta_data_type = "AI_AIVECTOR3D";
                        vector_value = *(aiVector3D *)meta_data->mValues[j].mData;
                        sprintf(value_buffer, "[%f %f %f]", vector_value.x, vector_value.y, vector_value.z);
                    break;

                    default:
                        continue;
                    break;
                }

                printf("{%s (%s) : %s}\n", meta_data->mKeys[j].C_Str(), meta_data_type, value_buffer);
            }
        }


        for(i = 0; i < node->mNumChildren; i++)
        {
            traverse_nodes2(node->mChildren[i]);
        }
    }

    level--;
}



void build_skeleton_recursive(struct node_anim_node_t *node, offline::RawSkeleton::Joint *joint)
{
    int i;
    int j;

    static int level = -1;

    level++;

    aiMetadata *meta_data;

    aiVector3D position;
    aiVector3D scale;
    aiQuaternion rotation;

    node->node->mTransformation.Decompose(scale, rotation, position);

    joint->transform.translation.x = position.x;
    joint->transform.translation.y = position.y;
    joint->transform.translation.z = position.z;

    joint->transform.scale.x = scale.x;
    joint->transform.scale.y = scale.y;
    joint->transform.scale.z = scale.z;

    joint->transform.rotation.x = rotation.x;
    joint->transform.rotation.y = rotation.y;
    joint->transform.rotation.z = rotation.z;
    joint->transform.rotation.w = rotation.w;

    joint->name = node->node->mName.C_Str();

    for(i = 0; i < level; i++)
    {
        printf("-");
    }

    printf("%s\n", node->node->mName.C_Str());

    joint->children.resize(node->children_count);

    for(i = 0; i < node->children_count; i++)
    {
        build_skeleton_recursive(node->children[i], &joint->children[i]);
    }

    level--;
}




void build_skeleton(struct node_anim_node_t *node_tree, offline::RawSkeleton *skeleton)
{
    int i;

    offline::RawSkeleton::Joint *root_joint;

    if(node_tree)
    {
        skeleton->roots.resize(node_tree->children_count);

        for(i = 0; i < node_tree->children_count; i++)
        {
            root_joint = &skeleton->roots[i];

            build_skeleton_recursive(node_tree->children[i], root_joint);
        }
    }
}

/*
===============================================================================
===============================================================================
===============================================================================
*/


void build_animation_recursive(struct node_anim_node_t *node_tree, ozz::animation::offline::RawAnimation *raw_animation)
{
    int i;
    ozz::animation::offline::RawAnimation::JointTrack joint_track;
    ozz::animation::offline::RawAnimation::JointTrack *parent_track = NULL;
    aiNodeAnim *node_anim;

    struct node_anim_node_t *parent_node_tree;

    static int level = -1;

    aiVector3D parent_translation = aiVector3D(0.0, 0.0, 0.0);
    aiQuaternion parent_rotation = aiQuaternion();
    aiVector3D parent_scaling = aiVector3D(1.0, 1.0, 1.0);



    level++;

    if(level)
    {
        node_anim = node_tree->node_anim;
        parent_node_tree = node_tree->parent;

        joint_track.translations.resize(node_anim->mNumPositionKeys);
        joint_track.rotations.resize(node_anim->mNumRotationKeys);
        joint_track.scales.resize(node_anim->mNumScalingKeys);

        if(parent_node_tree->node_index >= 0)
        {
            parent_track = &raw_animation->tracks[parent_node_tree->node_index];
        }


        for(i = 0; i < node_anim->mNumPositionKeys; i++)
        {
            /*if(parent_track)
            {
                parent_translation.x = parent_track->translations[i].value.x;
                parent_translation.y = parent_track->translations[i].value.y;
                parent_translation.z = parent_track->translations[i].value.z;
            }*/

            joint_track.translations[i].value.x = parent_translation.x + node_anim->mPositionKeys[i].mValue.x;
            joint_track.translations[i].value.y = parent_translation.y + node_anim->mPositionKeys[i].mValue.y;
            joint_track.translations[i].value.z = parent_translation.z + node_anim->mPositionKeys[i].mValue.z;

            joint_track.translations[i].time = node_anim->mPositionKeys[i].mTime;
        }

        for(i = 0; i < node_anim->mNumRotationKeys; i++)
        {
            /*if(parent_track)
            {
                parent_rotation.x = parent_track->rotations[i].value.x;
                parent_rotation.y = parent_track->rotations[i].value.y;
                parent_rotation.z = parent_track->rotations[i].value.z;
                parent_rotation.w = parent_track->rotations[i].value.w;
            }*/

            aiQuaternion rotation = node_anim->mRotationKeys[i].mValue;


            joint_track.rotations[i].value.x = rotation.x;
            joint_track.rotations[i].value.y = rotation.y;
            joint_track.rotations[i].value.z = rotation.z;
            joint_track.rotations[i].value.w = rotation.w;

            joint_track.rotations[i].time = node_anim->mRotationKeys[i].mTime;
        }

        for(i = 0; i < node_anim->mNumScalingKeys; i++)
        {
            /*if(parent_track)
            {
                parent_scaling.x = parent_track->scales[i].value.x;
                parent_scaling.y = parent_track->scales[i].value.y;
                parent_scaling.z = parent_track->scales[i].value.z;
            }*/

            joint_track.scales[i].value.x = parent_scaling.x * node_anim->mScalingKeys[i].mValue.x;
            joint_track.scales[i].value.y = parent_scaling.y * node_anim->mScalingKeys[i].mValue.y;
            joint_track.scales[i].value.z = parent_scaling.z * node_anim->mScalingKeys[i].mValue.z;

            joint_track.scales[i].time = node_anim->mScalingKeys[i].mTime;
        }

        raw_animation->tracks[node_tree->node_index] = joint_track;
    }

    for(i = 0; i < node_tree->children_count; i++)
    {
        build_animation_recursive(node_tree->children[i], raw_animation);
    }

    level--;
}

void build_animation(struct node_anim_node_t *node_tree, aiAnimation *animation, ozz::animation::offline::RawAnimation *raw_animation)
{

    fill_node_tree_node_anim(animation, node_tree);

    raw_animation->tracks.resize(animation->mNumChannels);
    build_animation_recursive(node_tree, raw_animation);

    raw_animation->duration = animation->mDuration;
    raw_animation->name = fix_stupid_name((char *)animation->mName.C_Str());
}

/*
===============================================================================
===============================================================================
===============================================================================
*/



int main(int argc, char *argv[])
{
    Assimp::Importer importer;
    ozz::animation::offline::SkeletonBuilder builder;
    ozz::animation::offline::RawSkeleton raw_skeleton;
    ozz::animation::Skeleton *skeleton;

    ozz::animation::offline::AnimationBuilder animation_builder;
    ozz::animation::offline::RawAnimation raw_animation;
    ozz::animation::Animation *animation;
    ozz::animation::offline::RawAnimation::JointTrack *track;

    aiAnimation *src_animation;
    aiNodeAnim *node_anim;

    struct node_anim_node_t *node_tree;

    const aiScene *scene;
    char *anim_name;
    char *arg;
    int i;
    int j;

    int import_type = -1;

    char input_file[256] = {0};
    char output_file[256] = {0};

    if(argc)
    {
        for(i = 1; i < argc; i++)
        {
            arg = argv[i];

            if(arg[0] == '-')
            {
                switch(arg[1])
                {
                    case 'f':
                    case 'F':
                        i++;
                        strcpy(input_file, argv[i]);
                        continue;
                    break;
                }
            }
        }

        //scene = importer.ReadFile("metarig_test.fbx", 0);

        scene = importer.ReadFile(input_file, 0);

        //traverse_nodes2(scene->mRootNode);

        if(!output_file[0])
        {
            for(i = 0; input_file[i] && input_file[i] != '.'; i++)
            {
                output_file[i] = input_file[i];
            }

            if(input_file[i] == '.')
            {
                if(strcmp(input_file + i, ".ozz"))
                {
                    strcpy(output_file + i, ".ozz");
                }
            }
            else
            {
                strcat(output_file, ".ozz");
            }
        }
        else
        {
            arg = strstr(output_file, ".");

            if(arg)
            {
                if(strcmp(arg, ".ozz"))
                {
                    strcpy(arg, ".ozz");
                }
            }
            else
            {
                strcat(output_file, ".ozz");
            }
        }

        node_tree = build_node_tree(scene);

        if(node_tree)
        {
            build_skeleton(node_tree, &raw_skeleton);

            if(raw_skeleton.Validate())
            {
                skeleton = builder(raw_skeleton);

                ozz::io::File file(output_file, "wb");
                ozz::io::OArchive archive(&file);

                archive << *skeleton;

                ozz::memory::default_allocator()->Delete(skeleton);

                file.Close();


                for(i = 0; i < scene->mNumAnimations; i++)
                {
                    src_animation = scene->mAnimations[i];

                    if(!strcmp(src_animation->mChannels[0]->mNodeName.C_Str(), node_tree->node->mName.C_Str()))
                    {
                        build_animation(node_tree, src_animation, &raw_animation);

                        if(raw_animation.Validate())
                        {
                            animation = animation_builder(raw_animation);

                            anim_name = fix_stupid_name((char *)src_animation->mName.C_Str());

                            strcpy(output_file, anim_name);
                            strcat(output_file, ".ozz");

                            ozz::io::File file(output_file, "wb");
                            ozz::io::OArchive archive(&file);

                            archive << *animation;

                            ozz::memory::default_allocator()->Delete(animation);

                            file.Close();


                            printf("animation %s exported successfully!\n", output_file);
                        }
                        else
                        {
                            printf("animation is not valid!\n");
                            return 1;
                        }

                        raw_animation.tracks.clear();
                    }
                }


            }

            else
            {
                printf("skeleton is invalid!\n");
            }
        }

    }
}







