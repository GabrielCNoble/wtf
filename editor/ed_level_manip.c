#include "ed_level_manip.h"
#include "..\common\r_view.h"
#include "..\common\r_imediate.h"
#include "..\common\r_gl.h"
#include "..\common\r_shader.h"
#include "..\common\entity.h"
#include "..\common\l_main.h"
#include "brush.h"


extern struct renderer_t r_renderer;


extern struct level_editor_selection_state_t level_editor_selection_state;
extern struct view_handle_t level_editor_view;


extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern float mouse_dx;
extern float mouse_dy;

void editor_LevelEditorTransformOp(struct selection_state_t *selection_state, int op)
{
    switch(op)
    {
        case LEVEL_EDITOR_TRANSFORM_OPERATION_TRANSLATION:
        case LEVEL_EDITOR_TRANSFORM_OPERATION_ROTATION:
        case LEVEL_EDITOR_TRANSFORM_OPERATION_SCALING:
            selection_state->transform_operation = op;
            selection_state->last_update = r_renderer.r_statistics.r_frame - 2;
        break;
    }
}

void editor_LevelEditorTransformFrame(struct selection_state_t *selection_state, int frame)
{
    switch(frame)
    {
        case LEVEL_EDITOR_TRANSFORM_FRAME_GLOBAL:
        case LEVEL_EDITOR_TRANSFORM_FRAME_LOCAL:
            selection_state->transform_frame = frame;
            selection_state->last_update = r_renderer.r_statistics.r_frame - 2;
        break;
    }
}

void editor_LevelEditorTransformPivot(struct selection_state_t *selection_state, int pivot)
{
    switch(pivot)
    {
        case LEVEL_EDITOR_TRANSFORM_PIVOT_MEDIAN_POINT:
        case LEVEL_EDITOR_TRANSFORM_PIVOT_INDIVIDUAL_ORIGINS:
        case LEVEL_EDITOR_TRANSFORM_PIVOT_3D_CURSOR:
            selection_state->transform_pivot = pivot;
            selection_state->last_update = r_renderer.r_statistics.r_frame - 2;
        break;
    }
}

void editor_LevelEditorTransformAxis(struct selection_state_t *selection_state, int axis_constraint)
{
    switch(axis_constraint)
    {
        case SELECTION_TRANSFORM_CONSTRAINT_X_AXIS:
        case SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS:
        case SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS:
            selection_state->axis_constraint &= axis_constraint;
            selection_state->axis_constraint ^= axis_constraint;
            selection_state->last_update = r_renderer.r_statistics.r_frame - 2;
        break;

        case SELECTION_TRANSFORM_CONSTRAINT_NONE:
            selection_state->axis_constraint = 0;
        break;
    }
}

vec3_t editor_LevelEditorGetCenterOfSelections(struct selection_state_t *selection_state)
{
    int i;
    struct pick_record_t *record;
    struct entity_t *entity;
    struct brush_t *brush;

    vec3_t center = vec3_t_c(0.0, 0.0, 0.0);
    vec3_t position;

    if(selection_state->pick_list.record_count)
    {
        for(i = 0; i < selection_state->pick_list.record_count; i++)
        {
            record = selection_state->pick_list.records + i;

            switch(record->type)
            {
                case PICK_ENTITY:
                    position = entity_GetEntityPosition(ENTITY_HANDLE(record->index0, 0), 1);
                break;

                case PICK_LIGHT:
                    position = light_GetLightPosition(record->index0);
                break;

                case PICK_BRUSH:
                    brush = (struct brush_t *)record->pointer;
                    position = brush->position;
                break;
            }

            center.x += position.x;
            center.y += position.y;
            center.z += position.z;
        }

        center.x /= selection_state->pick_list.record_count;
        center.y /= selection_state->pick_list.record_count;
        center.z /= selection_state->pick_list.record_count;
    }


    return center;
}

vec3_t editor_LevelEditorGetSelectionPosition(struct pick_record_t *record)
{
    struct brush_t *brush;

    switch(record->type)
    {
        case PICK_BRUSH:
            brush = (struct brush_t *)record->pointer;
            return brush->position;
        break;

        case PICK_LIGHT:
            return light_GetLightPosition(record->index0);
        break;
    }
}

void editor_LevelEditorCalculateMouseDelta(struct selection_state_t *selection_state)
{
    struct view_def_t *active_view;
    vec3_t transform_plane_rvec;
    vec3_t transform_plane_uvec;
    vec3_t transform_plane_fvec;

    vec3_t selection_camera_vector;
    vec3_t selection_center;
    vec3_t mouse_vector;
    vec3_t plane_mouse_vector;

    vec3_t camera_right_vector;
    vec3_t camera_forward_vector;
    vec3_t camera_up_vector;

    //vec2_t screen_mouse;
    vec3_t mouse_delta;
    vec3_t prev_mouse_delta;
    vec3_t camera_mouse;
    vec3_t world_mouse;
    //vec3_t world_mouse_b;
    vec3_t world_mouse_delta;
    //vec3_t plane_normal;

    float transform_scale;
    float d;

    //active_view = renderer_GetViewPointer(level_editor_view);
    active_view = renderer_GetMainViewPointer();

    selection_center = editor_LevelEditorGetCenterOfSelections(selection_state);

    selection_camera_vector = sub3(active_view->world_position, selection_center);

    camera_up_vector.x = active_view->world_orientation.floats[1][0];
    camera_up_vector.y = active_view->world_orientation.floats[1][1];
    camera_up_vector.z = active_view->world_orientation.floats[1][2];

    camera_right_vector.x = active_view->world_orientation.floats[0][0];
    camera_right_vector.y = active_view->world_orientation.floats[0][1];
    camera_right_vector.z = active_view->world_orientation.floats[0][2];

    camera_forward_vector.x = active_view->world_orientation.floats[2][0];
    camera_forward_vector.y = active_view->world_orientation.floats[2][1];
    camera_forward_vector.z = active_view->world_orientation.floats[2][2];

    camera_mouse.x = normalized_mouse_x * (active_view->frustum.right / active_view->frustum.znear);
    camera_mouse.y = normalized_mouse_y * (active_view->frustum.top / active_view->frustum.znear);
    camera_mouse.z = -1.0;


    mouse_vector = mul3(add3(mul3(camera_right_vector, camera_mouse.x),
                        add3(mul3(camera_up_vector, camera_mouse.y),
                             mul3(camera_forward_vector, camera_mouse.z))), 1.0);

    transform_plane_fvec = selection_camera_vector;

    switch(selection_state->axis_constraint)
    {
        case SELECTION_TRANSFORM_CONSTRAINT_X_AXIS:
            transform_plane_fvec.x = 0.0;
        break;

        case SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS:
            transform_plane_fvec.y = 0.0;
        break;

        case SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS:
            transform_plane_fvec.z = 0.0;
        break;

        default:
            transform_plane_fvec = camera_forward_vector;
        break;
    }

    transform_plane_fvec = normalize3(transform_plane_fvec);

    plane_mouse_vector = sub3(selection_center, active_view->world_position);

    d = dot3(transform_plane_fvec, plane_mouse_vector) / dot3(transform_plane_fvec, mouse_vector);

    if(dot3(transform_plane_fvec, camera_forward_vector) < 0.0)
    {
        d = -d;
    }

    world_mouse = add3(active_view->world_position, mul3(mouse_vector, d));

    selection_state->prev_mouse_pos = selection_state->cur_mouse_pos;
    selection_state->cur_mouse_pos = world_mouse;

    world_mouse_delta = sub3(selection_state->cur_mouse_pos, selection_state->prev_mouse_pos);

    selection_state->prev_mouse_delta = selection_state->cur_mouse_delta;
    selection_state->cur_mouse_delta = world_mouse_delta;

    switch(selection_state->transform_operation)
    {
        case LEVEL_EDITOR_TRANSFORM_OPERATION_TRANSLATION:

            selection_state->translation_delta = selection_state->cur_mouse_delta;

            switch(selection_state->axis_constraint)
            {
                case SELECTION_TRANSFORM_CONSTRAINT_X_AXIS:
                    selection_state->translation_delta.y = 0.0;
                    selection_state->translation_delta.z = 0.0;
                break;

                case SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS:
                    selection_state->translation_delta.x = 0.0;
                    selection_state->translation_delta.z = 0.0;
                break;

                case SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS:
                    selection_state->translation_delta.x = 0.0;
                    selection_state->translation_delta.y = 0.0;
                break;
            }
        break;

        case LEVEL_EDITOR_TRANSFORM_OPERATION_SCALING:
            selection_state->scaling_axis = vec3_t_c(1.0, 1.0, 1.0);

            switch(selection_state->axis_constraint)
            {
                case SELECTION_TRANSFORM_CONSTRAINT_X_AXIS:
                    selection_state->scaling_axis.y = 0.0;
                    selection_state->scaling_axis.z = 0.0;
                break;

                case SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS:
                    selection_state->scaling_axis.x = 0.0;
                    selection_state->scaling_axis.z = 0.0;
                break;

                case SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS:
                    selection_state->scaling_axis.x = 0.0;
                    selection_state->scaling_axis.y = 0.0;
                break;
            }

            selection_state->scaling_delta = length3(sub3(selection_state->cur_mouse_pos, selection_center)) -
                                             length3(sub3(selection_state->prev_mouse_pos, selection_center));
        break;

        case LEVEL_EDITOR_TRANSFORM_OPERATION_ROTATION:

            mouse_delta = normalize3(sub3(selection_center, selection_state->cur_mouse_pos));
            prev_mouse_delta = normalize3(sub3(selection_center, selection_state->prev_mouse_pos));

            selection_state->rotation_delta = dot3(cross(prev_mouse_delta, mouse_delta), transform_plane_fvec) / M_PI;

            switch(selection_state->axis_constraint)
            {
                case SELECTION_TRANSFORM_CONSTRAINT_X_AXIS:
                    selection_state->rotation_axis = vec3_t_c(1.0, 0.0, 0.0);
                break;

                case SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS:
                    selection_state->rotation_axis = vec3_t_c(0.0, 1.0, 0.0);
                break;

                case SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS:
                    selection_state->rotation_axis = vec3_t_c(0.0, 0.0, 1.0);
                break;

                default:
                    selection_state->rotation_axis = transform_plane_fvec;
                break;
            }
        break;
    }

    if(r_renderer.r_statistics.r_frame - selection_state->last_update > 1)
    {
        selection_state->translation_delta = vec3_t_c(0.0, 0.0, 0.0);
        selection_state->rotation_delta = 0.0;
        selection_state->scaling_delta = 0.0;
    }

    selection_state->last_update = r_renderer.r_statistics.r_frame;
}




void editor_LevelEditorTranslateSelections(struct selection_state_t *selection_state, vec3_t translation)
{
    int i;
    int c;
    struct pick_record_t *records;

    void (*translate_function)(struct pick_record_t *, vec3_t);


    records = selection_state->pick_list.records;
    c = selection_state->pick_list.record_count;

    for(i = 0; i < c; i++)
    {
        switch(records[i].type)
        {
            case PICK_BRUSH:
                translate_function = editor_LevelEditorTranslateBrush;
            break;

            case PICK_LIGHT:
                translate_function = editor_LevelEditorTranslateLight;
            break;
        }

        translate_function(records + i, translation);
    }
}

void editor_LevelEditorRotateSelections(struct selection_state_t *selection_state, vec3_t axis, float angle)
{
    int i;
    int c;
    struct pick_record_t *records;

    mat3_t rotation;
    vec3_t selection_center;
    vec3_t selection_position;
    vec3_t rotated_selection_position;
    vec3_t rotated_translation;

    void (*rotate_function)(struct pick_record_t *, vec3_t, float);
    void (*translate_function)(struct pick_record_t *, vec3_t);


    records = selection_state->pick_list.records;
    c = selection_state->pick_list.record_count;

    selection_center = editor_LevelEditorGetCenterOfSelections(selection_state);

    mat3_t_rotate(&rotation, axis, angle, 1);

    for(i = 0; i < c; i++)
    {
        switch(records[i].type)
        {
            case PICK_BRUSH:
                rotate_function = editor_LevelEditorRotateBrush;
                translate_function = editor_LevelEditorTranslateBrush;
            break;

            case PICK_LIGHT:
                rotate_function = editor_LevelEditorRotateLight;
                translate_function = editor_LevelEditorTranslateLight;
            break;
        }

        selection_position = editor_LevelEditorGetSelectionPosition(records + i);
        rotated_selection_position = sub3(selection_center, selection_position);
        mat3_t_vec3_t_mult(&rotation, &rotated_selection_position);
        rotated_selection_position = add3(rotated_selection_position, selection_center);
        rotated_translation = sub3(rotated_selection_position, selection_position);

        rotate_function(records + i, axis, angle);
        translate_function(records + i, rotated_translation);
    }
}

void editor_LevelEditorScaleSelections(struct selection_state_t *selection_state, vec3_t axis, float scaling)
{
    int i;
    struct pick_record_t *records;

    records = selection_state->pick_list.records;

    for(i = 0; i < selection_state->pick_list.record_count; i++)
    {
        switch(records[i].type)
        {
            case PICK_BRUSH:
                editor_LevelEditorScaleBrush(records + i, axis, scaling);
            break;
        }
    }
}

void editor_LevelEditorCopySelections(struct selection_state_t *selection_state)
{
    int i;
    struct pick_record_t *records;

    records = selection_state->pick_list.records;

    for(i = 0; i < selection_state->pick_list.record_count; i++)
    {
        switch(records[i].type)
        {
            case PICK_BRUSH:
                records[i].pointer = brush_CopyBrush(records[i].pointer);
            break;

            case PICK_LIGHT:
                records[i].index0 = light_CopyLightIndex(records[i].index0);
            break;
        }
    }
}








void editor_LevelEditorTranslateBrush(struct pick_record_t *pick_record, vec3_t translation)
{
    brush_TranslateBrush(pick_record->pointer, translation);
}

void editor_LevelEditorRotateBrush(struct pick_record_t *pick_record, vec3_t axis, float angle)
{
    brush_RotateBrush(pick_record->pointer, axis, angle);
}

void editor_LevelEditorScaleBrush(struct pick_record_t *pick_record, vec3_t axis, float scaling)
{
    brush_ScaleBrush(pick_record->pointer, axis, scaling);
}





void editor_LevelEditorTranslateLight(struct pick_record_t *pick_record, vec3_t translation)
{
    light_TranslateLight(pick_record->index0, translation, 1.0);
}

void editor_LevelEditorRotateLight(struct pick_record_t *pick_record, vec3_t axis, float angle)
{

}





