#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "portal.h"
#include "c_memory.h"
#include "camera.h"

int ptl_portal_list_cursor = 0;
int ptl_portal_list_size = 0;
int ptl_portal_free_stack_top = -1;
int *ptl_portal_free_stack = NULL;
portal_t *ptl_portals = NULL;


/* from r_main.c */

extern int r_width;
extern int r_height;
extern int r_frame;

#ifdef __cplusplus
extern "C"
{
#endif

int portal_Init()
{
	ptl_portal_list_size = 32;
	ptl_portal_free_stack = memory_Malloc(sizeof(int) * ptl_portal_list_size);
	ptl_portals = memory_Malloc(sizeof(portal_t) * ptl_portal_list_size);
	return 1;
}

void portal_Finish()
{
	memory_Free(ptl_portals);
	memory_Free(ptl_portal_free_stack);
}

int portal_CreatePortal(vec3_t position, vec2_t extents, mat3_t *orientation, int max_recursion)
{
	portal_t *portal;
	int portal_index;
	int i;

	if(ptl_portal_free_stack_top >= 0)
	{
		portal_index = ptl_portal_free_stack[ptl_portal_free_stack_top];
		ptl_portal_free_stack_top--;
	}
	else
	{
		portal_index = ptl_portal_list_cursor;
		ptl_portal_list_cursor++;

		if(portal_index >= ptl_portal_list_size)
		{
			memory_Free(ptl_portal_free_stack);

			ptl_portal_free_stack = memory_Malloc(sizeof(int) * (ptl_portal_list_size + 16));
			portal = memory_Malloc(sizeof(portal_t) * (ptl_portal_list_size + 16));

			memcpy(portal, ptl_portals, sizeof(portal_t) * ptl_portal_list_size);

			memory_Free(ptl_portals);
			ptl_portals = portal;
			ptl_portal_list_size += 16;
		}
	}

	portal = &ptl_portals[portal_index];


	//portal->view = camera_CreateCamera("portal camera", position, orientation, 0.5, extents.x, extents.y, 0.1, 500.0, 0);

	portal->portal_recursive_views = memory_Malloc(sizeof(portal_recursive_view_data_t) * W_MAX_PORTAL_RECURSION_LEVEL);

	/* first level of recursion, only seen by the current view point... */
	portal->portal_recursive_views[0].views = memory_Malloc(sizeof(portal_view_data_t));
	portal->portal_recursive_views[0].views_count = 0;
	portal->portal_recursive_views[0].views_max = 1;
	portal->portal_recursive_views[0].frame = 0;

	portal->max_recursion = max_recursion;

//	camera_InitViewData(&portal->portal_recursive_views[0].views[0].view_data);

	for(i = 1; i < portal->max_recursion; i++)
	{
		portal->portal_recursive_views[i].views = NULL;
		portal->portal_recursive_views[i].views_count = 0;
		portal->portal_recursive_views[i].views_max = 0;
		portal->portal_recursive_views[i].frame = 0;
	}

	portal->position = position;
	portal->orientation = *orientation;
	portal->extents = extents;
	portal->linked_portal = -1;


	return portal_index;
}

portal_t *portal_GetPortalPointerIndex(int portal_index)
{
	/*if(portal_index >= 0 && portal_index < ptl_portal_list_cursor)
	{
		if(ptl_portals[portal_index].view)
		{
			return &ptl_portals[portal_index];
		}
	}*/

	return NULL;
}

void portal_DestroyPortal(portal_t *portal)
{

}

void portal_DestroyPortalIndex(int portal_index)
{
	int linked;
	if(portal_index >= 0 && portal_index < ptl_portal_list_cursor)
	{
		//if(ptl_portals[portal_index].view)
		{
		//	camera_DestroyCamera(ptl_portals[portal_index].view);
		//	ptl_portals[portal_index].view = NULL;

			if(ptl_portals[portal_index].linked_portal >= 0)
			{
				linked = ptl_portals[portal_index].linked_portal;
				ptl_portals[linked].linked_portal = -1;
			}

			ptl_portal_free_stack_top++;
			ptl_portal_free_stack[ptl_portal_free_stack_top] = portal_index;
		}
	}
}

void portal_LinkPortals(int portal0, int portal1)
{
	if((portal0 < 0 || portal0 >= ptl_portal_list_cursor) ||
	   (portal1 < 0 || portal1 >= ptl_portal_list_cursor))
	{
		return;
	}

	//if(!(ptl_portals[portal0].view && ptl_portals[portal1].view))
	//{
	//	return;
	//}

	if(ptl_portals[portal0].linked_portal > -1)
	{
		printf("portal_LinkPortals: portal0 is already linked. Old link destroyed.\n");
	}

	if(ptl_portals[portal1].linked_portal > -1)
	{
		printf("portal_LinkPortals: portal1 is already linked. Old link destroyed.\n");
	}

	ptl_portals[portal0].linked_portal = portal1;
	ptl_portals[portal1].linked_portal = portal0;
}

/*
==========================================================================
==========================================================================
==========================================================================
*/

void portal_TranslatePortal(int portal_index, vec3_t direction, float amount)
{
	portal_t *portal;

	if(portal_index >= 0 && portal_index < ptl_portal_list_cursor)
	{
		//if(ptl_portals[portal_index].view)
		{
			portal = &ptl_portals[portal_index];

			portal->position.x += direction.x * amount;
			portal->position.y += direction.y * amount;
			portal->position.z += direction.z * amount;
		}
	}
}

void portal_RotatePortal(int portal_index, vec3_t axis, float amount)
{
	portal_t *portal;

	vec3_t local_axis;

	if(portal_index >= 0 && portal_index < ptl_portal_list_cursor)
	{
		//if(ptl_portals[portal_index].view)
		{
			portal = &ptl_portals[portal_index];
			mat3_t_rotate(&portal->orientation, axis, amount, 0);
		}
	}
}

void portal_ScalePortal(int portal_index, vec2_t scale, float amount)
{
	portal_t *portal;

	if(portal_index >= 0 && portal_index < ptl_portal_list_cursor)
	{
		//if(ptl_portals[portal_index].view)
		{
			portal = &ptl_portals[portal_index];
			portal->extents.x += scale.x * amount;
			portal->extents.y += scale.y * amount;

			if(portal->linked_portal >= 0)
			{
				ptl_portals[portal->linked_portal].extents = portal->extents;
			}

		}
	}
}

/*
==========================================================================
==========================================================================
==========================================================================
*/

/*int portal_CalculateViewMatrix(camera_t *view, portal_t *portal)
{

}*/

int portal_CalculatePortalView(portal_t *portal, mat3_t *view_orientation, vec3_t view_position, int recursion_level, int viewing_portal_index)
{
    #if 0
	//int i;
	//camera_t *main_view = camera_GetMainViewCamera();
	camera_t *active_view = camera_GetActiveCamera();
	//camera_t *portal_view;

	//portal_t *portal;
	portal_t *linked_portal;

	//float fovy;

	/*float portal_view_yaw;
	float portal_view_pitch;*/

	//float portal_view_yaw_sin;
	//float portal_view_yaw_cos;

	//float portal_view_pitch_sin;
	//float portal_view_pitch_cos;

	portal_recursive_view_data_t *recursive_view_data;
	portal_view_data_t *view_data;

	mat3_t view_orientation_in_portal_space;
	//mat3_t portal_orientation_in_main_view_space;

	mat3_t portal_orientation;
	//mat3_t portal_view_orientation;
	mat3_t linked_portal_orientation;
	//mat3_t portal_view_local_orientation;
	//mat3_t portal_view_world_orientation;
	mat3_t main_view_orientation;
	//mat3_t main_view_space_portal_orientation;
	//mat3_t main_view_inverse_orientation;

	//mat3_t portal_view_pitch;
	//mat3_t portal_view_yaw;
	//mat3_t portal_view_pitch_yaw;

	//vec3_t portal_view_vector;
	//vec3_t main_view_forward_vector;

	vec3_t current_portal_up_vector;
	vec3_t current_portal_right_vector;
	vec3_t current_portal_forward_vector;
	vec3_t current_portal_view_offset;

	vec3_t linked_portal_up_vector;
	vec3_t linked_portal_right_vector;
	vec3_t linked_portal_forward_vector;
	vec3_t linked_portal_view_offset;
	vec4_t linked_portal_position;

	vec3_t view_portal_vec;

	//for(i = 0; i < ptl_portal_list_cursor; i++)
	//{
	//	if(!ptl_portals[i].view)
	//	{
	//		continue;
	//	}

	//	portal = &ptl_portals[i];
	portal->ignore = 0;

	//portal_view = portal->view;

	view_portal_vec.x = view_position.x - portal->position.x;
	view_portal_vec.y = view_position.y - portal->position.y;
	view_portal_vec.z = view_position.z - portal->position.z;

	current_portal_up_vector.x = portal->orientation.floats[0][1];
	current_portal_up_vector.y = portal->orientation.floats[1][1];
	current_portal_up_vector.z = portal->orientation.floats[2][1];

	current_portal_right_vector.x = portal->orientation.floats[0][0];
	current_portal_right_vector.y = portal->orientation.floats[1][0];
	current_portal_right_vector.z = portal->orientation.floats[2][0];

	current_portal_forward_vector.x = portal->orientation.floats[0][2];
	current_portal_forward_vector.y = portal->orientation.floats[1][2];
	current_portal_forward_vector.z = portal->orientation.floats[2][2];

	current_portal_view_offset.x = dot3(current_portal_right_vector, view_portal_vec);
	current_portal_view_offset.y = dot3(current_portal_up_vector, view_portal_vec);
	current_portal_view_offset.z = -dot3(current_portal_forward_vector, view_portal_vec);

	if(current_portal_view_offset.z < 0.0)
	{
			//portal_view->world_position.x = portal->position.x;
			//portal_view->world_position.y = portal->position.y;
			//portal_view->world_position.z = portal->position.z;
		portal->ignore = 1;
		return 0;
			//continue;
	}

		/* non-linked portals behave like mirrors... */
	if(portal->linked_portal < 0)
	{
		return 0;

			/*
			current_portal_view_offset.x = -current_portal_view_offset.x;

			portal_view->world_position.x = portal->position.x + current_portal_right_vector.x * current_portal_view_offset.x +
																 current_portal_up_vector.x * current_portal_view_offset.y +
																 current_portal_forward_vector.x * current_portal_view_offset.z;

			portal_view->world_position.y = portal->position.y + current_portal_right_vector.y * current_portal_view_offset.x +
																 current_portal_up_vector.y * current_portal_view_offset.y +
																 current_portal_forward_vector.y * current_portal_view_offset.z;

			portal_view->world_position.z = portal->position.z + current_portal_right_vector.z * current_portal_view_offset.x +
																 current_portal_up_vector.z * current_portal_view_offset.y +
																 current_portal_forward_vector.z * current_portal_view_offset.z;

			portal_orientation = portal->orientation;
			mat3_t_transpose(&portal_orientation);

			main_view_orientation = main_view->world_orientation;


			main_view_orientation.floats[0][0] = -main_view_orientation.floats[0][0];
			main_view_orientation.floats[1][0] = -main_view_orientation.floats[1][0];
			main_view_orientation.floats[2][0] = -main_view_orientation.floats[2][0];

			main_view_orientation.floats[0][2] = -main_view_orientation.floats[0][2];
			main_view_orientation.floats[1][2] = -main_view_orientation.floats[1][2];
			main_view_orientation.floats[2][2] = -main_view_orientation.floats[2][2];

			mat3_t_mult(&portal_view->world_orientation, &main_view_orientation, &portal_orientation);*/

	}


	recursive_view_data = &portal->portal_recursive_views[recursion_level];

	if(recursive_view_data->frame != r_frame)
	{
		recursive_view_data->views_count = 0;
		recursive_view_data->frame = r_frame;
	}

	if(recursive_view_data->views_count >= recursive_view_data->views_max)
	{
		view_data = memory_Malloc(sizeof(portal_view_data_t) * (recursive_view_data->views_max + 1));

		if(recursive_view_data->views)
		{
			memcpy(view_data, recursive_view_data->views, sizeof(portal_view_data_t) * recursive_view_data->views_max);
			memory_Free(recursive_view_data->views);
		}

		camera_InitViewData(&view_data[recursive_view_data->views_count].view_data);
		recursive_view_data->views = view_data;
		recursive_view_data->views_max++;
	}

	view_data = &recursive_view_data->views[recursive_view_data->views_count];
	recursive_view_data->views_count++;

	view_data->ref_portal_index = viewing_portal_index;




		//else
	//{
	linked_portal = &ptl_portals[portal->linked_portal];

	current_portal_view_offset.x = -current_portal_view_offset.x;
	//current_portal_view_offset.y = -current_portal_view_offset.y;

	linked_portal_up_vector.x = linked_portal->orientation.floats[0][1];
	linked_portal_up_vector.y = linked_portal->orientation.floats[1][1];
	linked_portal_up_vector.z = linked_portal->orientation.floats[2][1];

	linked_portal_right_vector.x = linked_portal->orientation.floats[0][0];
	linked_portal_right_vector.y = linked_portal->orientation.floats[1][0];
	linked_portal_right_vector.z = linked_portal->orientation.floats[2][0];

	linked_portal_forward_vector.x = linked_portal->orientation.floats[0][2];
	linked_portal_forward_vector.y = linked_portal->orientation.floats[1][2];
	linked_portal_forward_vector.z = linked_portal->orientation.floats[2][2];




	view_data->position.x = linked_portal->position.x + linked_portal_right_vector.x * current_portal_view_offset.x +
																linked_portal_up_vector.x * current_portal_view_offset.y +
																linked_portal_forward_vector.x * current_portal_view_offset.z;

	view_data->position.y = linked_portal->position.y + linked_portal_right_vector.y * current_portal_view_offset.x +
																linked_portal_up_vector.y * current_portal_view_offset.y +
																linked_portal_forward_vector.y * current_portal_view_offset.z;

	view_data->position.z = linked_portal->position.z + linked_portal_right_vector.z * current_portal_view_offset.x +
																linked_portal_up_vector.z * current_portal_view_offset.y +
																linked_portal_forward_vector.z * current_portal_view_offset.z;

	/**********************************************************************/
	/* what the fuck is going on here? */
	/**********************************************************************/

	/* rotate the main view orientation by the source portal orientation... */
	main_view_orientation = *view_orientation;
	mat3_t_mult(&view_orientation_in_portal_space, &main_view_orientation, &portal->orientation);

	/* "rotate" it around the y axis... */
	view_orientation_in_portal_space.floats[0][0] = -view_orientation_in_portal_space.floats[0][0];
	view_orientation_in_portal_space.floats[1][0] = -view_orientation_in_portal_space.floats[1][0];
	view_orientation_in_portal_space.floats[2][0] = -view_orientation_in_portal_space.floats[2][0];

	view_orientation_in_portal_space.floats[0][2] = -view_orientation_in_portal_space.floats[0][2];
	view_orientation_in_portal_space.floats[1][2] = -view_orientation_in_portal_space.floats[1][2];
	view_orientation_in_portal_space.floats[2][2] = -view_orientation_in_portal_space.floats[2][2];

	/* multiply the resulting rotation by the inverse of the destination portal... */
	linked_portal_orientation = linked_portal->orientation;
	mat3_t_transpose(&linked_portal_orientation);
	mat3_t_mult(&view_data->orientation, &view_orientation_in_portal_space, &linked_portal_orientation);
	/**********************************************************************/
	/* oh lord, what the actual fuck is going on here? */
	/**********************************************************************/

	mat4_t_compose(&view_data->view_data.view_matrix, &view_data->orientation, view_data->position);
	mat4_t_inverse_transform(&view_data->view_data.view_matrix);
	view_data->view_data.projection_matrix = active_view->view_data.projection_matrix;

	linked_portal_position.x = linked_portal->position.x;
	linked_portal_position.y = linked_portal->position.y;
	linked_portal_position.z = linked_portal->position.z;
	linked_portal_position.w = 1.0;

	view_data->near_plane.x = linked_portal_forward_vector.x;
	view_data->near_plane.y = linked_portal_forward_vector.y;
	view_data->near_plane.z = linked_portal_forward_vector.z;
	view_data->near_plane.w = 0.0;

	mat4_t_vec4_t_mult(&view_data->view_data.view_matrix, &view_data->near_plane);
	mat4_t_vec4_t_mult(&view_data->view_data.view_matrix, &linked_portal_position);

	view_data->near_plane.w = -dot3(linked_portal_position.vec3, view_data->near_plane.vec3);


	return 1;

	#endif
}

void portal_UpdatePortals()
{
	#if 0
	int i;
	camera_t *main_view = camera_GetMainViewCamera();
	camera_t *portal_view;

	portal_t *portal;
	portal_t *linked_portal;

	float fovy;

	/*float portal_view_yaw;
	float portal_view_pitch;*/

	float portal_view_yaw_sin;
	float portal_view_yaw_cos;

	float portal_view_pitch_sin;
	float portal_view_pitch_cos;

	mat3_t view_orientation_in_portal_space;
	mat3_t portal_orientation_in_main_view_space;

	mat3_t portal_orientation;
	mat3_t portal_view_orientation;
	mat3_t linked_portal_orientation;
	//mat3_t portal_view_local_orientation;
	//mat3_t portal_view_world_orientation;
	mat3_t main_view_orientation;

	//mat3_t main_view_space_portal_orientation;
	//mat3_t main_view_inverse_orientation;

	mat3_t portal_view_pitch;
	mat3_t portal_view_yaw;
	mat3_t portal_view_pitch_yaw;

	vec3_t portal_view_vector;
	vec3_t main_view_forward_vector;

	vec3_t current_portal_up_vector;
	vec3_t current_portal_right_vector;
	vec3_t current_portal_forward_vector;
	vec3_t current_portal_view_offset;

	vec3_t linked_portal_up_vector;
	vec3_t linked_portal_right_vector;
	vec3_t linked_portal_forward_vector;
	vec3_t linked_portal_view_offset;

	vec3_t view_portal_vec;

	for(i = 0; i < ptl_portal_list_cursor; i++)
	{
		if(!ptl_portals[i].view)
		{
			continue;
		}

		portal = &ptl_portals[i];
		portal->ignore = 0;

		portal_view = portal->view;

		view_portal_vec.x = main_view->world_position.x - portal->position.x;
		view_portal_vec.y = main_view->world_position.y - portal->position.y;
		view_portal_vec.z = main_view->world_position.z - portal->position.z;

		current_portal_up_vector.x = portal->orientation.floats[0][1];
		current_portal_up_vector.y = portal->orientation.floats[1][1];
		current_portal_up_vector.z = portal->orientation.floats[2][1];

		current_portal_right_vector.x = portal->orientation.floats[0][0];
		current_portal_right_vector.y = portal->orientation.floats[1][0];
		current_portal_right_vector.z = portal->orientation.floats[2][0];

		current_portal_forward_vector.x = portal->orientation.floats[0][2];
		current_portal_forward_vector.y = portal->orientation.floats[1][2];
		current_portal_forward_vector.z = portal->orientation.floats[2][2];

		current_portal_view_offset.x = dot3(current_portal_right_vector, view_portal_vec);
		current_portal_view_offset.y = dot3(current_portal_up_vector, view_portal_vec);
		current_portal_view_offset.z = -dot3(current_portal_forward_vector, view_portal_vec);

		if(current_portal_view_offset.z < 0.0)
		{
			portal_view->world_position.x = portal->position.x;
			portal_view->world_position.y = portal->position.y;
			portal_view->world_position.z = portal->position.z;
			portal->ignore = 1;
			continue;
		}

		/* non-linked portals behave like mirrors... */
		if(portal->linked_portal < 0)
		{
			current_portal_view_offset.x = -current_portal_view_offset.x;
			//current_portal_view_offset.y = -current_portal_view_offset.y;

			portal_view->world_position.x = portal->position.x + current_portal_right_vector.x * current_portal_view_offset.x +
																 current_portal_up_vector.x * current_portal_view_offset.y +
																 current_portal_forward_vector.x * current_portal_view_offset.z;

			portal_view->world_position.y = portal->position.y + current_portal_right_vector.y * current_portal_view_offset.x +
																 current_portal_up_vector.y * current_portal_view_offset.y +
																 current_portal_forward_vector.y * current_portal_view_offset.z;

			portal_view->world_position.z = portal->position.z + current_portal_right_vector.z * current_portal_view_offset.x +
																 current_portal_up_vector.z * current_portal_view_offset.y +
																 current_portal_forward_vector.z * current_portal_view_offset.z;

			//portal_view->world_orientation = portal->orientation;

			portal_orientation = portal->orientation;
			mat3_t_transpose(&portal_orientation);

			main_view_orientation = main_view->world_orientation;


			/*portal_view_vector.x = main_view_orientation.floats[0][0];
			portal_view_vector.y = main_view_orientation.floats[1][0];
			portal_view_vector.z = main_view_orientation.floats[2][0];

			main_view_orientation.floats[0][0] = main_view_orientation.floats[0][2];
			main_view_orientation.floats[1][0] = main_view_orientation.floats[1][2];
			main_view_orientation.floats[2][0] = main_view_orientation.floats[2][2];

			main_view_orientation.floats[0][2] = portal_view_vector.x;
			main_view_orientation.floats[1][2] = portal_view_vector.y;
			main_view_orientation.floats[2][2] = portal_view_vector.z;*/

			main_view_orientation.floats[0][0] = -main_view_orientation.floats[0][0];
			main_view_orientation.floats[1][0] = -main_view_orientation.floats[1][0];
			main_view_orientation.floats[2][0] = -main_view_orientation.floats[2][0];

			main_view_orientation.floats[0][2] = -main_view_orientation.floats[0][2];
			main_view_orientation.floats[1][2] = -main_view_orientation.floats[1][2];
			main_view_orientation.floats[2][2] = -main_view_orientation.floats[2][2];

			mat3_t_mult(&portal_view->world_orientation, &main_view_orientation, &portal_orientation);

		}
		else
		{
			linked_portal = &ptl_portals[portal->linked_portal];

			current_portal_view_offset.x = -current_portal_view_offset.x;
			//current_portal_view_offset.y = -current_portal_view_offset.y;

			linked_portal_up_vector.x = linked_portal->orientation.floats[0][1];
			linked_portal_up_vector.y = linked_portal->orientation.floats[1][1];
			linked_portal_up_vector.z = linked_portal->orientation.floats[2][1];

			linked_portal_right_vector.x = linked_portal->orientation.floats[0][0];
			linked_portal_right_vector.y = linked_portal->orientation.floats[1][0];
			linked_portal_right_vector.z = linked_portal->orientation.floats[2][0];

			linked_portal_forward_vector.x = linked_portal->orientation.floats[0][2];
			linked_portal_forward_vector.y = linked_portal->orientation.floats[1][2];
			linked_portal_forward_vector.z = linked_portal->orientation.floats[2][2];




			portal_view->world_position.x = linked_portal->position.x + linked_portal_right_vector.x * current_portal_view_offset.x +
																		linked_portal_up_vector.x * current_portal_view_offset.y +
																		linked_portal_forward_vector.x * current_portal_view_offset.z;

			portal_view->world_position.y = linked_portal->position.y + linked_portal_right_vector.y * current_portal_view_offset.x +
																		linked_portal_up_vector.y * current_portal_view_offset.y +
																		linked_portal_forward_vector.y * current_portal_view_offset.z;

			portal_view->world_position.z = linked_portal->position.z + linked_portal_right_vector.z * current_portal_view_offset.x +
																		linked_portal_up_vector.z * current_portal_view_offset.y +
																		linked_portal_forward_vector.z * current_portal_view_offset.z;

			/**********************************************************************/
			/* what the actual fuck is going on here? */
			/**********************************************************************/

			/* rotate the main view orientation by the source portal orientation... */
			main_view_orientation = main_view->world_orientation;
			mat3_t_mult(&view_orientation_in_portal_space, &main_view_orientation, &portal->orientation);

			/* "rotate" it around the y axis... */
			view_orientation_in_portal_space.floats[0][0] = -view_orientation_in_portal_space.floats[0][0];
			view_orientation_in_portal_space.floats[1][0] = -view_orientation_in_portal_space.floats[1][0];
			view_orientation_in_portal_space.floats[2][0] = -view_orientation_in_portal_space.floats[2][0];

			view_orientation_in_portal_space.floats[0][2] = -view_orientation_in_portal_space.floats[0][2];
			view_orientation_in_portal_space.floats[1][2] = -view_orientation_in_portal_space.floats[1][2];
			view_orientation_in_portal_space.floats[2][2] = -view_orientation_in_portal_space.floats[2][2];

			/* multiply the resulting rotation by the inverse of the destination portal... */
			linked_portal_orientation = linked_portal->orientation;
			mat3_t_transpose(&linked_portal_orientation);
			mat3_t_mult(&portal_view->world_orientation, &view_orientation_in_portal_space, &linked_portal_orientation);

			/**********************************************************************/
			/* oh lord, what the actual fuck is going on here? */
			/**********************************************************************/
		}


		//fovy = atan((portal->extents.y / current_portal_view_offset.z));
		//portal_view->x_shift = -current_portal_view_offset.x;
		//portal_view->y_shift = current_portal_view_offset.y;

		//portal_view->x_shift = 0.0;
		//portal_view->y_shift = 0.0;
		//portal_view->y_shift = 0;

		//if(fovy > main_view->fov_y)
		//{
		//	fovy = main_view->fov_y;
		//}

		//printf("[%f %f %f %f]\n", fovy, portal_view->x_shift, portal_view->y_shift, current_portal_view_offset.z);

		CreatePerspectiveMatrix(&portal_view->projection_matrix, main_view->fov_y, (float)r_width / (float)r_height, 0.1, 500.0, 0.0, 0.0, &portal_view->frustum);
		//Frustum(&portal_view->projection_matrix, -portal->extents.x + portal_view->x_shift * 0.5, portal->extents.x + portal_view->x_shift * 0.5, portal->extents.y + portal_view->y_shift * 0.5, -portal->extents.y + portal_view->y_shift * 0.5, current_portal_view_offset.z, 500.0, &portal_view->frustum);
		camera_ComputeWorldToCameraMatrix(portal_view);

	/*	portal_view->world_to_camera_matrix.floats[0][0] = -portal_view->world_to_camera_matrix.floats[0][0];
		portal_view->world_to_camera_matrix.floats[0][1] = -portal_view->world_to_camera_matrix.floats[0][1];
		portal_view->world_to_camera_matrix.floats[0][2] = -portal_view->world_to_camera_matrix.floats[0][2];*/

	}

	#endif
}


#ifdef __cplusplus
}
#endif






