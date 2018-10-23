#include "r_debug.h"
#include "r_main.h"
#include "r_imediate.h"
#include "r_shader.h"
#include "camera.h"
#include "c_memory.h"
#include "navigation.h"
#include "portal.h"
#include "physics.h"
#include "engine.h"

#include "gui.h"

#include "l_common.h"

#include "entity.h"

#include "SDL2\SDL.h"
#include "GL\glew.h"

#include <stdio.h>
#include <string.h>

#include "stack_list.h"


/* from portal.c */
extern int ptl_portal_list_cursor;
extern portal_t *ptl_portals;

/* from physics.c */
extern struct stack_list_t phy_colliders[COLLIDER_TYPE_LAST];

/* from entity.c */
extern int ent_entity_list_cursor;
extern struct stack_list_t ent_entities[2];
extern struct stack_list_t ent_triggers;
extern struct entity_transform_t *ent_global_transforms;
extern struct stack_list_t ent_entity_aabbs;

/* from navigation.c */
extern struct stack_list_t nav_waypoints;


/* from light.c */
//extern int l_light_list_cursor;
//extern light_position_t *l_light_positions;
//extern light_params_t *l_light_params;

extern struct stack_list_t l_light_positions;
extern struct stack_list_t l_light_params;
extern struct stack_list_t l_light_clusters;



/*int r_dbg_debug_cmd_count = 0;
int r_dbg_max_debug_cmd_count = 0;*/

int r_dbg_max_dbg_cmds = 0;
int r_dbg_debug_cmd_next_in = 0;
int r_dbg_debug_cmd_next_out = 0;
dbg_command_t *r_dbg_debug_cmds = NULL;

/*int r_dbg_draw_bytes_count = 0;
int r_dbg_max_draw_bytes = 0;*/

int r_dbg_max_draw_bytes = 0;
int r_dbg_draw_bytes_next_in = 0;
int r_dbg_draw_bytes_next_out = 0;
void *r_dbg_draw_bytes = NULL;

extern int r_debug;
int r_debug_verbose = 0;
int r_debug_draw_portal_outlines = 0;
int r_debug_draw_views = 0;
int r_debug_draw_waypoints = 1;
int r_debug_draw_colliders = 0;
int r_debug_draw_entities = 0;
int r_debug_draw_triggers = 0;
int r_debug_draw_lights = 0;
int r_debug_draw_lights_screen_bounds = 0;
int r_debug_draw_clusters = 0;
int r_debug_draw_bsp_leaves = 0;
int r_debug_draw_statistics = 0;

extern unsigned int r_visible_lights_count;
extern unsigned int r_visible_lights[];

extern unsigned int r_visible_leaves_count;
extern struct bsp_dleaf_t *r_visible_leaves;



/* from r_imediate.c */
extern int r_imediate_color_shader;
int r_cluster_debug_shader;

extern struct framebuffer_t r_cbuffer;
extern int r_clusters_per_row;
extern int r_cluster_rows;
extern int r_cluster_layers;
extern int r_window_width;
extern int r_window_height;
extern int r_width;
extern int r_height;
extern int r_frame;

extern unsigned int r_draw_calls;
extern unsigned int r_material_swaps;
extern unsigned int r_shader_swaps;
extern unsigned int r_shader_uniform_updates;
extern unsigned int r_frame_vert_count;


/* from camera.c */
extern camera_t *cameras;


#define CHARACTER_COLLIDER_CAPSULE_SEGMENTS 16
vec3_t r_collider_capsule_shape[3][CHARACTER_COLLIDER_CAPSULE_SEGMENTS];





#define R_DBG_FUNCTION_NAME_STACK_DEPTH 512
#define R_DBG_FUNCTION_NAME_MAX_NAME_LEN 64

static int r_dbg_function_names_stack_top = -1;
static char r_dbg_function_names[R_DBG_FUNCTION_NAME_STACK_DEPTH][R_DBG_FUNCTION_NAME_MAX_NAME_LEN];


#define R_DEBUG_MAX_TIMERS 32

int r_timers_count = 0;
struct debug_timer_t r_timers[R_DEBUG_MAX_TIMERS];
unsigned int r_timestamp_query = 0;
//unsigned int r_query_objects[R_DEBUG_MAX_TIMERS];


__stdcall void renderer_GLReportCallback(GLenum source, GLenum type, GLenum id, GLenum severety, GLsizei length, const GLchar *message, void *parm)
{
	//printf("renderer_GLReportCallback: %s\n", message);
	int i;
	int j;
	printf("*******************************\n");

	switch(type)
	{
		case GL_DEBUG_TYPE_ERROR:
			printf("ERROR <");
		break;

		case GL_DEBUG_TYPE_PERFORMANCE:
			printf("PERFORMANCE <");
		break;

		case GL_DEBUG_TYPE_MARKER:
		case GL_DEBUG_TYPE_OTHER:
			printf("NOTIFY <");
		break;
	}

	switch(severety)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			printf("HIGH SEVERITY>: ");
		break;

		case GL_DEBUG_SEVERITY_MEDIUM:
			printf("MEDIUM SEVERITY>: ");
		break;

		case GL_DEBUG_SEVERITY_LOW:
			printf("LOW SEVERITY>: ");
		break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
			printf("NOTIFICATION>: ");
		break;
	}

	printf("%s\n", message);

	if(r_dbg_function_names_stack_top >= 0)
	{
		printf("callstack:\n---\n");

		for(i = 0; i <= r_dbg_function_names_stack_top; i++)
		{
			for(j = 0; j < i; j++)
			{
				putchar(' ');
			}

			printf("%s\n", r_dbg_function_names[i]);
		}
	}

	printf("*******************************\n");
}

#ifdef __cplusplus
extern "C"
{
#endif


void renderer_InitDebug()
{
	int i;

	float angle_increment = (3.14159265 * 2.0) / CHARACTER_COLLIDER_CAPSULE_SEGMENTS;
	float current_angle = 0.0;

	for(i = 0; i < CHARACTER_COLLIDER_CAPSULE_SEGMENTS; i++)
	{
		r_collider_capsule_shape[0][i].x = cos(current_angle);
		r_collider_capsule_shape[0][i].y = sin(current_angle);
		r_collider_capsule_shape[0][i].z = 0.0;

		current_angle += angle_increment;
	}

	for(i = 0; i < CHARACTER_COLLIDER_CAPSULE_SEGMENTS; i++)
	{
		r_collider_capsule_shape[1][i].x = 0.0;
		r_collider_capsule_shape[1][i].y = r_collider_capsule_shape[0][i].y;
		r_collider_capsule_shape[1][i].z = r_collider_capsule_shape[0][i].x;
	}

	for(i = 0; i < CHARACTER_COLLIDER_CAPSULE_SEGMENTS; i++)
	{
		r_collider_capsule_shape[2][i].x = r_collider_capsule_shape[0][i].y;
		r_collider_capsule_shape[2][i].y = 0.0;
		r_collider_capsule_shape[2][i].z = r_collider_capsule_shape[0][i].x;
	}

	r_dbg_max_dbg_cmds = 2048;
	r_dbg_debug_cmds = memory_Malloc(sizeof(dbg_command_t) * r_dbg_max_dbg_cmds);

	r_dbg_max_draw_bytes = 4 << 16;
	r_dbg_draw_bytes = memory_Malloc(r_dbg_max_draw_bytes);

	renderer_VerboseDebugOutput(0);

	//glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallbackARB((GLDEBUGPROC)renderer_GLReportCallback, 0);

    glGenQueries(1, &r_timestamp_query);
    //glGenQueries(1, &r_query_object);

	/*for(i = 0; i < R_DEBUG_MAX_TIMERS; i++)
    {
        r_timers[i].running = 0;
    }*/

}

void renderer_FinishDebug()
{
	memory_Free(r_dbg_debug_cmds);
	memory_Free(r_dbg_draw_bytes);
}

void renderer_Debug(int enable, int verbose)
{
	r_debug = enable && 1;

	if(verbose)
	{
		r_debug_verbose = 1;
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
	else
	{
		r_debug_verbose = 0;
		glDisable(GL_DEBUG_OUTPUT);
		glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
}

void renderer_VerboseDebugOutput(int enable)
{
	if(enable)
	{
		r_debug_verbose = 1;
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
	else
	{
		r_debug_verbose = 0;
		glDisable(GL_DEBUG_OUTPUT);
		glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
}


dbg_command_t *renderer_AllocDebugCommand(int size)
{
	dbg_command_t *cmd;
	void *bytes;
	int r;

	if(r_dbg_debug_cmd_next_in == r_dbg_debug_cmd_next_out - 1)
	{
		return NULL;
	}

	size = (size + 3) & (~(3));

	r = r_dbg_draw_bytes_next_in + size;

	if(r >= r_dbg_max_draw_bytes)
	{
		r %= r_dbg_max_draw_bytes;

		if(r + size >= r_dbg_draw_bytes_next_out)
		{
			return NULL;
		}

		r_dbg_draw_bytes_next_in = r;
	}
	else if(r_dbg_draw_bytes_next_in == r_dbg_draw_bytes_next_out - 4)
	{
		return NULL;
	}

	cmd = r_dbg_debug_cmds + r_dbg_debug_cmd_next_in;
	r_dbg_debug_cmd_next_in = (r_dbg_debug_cmd_next_in + 1) % r_dbg_max_dbg_cmds;
	bytes = (char *)r_dbg_draw_bytes + r_dbg_draw_bytes_next_in;
	r_dbg_draw_bytes_next_in += size;

	cmd->data = bytes;
	cmd->type = 0;
	cmd->persistent = 0;

	return cmd;
}

void renderer_DrawPoint(vec3_t position, vec3_t color, float size, int smooth, int persistent, int depth_test)
{
	dbg_command_t *cmd;
	point_dbg_draw_data_t *data;

	cmd = renderer_AllocDebugCommand(sizeof(point_dbg_draw_data_t));

	if(cmd)
	{
		cmd->type = DEBUG_COMMAND_TYPE_DRAW_POINT;

		data = cmd->data;
		data->position = position;
		data->color = color;
		data->size = size;
		data->smooth = smooth;
		data->draw_params.depth_test = depth_test;
		if(persistent) cmd->persistent = 1;
	}
}

void renderer_DrawLine(vec3_t from, vec3_t to, vec3_t color, float width, int persistent)
{
	dbg_command_t *cmd;
	line_dbg_draw_data_t *data;
	cmd = renderer_AllocDebugCommand(sizeof(line_dbg_draw_data_t));

	if(cmd)
	{
		cmd->type = DEBUG_COMMAND_TYPE_DRAW_LINE;

		data = cmd->data;
		data->from = from;
		data->to = to;
		data->color = color;
		data->width = width;
		if(persistent) cmd->persistent = 1;
	}
}

void renderer_Draw2dLine(vec2_t from, vec2_t to, vec3_t color, float width, int persistent)
{
	dbg_command_t *cmd;
	line_2d_dbg_draw_data_t *data;
	cmd = renderer_AllocDebugCommand(sizeof(line_2d_dbg_draw_data_t));

	if(cmd)
	{
		cmd->type = DEBUG_COMMAND_TYPE_DRAW_2D_LINE;

		data = cmd->data;
		data->from = from;
		data->to = to;
		data->color = color;
		data->width = width;
		if(persistent) cmd->persistent = 1;
	}
}

/*
==============================================================
==============================================================
==============================================================
*/

unsigned int renderer_GetTimeStamp()
{
    unsigned int timestamp;
    glQueryCounter(r_timestamp_query, GL_TIMESTAMP);
    glGetQueryObjectiv(r_timestamp_query, GL_QUERY_RESULT, &timestamp);
    return timestamp;
}

int renderer_StartGpuTimer(const char *timer_name)
{
    return renderer_StartTimer(timer_name, 1);
}

int renderer_StartCpuTimer(const char *timer_name)
{
    return renderer_StartTimer(timer_name, 0);
}

int renderer_StartTimer(const char *timer_name, int gpu_timer)
{
    int timer_index = -1;

    struct debug_timer_t *timer;

    R_DBG_PUSH_FUNCTION_NAME();

    if(r_debug)
    {
        if(r_timers_count < R_DEBUG_MAX_TIMERS)
        {
            timer_index = r_timers_count;
            timer = &r_timers[timer_index];
            r_timers_count++;

            timer->name = timer_name;
            timer->frame = r_frame;
            timer->gpu_timer = gpu_timer && 1;

            if(gpu_timer)
            {
                //glBeginQueryIndexed(GL_TIME_ELAPSED, timer_index, r_query_objects[timer_index]);
                timer->start_time_stamp = renderer_GetTimeStamp();
            }
            else
            {
                timer->start_time = engine_GetCurrentDeltaTime();
            }
        }
    }

    R_DBG_POP_FUNCTION_NAME();

    return timer_index;
}

void renderer_StopTimer(int timer_index)
{
    struct debug_timer_t *timer;
    unsigned long timestamp = 0;

    R_DBG_PUSH_FUNCTION_NAME();

    if(r_debug)
    {
        if(timer_index >= 0 && timer_index < R_DEBUG_MAX_TIMERS)
        {
            timer = &r_timers[timer_index];

            if(timer->frame == r_frame)
            {
                if(timer->gpu_timer)
                {
                    timestamp = renderer_GetTimeStamp();
                    timer->delta_time = (float)(timestamp - timer->start_time_stamp) / 1000000.0;
                    //glEndQueryIndexed(GL_TIME_ELAPSED, timer_index);
                    //glGetQueryObjectiv(r_query_objects[timer_index], GL_QUERY_RESULT, &elapsed);
                    //timer->delta_time = (float)elapsed /1000000.0;
                }
                else
                {
                    timer->delta_time = engine_GetCurrentDeltaTime() - timer->start_time;
                }

                timer->frame = -1;
            }
        }
    }

    R_DBG_POP_FUNCTION_NAME();
}

void renderer_StopAllTimers()
{
    int i;

    for(i = 0; i < R_DEBUG_MAX_TIMERS; i++)
    {
        //renderer_StopTimer(i);
    }

    r_timers_count = 0;
}


/*
==============================================================
==============================================================
==============================================================
*/



void renderer_StopTimer(int timer_index);

void renderer_DrawBox()
{
	renderer_Begin(GL_QUADS);

	/* top */
	renderer_Vertex3f(-1.0, 1.0,-1.0);
	renderer_Vertex3f(-1.0, 1.0, 1.0);
	renderer_Vertex3f( 1.0, 1.0, 1.0);
	renderer_Vertex3f( 1.0, 1.0,-1.0);

	/* bottom */
	renderer_Vertex3f(-1.0,-1.0, 1.0);
	renderer_Vertex3f(-1.0,-1.0,-1.0);
	renderer_Vertex3f( 1.0,-1.0,-1.0);
	renderer_Vertex3f( 1.0,-1.0, 1.0);

	/* left */
	renderer_Vertex3f(-1.0,-1.0,-1.0);
	renderer_Vertex3f(-1.0,-1.0, 1.0);
	renderer_Vertex3f(-1.0, 1.0, 1.0);
	renderer_Vertex3f(-1.0, 1.0,-1.0);

	/* right */
	renderer_Vertex3f( 1.0, 1.0,-1.0);
	renderer_Vertex3f( 1.0, 1.0, 1.0);
	renderer_Vertex3f( 1.0,-1.0, 1.0);
	renderer_Vertex3f( 1.0,-1.0,-1.0);

	/* front */
	renderer_Vertex3f(-1.0, 1.0, 1.0);
	renderer_Vertex3f(-1.0,-1.0, 1.0);
	renderer_Vertex3f( 1.0,-1.0, 1.0);
	renderer_Vertex3f( 1.0, 1.0, 1.0);

	/* back */
	renderer_Vertex3f(-1.0,-1.0,-1.0);
	renderer_Vertex3f(-1.0, 1.0,-1.0);
	renderer_Vertex3f( 1.0, 1.0,-1.0);
	renderer_Vertex3f( 1.0,-1.0,-1.0);

	renderer_End();
}


vec3_t verts[1024];

void renderer_DrawCylinder(int base_verts, float radius, float height, int outline)
{
	int i;
	int j;
	vec3_t vert;

	float current_angle;
	float angle_increment;

	int top_count = 0;
	int middle_count = 0;

	int mode;


	if(base_verts > 32)
	{
		base_verts = 32;
	}
	else if(base_verts < 0)
	{
		base_verts = 8;
	}

	current_angle = 0.0;
	angle_increment = (3.14159265 * 2.0) / (float)base_verts;


	//if(outline)
	//{
	for(top_count = 0; top_count < base_verts; top_count++)
	{
		verts[top_count].x = cos(current_angle) * radius;
		verts[top_count].y = height * 0.5;
		verts[top_count].z = -sin(current_angle) * radius;
		current_angle += angle_increment;
	}
	//}
/*	else
	{
		for(top_count = 0; top_count < base_verts * 3;)
		{
			verts[top_count].x = cos(current_angle) * radius;
			verts[top_count].y = height * 0.5;
			verts[top_count].z = sin(current_angle) * radius;
			top_count++;

			current_angle += angle_increment;

			verts[top_count].x = cos(current_angle) * radius;
			verts[top_count].y = height * 0.5;
			verts[top_count].z = sin(current_angle) * radius;
			top_count++;

			verts[top_count].x = 0.0;
			verts[top_count].y = height * 0.5;
			verts[top_count].z = 0.0;
			top_count++;
		}
	}*/

	base_verts <<= 2;

	for(middle_count = 0, j = 0; middle_count < base_verts;)
	{
		verts[top_count + middle_count].x = verts[j].x;
		verts[top_count + middle_count].y = height * 0.5;
		verts[top_count + middle_count].z = verts[j].z;

		middle_count++;

		verts[top_count + middle_count].x = verts[j].x;
		verts[top_count + middle_count].y = -height * 0.5;
		verts[top_count + middle_count].z = verts[j].z;

		middle_count++;

		j++;

		verts[top_count + middle_count].x = verts[j].x;
		verts[top_count + middle_count].y = -height * 0.5;
		verts[top_count + middle_count].z = verts[j].z;

		middle_count++;

		verts[top_count + middle_count].x = verts[j].x;
		verts[top_count + middle_count].y = height * 0.5;
		verts[top_count + middle_count].z = verts[j].z;

		middle_count++;
	}




	if(outline)
	{
		mode = GL_LINE_LOOP;
	}
	else
	{
		mode = GL_TRIANGLE_FAN;
	}

	renderer_Begin(mode);
	for(i = 0; i < top_count; i++)
	{
		renderer_Vertex3f(verts[i].x, verts[i].y, verts[i].z);
	}
	renderer_End();

	renderer_Begin(mode);
	for(i = 0; i < top_count; i++)
	{
		renderer_Vertex3f(-verts[i].x, -verts[i].y, verts[i].z);
	}
	renderer_End();



	renderer_Begin(GL_QUADS);
	for(i = 0; i < middle_count; i++)
	{
		renderer_Vertex3f(verts[top_count + i].x, verts[top_count + i].y, verts[top_count + i].z);
	}
	renderer_End();

}

void renderer_DrawSphere(float radius, int sub_divs)
{

}

/*
==============================================================
==============================================================
==============================================================
*/

void renderer_DrawPortalsOulines()
{
    #if 0
	int i;
	int j;
	int k;
	camera_t *active_view;
	camera_t *portal_view;

	portal_t *portal;
	portal_t *linked_portal;

	portal_view_data_t *view_data;
	portal_recursive_view_data_t *recursive_view_data;

	vec3_t up_vector;
	vec3_t right_vector;
	vec3_t forward_vector;
	vec3_t center;

	vec4_t world_space_near_plane_direction;
	vec4_t world_space_near_plane_point;
	mat4_t view_to_world_matrix;

	vec3_t fcenter;
	vec3_t ncenter;
	vec3_t fright;
	vec3_t fleft;
	vec3_t ftop;
	vec3_t fbottom;
	vec3_t fup;

	float fr;
	float fl;
	float ft;
	float fb;

	float x_shift;
	float y_shift;

	active_view = camera_GetActiveCamera();


	//renderer_SetShader(r_imediate_color_shader);
	//renderer_EnableImediateDrawing();

	//renderer_SetProjectionMatrix(&active_view->projection_matrix);
	//renderer_SetViewMatrix(&active_view->world_to_camera_matrix);
	//renderer_SetModelMatrix(NULL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	glPointSize(8.0);

	for(i = 0; i < ptl_portal_list_cursor; i++)
	{
		/*if(!ptl_portals[i].view)
		{
			continue;
		}*/

		portal = &ptl_portals[i];

		right_vector.x = portal->orientation.floats[0][0];
		right_vector.y = portal->orientation.floats[1][0];
		right_vector.z = portal->orientation.floats[2][0];

		up_vector.x = portal->orientation.floats[0][1];
		up_vector.y = portal->orientation.floats[1][1];
		up_vector.z = portal->orientation.floats[2][1];

		forward_vector.x = portal->orientation.floats[0][2];
		forward_vector.y = portal->orientation.floats[1][2];
		forward_vector.z = portal->orientation.floats[2][2];

		center = portal->position;

		/*renderer_Begin(GL_LINES);

		renderer_Color3f(1.0, 0.0, 0.0);
		renderer_Vertex3f(center.x, center.y, center.z);
		renderer_Vertex3f(center.x + right_vector.x, center.y + right_vector.y, center.z + right_vector.z);

		renderer_Color3f(0.0, 1.0, 0.0);
		renderer_Vertex3f(center.x, center.y, center.z);
		renderer_Vertex3f(center.x + up_vector.x, center.y + up_vector.y, center.z + up_vector.z);

		renderer_Color3f(0.0, 0.0, 1.0);
		renderer_Vertex3f(center.x, center.y, center.z);
		renderer_Vertex3f(center.x + forward_vector.x, center.y + forward_vector.y, center.z + forward_vector.z);

		renderer_End();*/

		/*********************************************************************************************************/


		linked_portal = &ptl_portals[portal->linked_portal];

		right_vector.x *= portal->extents.x + 0.02;
		right_vector.y *= portal->extents.x + 0.02;
		right_vector.z *= portal->extents.x + 0.02;

		up_vector.x *= portal->extents.y + 0.02;
		up_vector.y *= portal->extents.y + 0.02;
		up_vector.z *= portal->extents.y + 0.02;

		glLineWidth(1.0);
		glEnable(GL_BLEND);
		renderer_Begin(GL_QUADS);
		renderer_Color4f(0.2, 0.8, 0.2, 0.5);
		renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);
		renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);
		renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);
		renderer_End();
		glDisable(GL_BLEND);
		glLineWidth(1.0);

		/*right_vector.x = linked_portal->orientation.floats[0][0];
		right_vector.y = linked_portal->orientation.floats[1][0];
		right_vector.z = linked_portal->orientation.floats[2][0];

		up_vector.x = linked_portal->orientation.floats[0][1];
		up_vector.y = linked_portal->orientation.floats[1][1];
		up_vector.z = linked_portal->orientation.floats[2][1];

		forward_vector.x = linked_portal->orientation.floats[0][2];
		forward_vector.y = linked_portal->orientation.floats[1][2];
		forward_vector.z = linked_portal->orientation.floats[2][2];

		right_vector.x *= linked_portal->extents.x;
		right_vector.y *= linked_portal->extents.x;
		right_vector.z *= linked_portal->extents.x;

		up_vector.x *= linked_portal->extents.y;
		up_vector.y *= linked_portal->extents.y;
		up_vector.z *= linked_portal->extents.y;

		center = linked_portal->position;

		for(j = 0; j < W_MAX_PORTAL_RECURSION_LEVEL; j++)
		{
			recursive_view_data = &portal->portal_recursive_views[j];

			if(recursive_view_data->frame != r_frame)
			{
				break;
			}

			switch(j)
			{
				case 0:
					renderer_Color3f(1.0, 0.0, 0.0);
					glLineWidth(8.0);
				break;

				case 1:
					renderer_Color3f(0.0, 1.0, 0.0);
					glLineWidth(4.0);
				break;

				case 2:
					renderer_Color3f(0.0, 0.0, 1.0);
					glLineWidth(2.0);
				break;

				case 3:
					renderer_Color3f(1.0, 0.0, 1.0);
					glLineWidth(1.0);
				break;
			}

			for(k = 0; k < recursive_view_data->views_count; k++)
			{
				view_data = &recursive_view_data->views[k];

				glDepthMask(GL_FALSE);

				renderer_Begin(GL_POINTS);
				renderer_Vertex3f(view_data->position.x, view_data->position.y, view_data->position.z);
				renderer_End();

				renderer_Begin(GL_LINES);

				renderer_Vertex3f(view_data->position.x, view_data->position.y, view_data->position.z);
				renderer_Vertex3f(center.x - right_vector.x + up_vector.x, center.y - right_vector.y + up_vector.y, center.z - right_vector.z + up_vector.z);

				renderer_Vertex3f(view_data->position.x, view_data->position.y, view_data->position.z);
				renderer_Vertex3f(center.x - right_vector.x - up_vector.x, center.y - right_vector.y - up_vector.y, center.z - right_vector.z - up_vector.z);

				renderer_Vertex3f(view_data->position.x, view_data->position.y, view_data->position.z);
				renderer_Vertex3f(center.x + right_vector.x - up_vector.x, center.y + right_vector.y - up_vector.y, center.z + right_vector.z - up_vector.z);

				renderer_Vertex3f(view_data->position.x, view_data->position.y, view_data->position.z);
				renderer_Vertex3f(center.x + right_vector.x + up_vector.x, center.y + right_vector.y + up_vector.y, center.z + right_vector.z + up_vector.z);

				renderer_End();

				glDepthMask(GL_TRUE);

			}

		}

		glLineWidth(1.0);*/


		/*for(j = 0; j < recursive_view_data->views_count; j++)
		{
			view_data = &recursive_view_data->views[j];

			for(k = 0; k < view_data->)

		}*/





		/************************************************************************************************************************************************/
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glPointSize(1.0);
	//renderer_DisableImediateDrawing();

	#endif

}

void renderer_DrawPortalViews()
{

}

void renderer_DrawViews()
{
	view_def_t *view;
	view_def_t *active_view;
	mat4_t view_transform;

	vec3_t near_color;
	vec3_t far_color;

	vec3_t forward_vector;
	vec3_t right_vector;
	vec3_t up_vector;
	vec3_t center;

	view = cameras;
	float far;

	//active_view = camera_GetActiveCamera();

	active_view = renderer_GetActiveView();

	renderer_SetShader(r_imediate_color_shader);
	renderer_SetModelMatrix(NULL);

	//renderer_EnableImediateDrawing();
	glPointSize(8.0);
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while(view)
	{
		if(view != active_view)
		{
		/*	forward_vector.x = view->world_orientation.floats[0][2];
			forward_vector.y = view->world_orientation.floats[1][2];
			forward_vector.z = view->world_orientation.floats[2][2];

			right_vector.x = view->world_orientation.floats[0][0];
			right_vector.y = view->world_orientation.floats[1][0];
			right_vector.z = view->world_orientation.floats[2][0];

			up_vector.x = view->world_orientation.floats[0][1];
			up_vector.y = view->world_orientation.floats[1][1];
			up_vector.z = view->world_orientation.floats[2][1];

			center = view->world_position;*/

			if(view->bm_flags & CAMERA_INACTIVE)
			{
				/*near_color.r = 0.1;
				near_color.g = 0.1;
				near_color.b = 0.1;

				far_color.r = 0.01;
				far_color.g = 0.01;
				far_color.b = 0.01;*/

				view = view->next;
				continue;
			}

			mat4_t_compose(&view_transform, &view->world_orientation, view->world_position);
			renderer_SetModelMatrix(&view_transform);
			//else
			//{
			near_color.r = 0.65;
			near_color.g = 0.65;
			near_color.b = 0.0;

			far_color.r = 0.45;
			far_color.g = 0.45;
			far_color.b = 0.0;
			//}

			/*glLineWidth(2.0);
			renderer_Begin(GL_TRIANGLE_FAN);
			renderer_Color3f(near_color.r, near_color.g, near_color.b);
			renderer_Vertex3f(0.0, 0.0, 0.0);
			renderer_Vertex3f(view->frustum.left, view->frustum.top, -view->frustum.znear);
			renderer_Vertex3f(view->frustum.left, view->frustum.bottom , -view->frustum.znear);
			renderer_Vertex3f(view->frustum.right, view->frustum.bottom, -view->frustum.znear);
			renderer_Vertex3f(view->frustum.right, view->frustum.top, -view->frustum.znear);
			renderer_Vertex3f(view->frustum.left, view->frustum.top, -view->frustum.znear);
			renderer_End();*/

			glLineWidth(2.0);
			far = view->frustum.zfar / view->frustum.znear;
			renderer_Begin(GL_TRIANGLE_FAN);
			renderer_Color3f(far_color.r, far_color.g, far_color.b);
			renderer_Vertex3f(0.0, 0.0, 0.0);
			renderer_Vertex3f(view->frustum.left * far, view->frustum.top * far, -view->frustum.zfar);
			renderer_Vertex3f(view->frustum.left * far, view->frustum.bottom * far, -view->frustum.zfar);
			renderer_Vertex3f(view->frustum.right * far, view->frustum.bottom * far, -view->frustum.zfar);
			renderer_Vertex3f(view->frustum.right * far, view->frustum.top * far, -view->frustum.zfar);
			renderer_Vertex3f(view->frustum.left * far, view->frustum.top * far, -view->frustum.zfar);
			renderer_End();

			renderer_Begin(GL_LINES);

			renderer_Color3f(1.0, 0.0, 0.0);
			renderer_Vertex3f(0.0, 0.0, 0.0);
			renderer_Vertex3f(2.0, 0.0, 0.0);

			renderer_Color3f(0.0, 1.0, 0.0);
			renderer_Vertex3f(0.0, 0.0, 0.0);
			renderer_Vertex3f(0.0, 2.0, 0.0);

			renderer_Color3f(0.0, 0.0, 1.0);
			renderer_Vertex3f(0.0, 0.0, 0.0);
			renderer_Vertex3f(0.0, 0.0, 2.0);

			renderer_End();

		}

		view = view->next;
	}

	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_POINT_SMOOTH);
	glPointSize(1.0);

	//renderer_SetViewMatrix(NULL);


	//renderer_DisableImediateDrawing();
}


void renderer_DrawWaypoints()
{
	int i;
	int c;
	int j;

	struct waypoint_t *waypoints;
	struct waypoint_t *waypoint;
	struct waypoint_t *linked_waypoint;


	renderer_SetShader(r_imediate_color_shader);
	renderer_SetModelMatrix(NULL);


	waypoints = (struct waypoint_t *)nav_waypoints.elements;
	c = nav_waypoints.element_count;


	for(i = 0; i < c; i++)
	{
		waypoint = waypoints + i;

		if(waypoint->flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

		#define WAYPOINT_SIZE 0.2

		renderer_Begin(GL_LINES);

		renderer_Color3f(0.8, 0.8, 1.0);

		renderer_Vertex3f(waypoint->position.x, waypoint->position.y + WAYPOINT_SIZE, waypoint->position.z);
		renderer_Vertex3f(waypoint->position.x, waypoint->position.y - WAYPOINT_SIZE, waypoint->position.z);

		renderer_Vertex3f(waypoint->position.x + WAYPOINT_SIZE, waypoint->position.y, waypoint->position.z);
		renderer_Vertex3f(waypoint->position.x - WAYPOINT_SIZE, waypoint->position.y, waypoint->position.z);

		renderer_Vertex3f(waypoint->position.x, waypoint->position.y, waypoint->position.z + WAYPOINT_SIZE);
		renderer_Vertex3f(waypoint->position.x, waypoint->position.y, waypoint->position.z - WAYPOINT_SIZE);


		renderer_Color3f(1.0, 0.0, 0.0);

		for(j = 0; j < waypoint->links_count; j++)
		{
			linked_waypoint = waypoints + waypoint->links[j].waypoint_index;

			if(linked_waypoint->flags & WAYPOINT_FLAG_INVALID)
			{
				continue;
			}

			renderer_Vertex3f(waypoint->position.x, waypoint->position.y, waypoint->position.z);
			renderer_Vertex3f(linked_waypoint->position.x, linked_waypoint->position.y, linked_waypoint->position.z);
		}

		renderer_End();
	}

}

void renderer_DrawCharacterCollider(void *collider)
{
	struct collider_t *collider_ptr;
	collider_ptr = (struct collider_t *)collider;
}

void renderer_DrawColliders()
{
	int i;
	int j;
	int k;
	int type;
	glPointSize(8.0);
	float h_offset;

	vec3_t capsule_vert;

	struct collider_t *colliders;
	struct rigid_body_collider_t *rigid_body_colliders;
	struct character_collider_t *character_colliders;
	int collider_count;

	//colliders

	renderer_SetShader(r_imediate_color_shader);
	renderer_SetModelMatrix(NULL);

	for(type = 0; type < COLLIDER_TYPE_LAST; type++)
	{
		collider_count = phy_colliders[type].element_count;

		switch(type)
		{
			case COLLIDER_TYPE_CHARACTER_COLLIDER:

				character_colliders = (struct character_collider_t *)phy_colliders[type].elements;

				for(i = 0; i < collider_count; i++)
				{
					if(character_colliders[i].base.flags & COLLIDER_FLAG_INVALID)
					{
						continue;
					}

					renderer_Color3f(1.0, 1.0, 1.0);

					for(k = 0; k < 3; k++)
					{
						renderer_Begin(GL_LINE_LOOP);

						for(j = 0; j < CHARACTER_COLLIDER_CAPSULE_SEGMENTS; j++)
						{
							if(k < 2)
							{
								if(j < CHARACTER_COLLIDER_CAPSULE_SEGMENTS / 2 + 1)
								{
									h_offset = character_colliders[i].height * 0.5 - character_colliders[i].radius;
								}
								else
								{
									h_offset = -character_colliders[i].height * 0.5 + character_colliders[i].radius;
								}
							}
							else
							{
								h_offset = 0.0;
							}

							renderer_Vertex3f(r_collider_capsule_shape[k][j].x * character_colliders[i].radius + character_colliders[i].base.position.x,
											  r_collider_capsule_shape[k][j].y * character_colliders[i].radius + character_colliders[i].base.position.y + h_offset,
											  r_collider_capsule_shape[k][j].z * character_colliders[i].radius + character_colliders[i].base.position.z);

						}

						renderer_End();
					}
				}
			break;

			case COLLIDER_TYPE_PROJECTILE_COLLIDER:

				/*for(i = 0; i < collider_count; i++)
				{
					if(character_colliders[i].flags & COLLIDER_FLAG_INVALID)
					{
						continue;
					}

					renderer_Color3f(1.0, 1.0, 0.0);

					for(k = 0; k < 3; k++)
					{
						renderer_Begin(GL_LINE_LOOP);

						for(j = 0; j < CHARACTER_COLLIDER_CAPSULE_SEGMENTS; j++)
						{
							renderer_Vertex3f(r_collider_capsule_shape[k][j].x * colliders[i].radius + colliders[i].position.x,
											  r_collider_capsule_shape[k][j].y * colliders[i].radius + colliders[i].position.y,
											  r_collider_capsule_shape[k][j].z * colliders[i].radius + colliders[i].position.z);

						}
						renderer_End();
					}

				}*/

			break;

			case COLLIDER_TYPE_RIGID_BODY_COLLIDER:

			break;

		}

	}


}

void renderer_DrawEntities()
{
	int i;
	int c;
	struct entity_t *entity;
	struct entity_transform_t *world_transform;
	struct entity_aabb_t *aabb;

	vec3_t world_position;

	glEnable(GL_POINT_SMOOTH);
	//glPointSize(8.0);
	renderer_SetShader(r_imediate_color_shader);
	renderer_SetModelMatrix(NULL);

	c = ent_entities[0].element_count;

	for(i = 0; i < c; i++)
	{
		entity = (struct entity_t *)ent_entities[0].elements + i;

		if(entity->flags & ENTITY_FLAG_INVALID)
		{
			continue;
		}

		world_transform = entity_GetWorldTransformPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

		world_position.x = world_transform->transform.floats[3][0];
		world_position.y = world_transform->transform.floats[3][1];
		world_position.z = world_transform->transform.floats[3][2];

		glDisable(GL_DEPTH_TEST);
		glPointSize(10.0);
		renderer_Begin(GL_POINTS);
		renderer_Color3f(0.0, 0.0, 0.0);
		renderer_Vertex3f(world_position.x, world_position.y, world_position.z);
		renderer_End();

		glPointSize(8.0);
		renderer_Begin(GL_POINTS);
		renderer_Color3f(1.0, 0.5, 0.0);
		renderer_Vertex3f(world_position.x, world_position.y, world_position.z);
		renderer_End();
		glEnable(GL_DEPTH_TEST);



		if(entity->components[COMPONENT_TYPE_MODEL].type != COMPONENT_TYPE_NONE)
		{
			aabb = entity_GetAabbPointer(entity->components[COMPONENT_TYPE_TRANSFORM]);

			glDisable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			renderer_Begin(GL_QUADS);
			renderer_Color3f(0.6, 0.6, 1.0);

			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z - aabb->current_extents.z);
			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z - aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z - aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z - aabb->current_extents.z);

			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z + aabb->current_extents.z);
			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z + aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z + aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z + aabb->current_extents.z);



			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z - aabb->current_extents.z);
			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z + aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z + aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y + aabb->current_extents.y, world_position.z - aabb->current_extents.z);

			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z - aabb->current_extents.z);
			renderer_Vertex3f(world_position.x - aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z + aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z + aabb->current_extents.z);
			renderer_Vertex3f(world_position.x + aabb->current_extents.x, world_position.y - aabb->current_extents.y, world_position.z - aabb->current_extents.z);

			renderer_End();

			glEnable(GL_CULL_FACE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		}



	}

	glPointSize(1.0);
	glDisable(GL_POINT_SMOOTH);

}

void renderer_DrawTriggers()
{
	int i;
	int c;

    struct trigger_t *triggers;
    struct trigger_t *trigger;

    mat4_t trigger_transform;

    triggers = (struct trigger_t *)ent_triggers.elements;

    c = ent_triggers.element_count;

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDisable(GL_CULL_FACE);
    //renderer_Color3f(1.0, 1.0, 0.0);

    renderer_SetShader(r_imediate_color_shader);
	renderer_SetModelMatrix(NULL);

	glEnable(GL_BLEND);

    for(i = 0; i < c; i++)
	{
        trigger = triggers + i;

		if(trigger->flags & TRIGGER_FLAG_INVALID)
		{
			continue;
		}

		mat4_t_compose2(&trigger_transform, &trigger->orientation, trigger->position, trigger->scale);
        renderer_SetModelMatrix(&trigger_transform);

		renderer_Color4f(1.0, 1.0, 0.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        renderer_DrawBox();

        renderer_Color4f(1.0, 1.0, 0.0, 0.25);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		glCullFace(GL_FRONT);
        renderer_DrawBox();
        glCullFace(GL_BACK);
        renderer_DrawBox();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);
}

void renderer_DrawLights()
{
    int i;
    int c;

    c = l_light_positions.element_count;

    struct light_position_data_t *position;
    struct light_params_data_t *params;
    struct light_cluster_data_t *cluster;

    float color[3] = {0.0};

    renderer_SetShader(r_imediate_color_shader);
	renderer_SetModelMatrix(NULL);


	glEnable(GL_POINT_SMOOTH);

	renderer_Begin(GL_POINTS);
    glPointSize(14.0);
    renderer_Color3f(0.5, 0.5, 1.0);

    for(i = 0; i < c; i++)
    {
        position = (struct light_position_data_t *)l_light_positions.elements + i;

        if(position->flags & LIGHT_INVALID)
        {
            continue;
        }

        renderer_Vertex3f(position->position.x, position->position.y, position->position.z);
    }

    renderer_End();


	renderer_Begin(GL_POINTS);
    glPointSize(8.0);

    for(i = 0; i < c; i++)
    {
        position = (struct light_position_data_t *)l_light_positions.elements + i;

        if(position->flags & LIGHT_INVALID)
        {
            continue;
        }

        params = (struct light_params_data_t *)l_light_params.elements + i;


        color[0] = (float)params->r / 255.0;
        color[1] = (float)params->g / 255.0;
        color[2] = (float)params->b / 255.0;

        renderer_Color3f(color[0], color[1], color[2]);
        renderer_Vertex3f(position->position.x, position->position.y, position->position.z);
    }

    renderer_End();

    glPointSize(1.0);
    glDisable(GL_POINT_SMOOTH);
}

void renderer_DrawClusters()
{
    int i;
    int c;

    view_def_t *active_view;
    active_view = renderer_GetActiveView();

    renderer_SetShader(r_cluster_debug_shader);
    renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, r_cbuffer.depth_attachment);
    renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);
    renderer_SetDefaultUniform1f(UNIFORM_r_near, active_view->frustum.znear);
    renderer_SetDefaultUniform1f(UNIFORM_r_far, active_view->frustum.zfar);
    //renderer_SetDefaultUniform1i(UNIFORM_r_clusters_per_row, r_clusters_per_row);
    //renderer_SetDefaultUniform1i(UNIFORM_r_cluster_rows, r_cluster_rows);
    //renderer_SetDefaultUniform1i(UNIFORM_r_cluster_layers, r_cluster_layers);
    renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
    renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);
    renderer_BindClusterTexture();



    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    //glPointSize(16.0);

    //renderer_Begin(GL_POINTS);
    //renderer_Color3f(1.0, 0.0, 0.0);
    //renderer_Vertex3f(0.0, 0.0, -0.5);
    //renderer_End();

    //glPointSize(1.0);

    renderer_Rectf(-1.0, -1.0, 1.0, 1.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void renderer_DrawStatistics()
{

    int i;

    const char *timer_type;

    gui_ImGuiPushStyleVarf(ImGuiStyleVar_WindowRounding, 0.0);
    gui_ImGuiSetNextWindowBgAlpha(0.0);
    gui_ImGuiSetNextWindowPos(vec2(0.0, 0.0), 0, vec2(0.0, 0.0));
    gui_ImGuiSetNextWindowFocus();
    gui_ImGuiBegin("Statistics", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs);

    gui_ImGuiText("Screen size: %d x %d\n", r_window_width, r_window_height);
    gui_ImGuiText("Renderer resolution: %d x %d\n", r_width, r_height);
    gui_ImGuiText("Frame: %d\n", r_frame);
    gui_ImGuiText("Draw calls: %d\n", r_draw_calls);
    gui_ImGuiText("Shader swaps: %d\n", r_shader_swaps);
    gui_ImGuiText("Uniform updates: %d\n", r_shader_uniform_updates);
    gui_ImGuiText("Drawn verts: %d\n", r_frame_vert_count);

    if(r_timers_count)
    {
        gui_ImGuiText("Timers:\n");

        for(i = 0; i < r_timers_count; i++)
        {
            if(r_timers[i].frame == -1)
            {
                if(r_timers[i].gpu_timer)
                {
                    timer_type = "GPU";
                }
                else
                {
                    timer_type = "CPU";
                }

                gui_ImGuiText("%s (%s): %f ms\n", r_timers[i].name, timer_type, r_timers[i].delta_time);
            }

        }
    }


    gui_ImGuiEnd();

    gui_ImGuiPopStyleVar();
}

/*
==============================================================
==============================================================
==============================================================
*/



void renderer_DrawDebug()
{
	int i;
	int j;

	dbg_command_t *cmd;
	point_dbg_draw_data_t *point_data;
	line_dbg_draw_data_t *line_data;
	line_2d_dbg_draw_data_t *line_2d_data;
	view_def_t *active_view;

    active_view = renderer_GetActiveView();

	//active_camera = camera_GetActiveCamera();


	renderer_EnableImediateDrawing();

    renderer_SetProjectionMatrix(&active_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_view->view_data.view_matrix);



	/*if(r_debug_draw_portal_outlines)
	{
		renderer_DrawPortalsOulines();
	}*/




	if(r_debug_draw_views)
	{
		renderer_DrawViews();
	}

	if(r_debug_draw_waypoints)
	{
		renderer_DrawWaypoints();
	}

	if(r_debug_draw_colliders)
	{
		renderer_DrawColliders();
	}

	if(r_debug_draw_entities)
	{
		renderer_DrawEntities();
	}

	if(r_debug_draw_triggers)
	{
		renderer_DrawTriggers();
	}

	if(r_debug_draw_lights)
    {
        renderer_DrawLights();
    }

    if(r_debug_draw_clusters)
    {
        renderer_DrawClusters();
    }

    if(r_debug_draw_statistics)
    {
        renderer_DrawStatistics();
    }

    renderer_SetShader(r_imediate_color_shader);
	renderer_SetModelMatrix(NULL);

	while(r_dbg_debug_cmd_next_out != r_dbg_debug_cmd_next_in)
	{
		cmd = r_dbg_debug_cmds + r_dbg_debug_cmd_next_out;
		r_dbg_debug_cmd_next_out = (r_dbg_debug_cmd_next_out + 1) % r_dbg_max_dbg_cmds;

		switch(cmd->type)
		{

			case DEBUG_COMMAND_TYPE_DRAW_POINT:
				point_data = cmd->data;

				glPointSize(point_data->size);

				if(point_data->smooth)
				{
					glEnable(GL_POINT_SMOOTH);
				}

				if(point_data->draw_params.depth_test)
				{
					glEnable(GL_DEPTH_TEST);
				}
				else
				{
					glDisable(GL_DEPTH_TEST);
				}

				renderer_Begin(GL_POINTS);
				renderer_Color3f(point_data->color.r, point_data->color.y, point_data->color.z);
				renderer_Vertex3f(point_data->position.x, point_data->position.y, point_data->position.z);
				renderer_End();

				glPointSize(1.0);
				if(point_data->smooth)
				{
					glDisable(GL_POINT_SMOOTH);
				}

				if(!point_data->draw_params.depth_test)
				{
					glEnable(GL_DEPTH_TEST);
				}

				r_dbg_draw_bytes_next_out = (r_dbg_draw_bytes_next_out + sizeof(point_dbg_draw_data_t)) % r_dbg_max_draw_bytes;
			break;

			case DEBUG_COMMAND_TYPE_DRAW_LINE:
				line_data = cmd->data;

				glLineWidth(line_data->width);

				renderer_Begin(GL_LINES);
				renderer_Color3f(line_data->color.r, line_data->color.g, line_data->color.b);
				renderer_Vertex3f(line_data->from.x, line_data->from.y, line_data->from.z);
				renderer_Vertex3f(line_data->to.x, line_data->to.y, line_data->to.z);
				renderer_End();

				glLineWidth(1.0);

				r_dbg_draw_bytes_next_out = (r_dbg_draw_bytes_next_out + sizeof(line_dbg_draw_data_t)) % r_dbg_max_draw_bytes;
			break;


			case DEBUG_COMMAND_TYPE_DRAW_2D_LINE:
				line_2d_data = cmd->data;

				glLineWidth(line_2d_data->width);

				renderer_SetProjectionMatrix(NULL);
				renderer_SetViewMatrix(NULL);


				renderer_Begin(GL_LINES);
				renderer_Color3f(line_2d_data->color.r, line_2d_data->color.g, line_2d_data->color.b);
				renderer_Vertex3f(line_2d_data->from.x, line_2d_data->from.y, 0.0);
				renderer_Vertex3f(line_2d_data->to.x, line_2d_data->to.y, 0.0);
				renderer_End();

				renderer_SetProjectionMatrix(&active_view->view_data.projection_matrix);
				renderer_SetViewMatrix(&active_view->view_data.view_matrix);

				glLineWidth(1.0);

				r_dbg_draw_bytes_next_out = (r_dbg_draw_bytes_next_out + sizeof(line_dbg_draw_data_t)) % r_dbg_max_draw_bytes;
			break;

		}

	}

	renderer_DisableImediateDrawing();

	renderer_StopAllTimers();
	//r_timers_count = 0;
}



void renderer_PushFunctionName(const char *name)
{
	if(r_dbg_function_names_stack_top < R_DBG_FUNCTION_NAME_STACK_DEPTH - 1)
	{
		r_dbg_function_names_stack_top++;
		strcpy(r_dbg_function_names[r_dbg_function_names_stack_top], name);
	}
}

void renderer_PopFunctionName()
{
	if(r_dbg_function_names_stack_top >= 0)
	{
		while(glGetError() != GL_NO_ERROR);
		r_dbg_function_names_stack_top--;
	}
}

#ifdef __cplusplus
}
#endif



