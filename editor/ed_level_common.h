#ifndef ED_LEVEL_COMMON_H
#define ED_LEVEL_COMMON_H


#include "vector.h"
#include "matrix.h"

#include "r_view.h"

#include "ed_picklist.h"


enum LEVEL_EDITOR_TRANSFORM_OPERATION
{
	LEVEL_EDITOR_TRANSFORM_OPERATION_TRANSLATION = 0,
	LEVEL_EDITOR_TRANSFORM_OPERATION_ROTATION,
	LEVEL_EDITOR_TRANSFORM_OPERATION_SCALING,
};

enum LEVEL_EDITOR_TRANSFORM_FRAME
{
	LEVEL_EDITOR_TRANSFORM_FRAME_LOCAL,
	LEVEL_EDITOR_TRANSFORM_FRAME_GLOBAL,
};

enum LEVEL_EDITOR_TRANSFORM_PIVOT
{
	LEVEL_EDITOR_TRANSFORM_PIVOT_MEDIAN_POINT = 1,
	LEVEL_EDITOR_TRANSFORM_PIVOT_INDIVIDUAL_ORIGINS,
	LEVEL_EDITOR_TRANSFORM_PIVOT_3D_CURSOR
};

//enum LEVEL_EDITOR_EDITING_MODE
//{
//	EDITING_MODE_OBJECT,
//	EDITING_MODE_BRUSH,
//	EDITING_MODE_UV,
//};




struct transform_manipulator_t
{
    vec3_t position;
    int transform_operation;
};


enum SELECTION_TRANSFORM_CONSTRAINT
{
    SELECTION_TRANSFORM_CONSTRAINT_NONE = 0,
    SELECTION_TRANSFORM_CONSTRAINT_X_AXIS = 1,
    SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS = 1 << 1,
    SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS = 1 << 2,
};


struct selection_state_t
{
    float linear_snap_value;
    float angula_snap_value;

    int axis_constraint;

    int transform_frame;
    int transform_pivot;
    int transform_operation;

    //struct transform_manipulator_t manipulator;
    int use_manipulator;

    vec3_t manipulator_position;
    vec3_t cursor_position;

    struct pick_list_t pick_list;

    vec3_t cur_mouse_delta;
    vec3_t prev_mouse_delta;


    vec3_t cur_mouse_pos;
    vec3_t prev_mouse_pos;


    vec3_t translation_delta;


    vec3_t scaling_axis;
    float scaling_delta;

    vec3_t rotation_axis;
    float rotation_delta;

    unsigned int last_update;
};


struct editor_state_t
{
    float editor_camera_pitch;
    float editor_camera_yaw;
    float editor_camera_speed_multiplier;
    struct view_handle_t editor_view;
    int transforming_selection;
    int first_transform_frame;
    int editor_state;
    int edition_target;

    struct selection_state_t selection_state;
};

#endif // ED_LEVEL_COMMON_H








