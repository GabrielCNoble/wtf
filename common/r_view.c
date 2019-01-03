#include "r_view.h"
#include "c_memory.h"
#include "r_main.h"
#include "containers/stack_list.h"
#include <string.h>


extern struct stack_list_t r_views;
extern struct view_data_t *r_active_views[R_MAX_ACTIVE_VIEWS];
extern struct view_handle_t r_main_view;
extern struct view_def_t r_default_view;

struct view_handle_t renderer_CreateViewDef(char *name, vec3_t position, mat3_t *orientation, float fovy, float width, float height, float znear, float zfar, int flags)
{
    struct view_def_t *view_def;
    struct view_handle_t handle = INVALID_VIEW_HANDLE;

    handle.view_index = stack_list_add(&r_views, NULL);
    view_def = stack_list_get(&r_views, handle.view_index);

    view_def->flags = 0;
    view_def->name = memory_Strdup(name);
    view_def->world_position = position;


    if(!orientation)
    {
        view_def->world_orientation = mat3_t_id();
    }
    else
    {
        view_def->world_orientation = *orientation;
    }


	CreatePerspectiveMatrix(&view_def->view_data.projection_matrix, fovy, width/height, znear, zfar, 0.0, 0.0, &view_def->frustum);
	renderer_CreateViewData(&view_def->view_data);

	view_def->width = width;
	view_def->height = height;
	view_def->fov_y = fovy;

	renderer_ComputeViewMatrix(handle);

	return handle;
}

void renderer_CreateViewData(struct view_data_t *view_data)
{
    view_data->draw_commands_frame = 0;
    view_data->draw_commands = list_create(sizeof(struct draw_command_t), R_MAX_DRAW_COMMANDS, NULL);
}

void renderer_DestroyViewData(struct view_data_t *view_data)
{
    list_destroy(&view_data->draw_commands);
}

void renderer_DestroyViewDef(struct view_handle_t view)
{
    struct view_def_t *view_def;


    if(view.view_index == DEFAULT_VIEW_INDEX)
    {
        return;
    }

    if(view.view_index != INVALID_VIEW_INDEX)
    {
        view_def = renderer_GetViewPointer(view);

        if(view_def)
        {
            if(view.view_index == r_main_view.view_index)
            {
                renderer_SetMainView(DEFAULT_VIEW_HANDLE);
            }

            view_def->flags |= R_VIEW_FLAG_INVALID;
            stack_list_remove(&r_views, view.view_index);
        }
    }
}



void renderer_SetMainView(struct view_handle_t view)
{
    struct view_def_t *view_def;

    r_main_view = view;

    view_def = renderer_GetViewPointer(view);

    if(!view_def)
    {
        r_main_view = DEFAULT_VIEW_HANDLE;
        view_def = &r_default_view;
    }

	renderer_SetProjectionMatrix(&view_def->view_data.projection_matrix);
	renderer_SetViewMatrix(&view_def->view_data.view_matrix);
}

struct view_def_t *renderer_GetMainViewPointer()
{
    return renderer_GetViewPointer(r_main_view);
}

struct view_handle_t renderer_GetMainView()
{
    return r_main_view;
}

struct view_def_t *renderer_GetViewPointer(struct view_handle_t view)
{
    struct view_def_t *view_def = NULL;

    if(view.view_index == DEFAULT_VIEW_INDEX)
    {
        return &r_default_view;
    }

    if(view.view_index >= 0 && view.view_index < r_views.element_count)
    {
        view_def = (struct view_def_t *)stack_list_get(&r_views, view.view_index);

        if(view_def->flags & R_VIEW_FLAG_INVALID)
        {
            view_def = NULL;
        }
    }

    return view_def;
}

struct view_handle_t renderer_GetViewByName(char *view_name)
{
    struct view_handle_t view = INVALID_VIEW_HANDLE;
    struct view_def_t *views;
    int i;


    views = (struct view_def_t *)r_views.elements;

    if(!strcmp(view_name, "default view"))
    {
        return DEFAULT_VIEW_HANDLE;
    }

    for(i = 0; i < r_views.element_count; i++)
    {
        if(views[i].flags & R_VIEW_FLAG_INVALID)
        {
            continue;
        }

        if(!strcmp(views[i].name, view_name))
        {
            view.view_index = i;
            break;
        }
    }

    return view;
}

struct view_def_t *renderer_GetViewPointerByName(char *view_name)
{
    struct view_handle_t view_handle;

    view_handle = renderer_GetViewByName(view_name);
    return renderer_GetViewPointer(view_handle);
}




void renderer_PitchYawView(struct view_handle_t view, float yaw, float pitch)
{
    mat3_t p;
	mat3_t y;

	float s;
	float c;

	struct view_def_t *view_def = renderer_GetViewPointer(view);

	if(view_def)
    {
        /* First apply pitch, THEN yaw. This is necesary
        because the camera is using the world vector
        (1, 0, 0) as its right vector, which is not
        modified when the camera turns. So, if applying
        the yaw first, the camera's right vector might
        become its forward vector at times, so it will
        roll instead of pitch. To apply the yaw before
        the pitch, the world right vector must be rotated
        by the yaw matrix, so it will always be pointing
        to the right relative to the camera.*/
        //mat3_t_mult(&camera->local_orientation, &p, &y);

        s = sin(pitch * 3.14159265);
        c = cos(pitch * 3.14159265);

        p.floats[0][0] = 1.0;
        p.floats[0][1] = 0.0;
        p.floats[0][2] = 0.0;

        p.floats[1][0] = 0.0;
        p.floats[1][1] = c;
        p.floats[1][2] = s;

        p.floats[2][0] = 0.0;
        p.floats[2][1] = -s;
        p.floats[2][2] = c;



        s = sin(yaw * 3.14159265);
        c = cos(yaw * 3.14159265);

        y.floats[0][0] = c;
        y.floats[0][1] = 0.0;
        y.floats[0][2] = -s;

        y.floats[1][0] = 0.0;
        y.floats[1][1] = 1.0;
        y.floats[1][2] = 0.0;

        y.floats[2][0] = s;
        y.floats[2][1] = 0.0;
        y.floats[2][2] = c;


        //end = _rdtsc();

        //printf("%llu\n", end - start);

        mat3_t_mult(&view_def->world_orientation, &p, &y);



        //camera_ComputeWorldToCameraMatrix(camera);

        //mat3_t_mult(&camera->local_orientation, &y, &p);
	}
}

void renderer_TranslateView(struct view_handle_t view, vec3_t direction, float amount, int set)
{

    struct view_def_t *view_def;

    view_def = renderer_GetViewPointer(view);

    if(view_def)
    {
        if(set)
        {
            view_def->world_position.floats[0] = direction.floats[0]*amount;
            view_def->world_position.floats[1] = direction.floats[1]*amount;
            view_def->world_position.floats[2] = direction.floats[2]*amount;
        }
        else
        {
            view_def->world_position.floats[0] += direction.floats[0]*amount;
            view_def->world_position.floats[1] += direction.floats[1]*amount;
            view_def->world_position.floats[2] += direction.floats[2]*amount;
        }

        renderer_ComputeViewMatrix(view);
    }


        return;
}



void renderer_ComputeViewMatrix(struct view_handle_t view)
{

    struct view_def_t *view_def;

    view_def = renderer_GetViewPointer(view);

    if(view_def)
    {
        view_def->view_data.view_matrix.floats[0][0] = view_def->world_orientation.floats[0][0];
        view_def->view_data.view_matrix.floats[1][0] = view_def->world_orientation.floats[0][1];
        view_def->view_data.view_matrix.floats[2][0] = view_def->world_orientation.floats[0][2];

        view_def->view_data.view_matrix.floats[3][0] =  (-view_def->world_position.floats[0]) * view_def->view_data.view_matrix.floats[0][0]	+
                                                        (-view_def->world_position.floats[1]) * view_def->view_data.view_matrix.floats[1][0]	+
                                                        (-view_def->world_position.floats[2]) * view_def->view_data.view_matrix.floats[2][0];



        view_def->view_data.view_matrix.floats[0][1] = view_def->world_orientation.floats[1][0];
        view_def->view_data.view_matrix.floats[1][1] = view_def->world_orientation.floats[1][1];
        view_def->view_data.view_matrix.floats[2][1] = view_def->world_orientation.floats[1][2];

        view_def->view_data.view_matrix.floats[3][1] =  (-view_def->world_position.floats[0]) * view_def->view_data.view_matrix.floats[0][1]	+
                                                        (-view_def->world_position.floats[1]) * view_def->view_data.view_matrix.floats[1][1]	+
                                                        (-view_def->world_position.floats[2]) * view_def->view_data.view_matrix.floats[2][1];




        view_def->view_data.view_matrix.floats[0][2] = view_def->world_orientation.floats[2][0];
        view_def->view_data.view_matrix.floats[1][2] = view_def->world_orientation.floats[2][1];
        view_def->view_data.view_matrix.floats[2][2] = view_def->world_orientation.floats[2][2];

        view_def->view_data.view_matrix.floats[3][2] =  (-view_def->world_position.floats[0]) * view_def->view_data.view_matrix.floats[0][2]	+
                                                        (-view_def->world_position.floats[1]) * view_def->view_data.view_matrix.floats[1][2]	+
                                                        (-view_def->world_position.floats[2]) * view_def->view_data.view_matrix.floats[2][2];


        view_def->view_data.view_matrix.floats[0][3] = 0.0;
        view_def->view_data.view_matrix.floats[1][3] = 0.0;
        view_def->view_data.view_matrix.floats[2][3] = 0.0;
        view_def->view_data.view_matrix.floats[3][3] = 1.0;
    }
}

void renderer_ComputeMainViewMatrix()
{
    renderer_ComputeViewMatrix(r_main_view);
}
