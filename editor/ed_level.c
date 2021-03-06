#include "ed_level.h"
#include "ed_level_draw.h"
#include "ed_level_ui.h"
#include "ed_picking.h"
#include "..\..\common\r_main.h"
#include "..\..\common\r_view.h"
#include "..\common\r_debug.h"

#include "..\..\common\gmath\matrix.h"
#include "..\..\common\gmath\vector.h"

#include "..\..\common\gui.h"

#include "..\ed_common.h"
//#include "..\ed_cursors.h"
#include "..\editor.h"
#include "..\ed_selection.h"
#include "..\ed_level_manip.h"
#include "..\brush.h"
#include "..\ed_ui_explorer.h"

#include "..\..\common\camera.h"
#include "..\..\common\input.h"
#include "..\..\common\c_memory.h"
#include "..\..\common\l_main.h"
#include "..\..\common\player.h"
#include "..\..\common\entity.h"
#include "..\..\common\ent_serialization.h"
#include "..\..\common\world.h"
#include "..\..\common\entity.h"
#include "..\..\common\model.h"
#include "..\..\common\ent_common.h"
#include "..\..\common\bsp.h"
#include "..\..\common\engine.h"
#include "..\..\common\portal.h"
#include "..\..\common\particle.h"
#include "..\..\common\script.h"
#include "..\..\common\navigation.h"
#include "..\..\common\resource.h"
#include "..\..\common\sound.h"
#include "..\..\common\serializer.h"

#include "..\..\common\containers\stack_list.h"

#include "..\..\common\GLEW\include\GL\glew.h"


/* from r_main.c */
extern struct renderer_t r_renderer;
//extern int r_width;
//extern int r_height;
//extern int r_window_width;
//extern int r_window_height;

/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern float last_mouse_x;
extern float last_mouse_y;
extern float mouse_dx;
extern float mouse_dy;
extern int bm_mouse;
extern int mouse_x;
extern int mouse_y;


/* from world.c */
extern int w_world_vertices_count;
extern vertex_t *w_world_vertices;

extern int w_world_nodes_count;
extern struct bsp_pnode_t *w_world_nodes;

extern int w_world_leaves_count;
extern struct bsp_dleaf_t *w_world_leaves;

extern int w_world_batch_count;
extern struct batch_t *w_world_batches;


/* from l_main.c */
//extern int l_light_list_cursor;
//extern int l_light_list_size;
//extern int l_free_position_stack_top;
//extern int *l_free_position_stack;
//extern light_params_t *l_light_params;
//extern light_position_t *l_light_positions;

/* from player.c */
extern spawn_point_t *spawn_points;

/* from entity.c */
//extern int ent_entity_list_cursor;
//extern int ent_entity_list_size;
//extern int ent_free_stack_top;
//extern int *ent_free_stack;
extern struct stack_list_t ent_entities[2];
extern struct stack_list_t ent_components[2][COMPONENT_TYPE_LAST];
//extern struct entity_aabb_t *ent_aabbs;

/* from physics.c */
//extern int collider_list_cursor;
//extern int max_colliders;
//int colliders_free_positions_stack_top;
//int *colliders_free_positions_stack;
//collider_t *colliders;


/* from navigation.c */
struct stack_list_t *nav_waypoints;
//extern int nav_waypoint_count;
//extern struct waypoint_t *nav_waypoints;


/* from portal.c */
extern int ptl_portals_list_cursor;
extern portal_t *ptl_portals;

/* from brush.c */
extern brush_t *brushes;
extern brush_t *last_brush;
extern int brush_count;

/* from texture.c */
extern struct stack_list_t tex_textures;
extern struct stack_list_t mdl_models;

/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/


struct editor_state_t level_editor_state;



//float level_editor_camera_pitch = 0.0;
//float level_editor_camera_yaw = 0.0;
//float level_editor_camera_speed_multiplier = 1.0;
//struct view_handle_t level_editor_view = INVALID_VIEW_HANDLE;
//camera_t *level_editor_camera = NULL;


/*int level_editor_max_selections = 0;
int level_editor_selection_count = 0;
pick_record_t *level_editor_selections = NULL;
int level_editor_last_selection_type = PICK_NONE;*/

//pick_list_t level_editor_pick_list;
struct pick_list_t level_editor_brush_face_pick_list;
struct pick_list_t level_editor_brush_face_uv_pick_list;

//
//int level_editor_3d_handle_transform_mode = ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION;
//int level_editor_3d_handle_transform_orientation = ED_3D_HANDLE_TRANSFORM_ORIENTATION_GLOBAL;
//int level_editor_3d_handle_pivot_mode = ED_3D_HANDLE_PIVOT_MODE_MEDIAN_POINT;
//int level_editor_3d_handle_flags = 0;

//struct level_editor_3d_manipulator_t level_editor_3d_manipulator;

//struct selection_state_t level_editor_selection_state;

//vec3_t level_editor_3d_handle_position;

vec3_t level_editor_3d_cursor_position;


//float level_editor_linear_snap_value;
//float level_editor_angular_snap_value;


int level_editor_editing_mode = EDITING_MODE_OBJECT;
//int level_editor_state = EDITOR_EDITING;



int level_editor_draw_brushes = 1;
int level_editor_draw_wireframe_brushes = 0;
int level_editor_draw_grid = 1;
int level_editor_draw_selected = 1;
int level_editor_draw_cursors = 1;
int level_editor_draw_lights = 1;
int level_editor_draw_spawn_points = 1;
int level_editor_draw_leaves = 1;
int level_editor_draw_world_polygons = 1;
int level_editor_draw_clipped_polygons = 1;
int level_editor_draw_entities_aabbs = 1;
int level_editor_draw_3d_handle = 0;


/* the level editor plain copies all the data
that exists in the world... */
int level_editor_has_copied_data = 0;
int level_editor_need_to_copy_data = 1;
int level_editor_debug_draw_flags = 0;

brush_t *level_editor_brushes = NULL;
brush_t *level_editor_last_brush = NULL;
int level_editor_brush_count = 0;

int level_editor_world_vertices_count = 0;
vertex_t *level_editor_world_vertices = NULL;

int level_editor_world_nodes_count = 0;
struct bsp_pnode_t *level_editor_world_nodes = NULL;

int level_editor_world_leaves_count = 0;
struct bsp_dleaf_t *level_editor_world_leaves = NULL;

int level_editor_world_batch_count = 0;
struct batch_t *level_editor_world_batches = NULL;

int level_editor_entity_buffer_size = 0;
void *level_editor_entity_buffer = NULL;

int level_editor_waypoint_buffer_size = 0;
void *level_editor_waypoint_buffer = NULL;

int level_editor_light_buffer_size = 0;
void *level_editor_light_buffer = NULL;

int level_editor_world_buffer_size = 0;
void *level_editor_world_buffer = NULL;

//int level_editor_entity_list_cursor = 0;
//int level_editor_entity_list_size = 0;
//int level_editor_entity_free_stack_top = -1;
//int *level_editor_entity_free_stack = NULL;
//struct entity_t *level_editor_entities = NULL;
//struct entity_aabb_t *level_editor_entity_aabbs = NULL;

//int level_editor_collider_list_cursor = 0;
//int level_editor_collider_list_size = 0;
//int level_editor_collider_free_stack_top = -1;
//int *level_editor_collider_free_stack = NULL;
//collider_t *level_editor_colliders = NULL;

//int level_editor_light_list_cursor = 0;
//int level_editor_light_list_size = 0;
//int level_editor_light_free_stack_top = -1;
//int *level_editor_light_free_stack = NULL;
//light_position_t *level_editor_light_positions = NULL;
//light_params_t *level_editor_light_params = NULL;

int level_editor_portal_list_cursor = 0;
int level_editor_portal_list_size = 0;
int level_editor_portal_free_stack_top = -1;
int *level_editor_portal_free_stack = NULL;
portal_t *level_editor_portals = NULL;

struct stack_list_t brush_transforms;



char level_editor_level_name[PATH_MAX] = {0};
char level_editor_level_path[PATH_MAX] = {0};

char default_level_script_text[] =
{
    "void OnMapLoad()\n{\n\n}\n\n"
    "void OnMapEnter()\n{\n\n}\n\n"
    "void OnMapUpdate()\n{\n\n}\n\n"
    "void OnMapLeave()\n{\n\n}\n\n"
};


enum LEVEL_EDITOR_LEVEL_FODERS
{
    LEVEL_EDITOR_FOLDER_FIRST = 0,
    LEVEL_EDITOR_FOLDER_MODELS = LEVEL_EDITOR_FOLDER_FIRST,
    LEVEL_EDITOR_FOLDER_ENTITIES,
    LEVEL_EDITOR_FOLDER_SCRIPTS,
    LEVEL_EDITOR_FOLDER_SOUNDS,
    LEVEL_EDITOR_FOLDER_TEXTURES,
    LEVEL_EDITOR_FOLDER_LAST,
};


char *level_editor_level_folder_names[] =
{
    "models",
    "entities",
    "scripts",
    "sounds",
    "textures",
    NULL,
};


/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/

int portal0;
int portal1;

void editor_LevelEditorInit()
{
	mat3_t r = mat3_t_id();
	int i;
	level_editor_state.editor_view = renderer_CreateViewDef("level editor camera", vec3_t_c(20.0, 10.0, 15.0), &r, 0.68, r_renderer.r_width, r_renderer.r_height, 0.1, 500.0, 0);
	//level_editor_camera = camera_CreateCamera("level editor camera", vec3(0.0, 7.0, 0.0), &r, 0.68, r_width, r_height, 0.1, 500.0, 0);

//	camera_Deactivate(level_editor_camera);
	level_editor_state.editor_camera_yaw = 0.2;
	level_editor_state.editor_camera_pitch = -0.15;
	level_editor_state.editor_camera_speed_multiplier = 0.4;
    level_editor_state.transforming_selection = 0;
    level_editor_state.editor_state = EDITOR_EDITING;
    level_editor_state.edition_target = EDITING_MODE_OBJECT;



	level_editor_level_folder_names[LEVEL_EDITOR_FOLDER_MODELS] = "models";
	level_editor_level_folder_names[LEVEL_EDITOR_FOLDER_ENTITIES] = "entities";
	level_editor_level_folder_names[LEVEL_EDITOR_FOLDER_SCRIPTS] = "scripts";
	level_editor_level_folder_names[LEVEL_EDITOR_FOLDER_SOUNDS] = "sounds";
	level_editor_level_folder_names[LEVEL_EDITOR_FOLDER_TEXTURES] = "textures";

	//level_editor_camera_yaw = 0.0;
	//level_editor_camera_pitch = 0.0;

    level_editor_state.selection_state.pick_list = editor_CreatePickList();
    level_editor_state.selection_state.cursor_position = vec3_t_c(0.0, 0.0, 0.0);
    level_editor_state.selection_state.manipulator_position = vec3_t_c(0.0, 0.0, 0.0);

    level_editor_brush_face_pick_list = editor_CreatePickList();
    level_editor_brush_face_uv_pick_list = editor_CreatePickList();

//	level_editor_pick_list.max_records = 1024;
//	level_editor_pick_list.record_count = 0;
//	level_editor_pick_list.records = memory_Malloc(sizeof(pick_record_t) * level_editor_pick_list.max_records);
//

//	level_editor_brush_face_pick_list.max_records = 10;
//	level_editor_brush_face_pick_list.record_count = 0;
//	level_editor_brush_face_pick_list.records = memory_Malloc(sizeof(pick_record_t) * level_editor_brush_face_pick_list.max_records);


//	level_editor_brush_face_uv_pick_list.max_records = 128;
//	level_editor_brush_face_uv_pick_list.record_count = 0;
//	level_editor_brush_face_uv_pick_list.records = memory_Malloc(sizeof(pick_record_t) * level_editor_brush_face_uv_pick_list.max_records);

	renderer_PitchYawView(level_editor_state.editor_view, level_editor_state.editor_camera_yaw, level_editor_state.editor_camera_pitch);
	renderer_ComputeViewMatrix(level_editor_state.editor_view);

	editor_LevelEditorUIInit();

	brush_transforms = stack_list_create(sizeof(mat4_t), 1024, NULL);

	brush_Init();

    gui_ImGuiAddFontFromFileTTF("fixedsys.ttf", 32.0);


	level_editor_debug_draw_flags = R_DEBUG_DRAW_FLAG_DRAW_ENTITIES | R_DEBUG_DRAW_FLAG_DRAW_LIGHTS |
                                    R_DEBUG_DRAW_FLAG_DRAW_TRIGGERS | R_DEBUG_DRAW_FLAG_DRAW_WAYPOINTS;


    editor_LevelEditorNewLevel();
}

void editor_LevelEditorFinish()
{
	editor_LevelEditorUIFinish();
	brush_Finish();

//	memory_Free(level_editor_pick_list.records);
//	memory_Free(level_editor_brush_face_pick_list.records);
//	memory_Free(level_editor_brush_face_uv_pick_list.records);

	if(level_editor_has_copied_data)
	{
		//memory_Free(level_editor_entities);
		//memory_Free(level_editor_entity_aabbs);
		//memory_Free(level_editor_entity_free_stack);

		//memory_Free(level_editor_collider_free_stack);
		//memory_Free(level_editor_colliders);

		//memory_Free(level_editor_light_positions);
		//memory_Free(level_editor_light_params);
		//memory_Free(level_editor_light_free_stack);

		//memory_Free(level_editor_portal_free_stack);
		//memory_Free(level_editor_portals);
	}

}

void editor_LevelEditorSetup()
{
//	camera_SetCamera(level_editor_camera);
//	camera_SetMainViewCamera(level_editor_camera);
    renderer_SetMainView(level_editor_state.editor_view);

	renderer_RegisterCallback(editor_LevelEditorPreDraw, PRE_SHADING_STAGE_CALLBACK);
	renderer_RegisterCallback(editor_LevelEditorPostDraw, POST_SHADING_STAGE_CALLBACK);
	editor_LevelEditorUISetup();
	editor_LevelEditorRestoreLevelData();

	renderer_DebugDrawFlags(level_editor_debug_draw_flags);

	//editor_SetExplorerReadFileCallback(editor_LevelEditorLoadLevel);
	//editor_SetExplorerWriteFileCallback(editor_LevelEditorSaveLevel);
}

void editor_LevelEditorShutdown()
{

	editor_LevelEditorCloseAddToWorldMenu();
	editor_LevelEditorCloseDeleteSelectionsMenu();

	renderer_ClearRegisteredCallbacks();
	editor_LevelEditorUIShutdown();
	editor_LevelEditorCopyLevelData();
	editor_LevelEditorClearLevelData();

	level_editor_debug_draw_flags = renderer_GetDebugDrawFlags();

	//editor_ClearExplorerFileCallbacks();

	//camera_Deactivate(level_editor_camera);
}

void editor_LevelEditorRestart()
{

}

void editor_LevelEditorMain(float delta_time)
{
//	camera_t *active_camera;

	portal_t *portal;

	struct view_def_t *view;


	static float r = -1.0;
	static int d = 0;


	r += 0.005;



    view = renderer_GetViewPointer(level_editor_state.editor_view);

	//active_camera = camera_GetActiveCamera();

	//active_camera = camera_GetMainViewCamera();
	//if(active_camera == level_editor_camera)
	{
		CreatePerspectiveMatrix(&view->view_data.projection_matrix, 0.68, (float)r_renderer.r_window_width / (float)r_renderer.r_window_height, 0.1, 500.0, 0.0, 0.0, &view->frustum);
	}




	if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED /*&& (!(bm_mouse & MOUSE_OVER_WIDGET))*/)
	{
		editor_LevelEditorFly(delta_time);
	}
	else
	{
		editor_LevelEditorEdit(delta_time);
	}

	editor_LevelEditorUpdateUI();

//	brush_ProcessBrushes();

    brush_UpdateBrushes();

}
//
//
//struct pick_record_t editor_LevelEditorPickObject(float mouse_x, float mouse_y)
//{
////	pick_record_t record;
////	pick_list_t *pick_list;
////	int lshift;
////
////
////	if(level_editor_editing_mode == EDITING_MODE_OBJECT)
////	{
////		pick_list = &level_editor_pick_list;
////		record = editor_PickObject(mouse_x, mouse_y);
////	}
////	else
////	{
////		pick_list = &level_editor_brush_face_pick_list;
////		record = editor_GetLastSelection(&level_editor_pick_list);
////
////		if(record.type == PICK_BRUSH)
////		{
////			record = editor_PickBrushFace((brush_t *)record.pointer, mouse_x, mouse_y);
////			//editor_ClearSelection(&level_editor_brush_face_pick_list);
////			//editor_AddSelection(&record, &level_editor_brush_face_pick_list);
////		}
////		else
////		{
////			return record;
////		}
////	}
////
////
////	lshift = input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED;
////
////	if(record.type == pick_list->records[pick_list->record_count - 1].type)
////	{
////		if(record.type == PICK_BRUSH)
////		{
////			if(record.pointer == pick_list->records[pick_list->record_count - 1].pointer)
////			{
////				goto _same_selection;
////			}
////
////			goto _add_new_selection;
////		}
////
////		/* if the just picked thing is the same as the last picked thing... */
////		if(record.index0 == pick_list->records[pick_list->record_count - 1].index0)
////		{
////			_same_selection:
////			/* this record already exists in the list, so drop it... */
////			editor_DropSelection(&record, pick_list);
////
////			if(!lshift)
////			{
////				if(pick_list->record_count)
////				{
////					goto _add_new_selection;
////				}
////			}
////
////			goto _set_handle_3d_position;
////		}
////		else
////		{
////			/* if this record  is not equal to the last in the list,
////			append it to the list or set it as the only active object... */
////			goto _add_new_selection;
////		}
////
////	}
////	else
////	{
////		_add_new_selection:
////		/* holding shift enables selecting multiple objects... */
////		if(!lshift)
////		{
////			editor_ClearSelection(pick_list);
////		}
////
////		editor_AddSelection(&record, pick_list);
////
////		_set_handle_3d_position:
////
////		editor_LevelEditorUpdate3dHandlePosition();
////	}
////
////
//
//
//
//}

int editor_LevelEditorPickBrushFace(brush_t *brush, float mouse_x, float mouse_y)
{

}


/*
====================================================================
====================================================================
====================================================================
*/


void editor_LevelEditorCheck3dHandle(float mouse_x, float mouse_y)
{
	//level_editor_3d_handle_flags = editor_Check3dHandle(mouse_x, mouse_y, level_editor_3d_handle_position, level_editor_3d_handle_transform_mode);
}

void editor_LevelEditorSet3dCursorPosition(float mouse_x, float mouse_y)
{
	//level_editor_3d_cursor_position = editor_3dCursorPosition(mouse_x, mouse_y);
}

void editor_LevelEditorUpdate3dHandlePosition()
{
	//level_editor_3d_handle_position = editor_GetCenterOfSelections(&level_editor_pick_list);
}

void editor_LevelEditorSet3dHandleTransformMode(int mode)
{
	/*switch(mode)
	{
		case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
		case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:
		case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:
			level_editor_3d_handle_transform_mode = mode;
		break;
	}*/
}

void editor_LevelEditorSet3dHandleTransformOrientation(int orientation)
{
    /*switch(orientation)
    {
        case ED_3D_HANDLE_TRANSFORM_ORIENTATION_LOCAL:
        case ED_3D_HANDLE_TRANSFORM_ORIENTATION_GLOBAL:
            level_editor_3d_handle_transform_orientation = orientation;
        break;
    }*/
}

void editor_LevelEditorSetEditingMode(int mode)
{
	switch(mode)
	{
		case EDITING_MODE_BRUSH:
			editor_ClearSelection(&level_editor_brush_face_pick_list);
		case EDITING_MODE_OBJECT:
		case EDITING_MODE_UV:
			level_editor_editing_mode = mode;
		break;
	}
}

void editor_LevelEditorEnterManipulationMode(int transform_op, int axis_constraint)
{
    level_editor_state.transforming_selection = 1;
    editor_LevelEditorTransformOp(&level_editor_state.selection_state, transform_op);
    editor_LevelEditorTransformAxis(&level_editor_state.selection_state, axis_constraint);
}

void editor_LevelEditorLeaveManipulationMode()
{
    level_editor_state.transforming_selection = 0;
}


/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorEdit(float delta_time)
{
//	pick_record_t record;
	//camera_t *active_camera = camera_GetActiveCamera();
	struct view_def_t *main_view = renderer_GetMainViewPointer();
	mat4_t model_view_projection_matrix;
	vec4_t p;
	vec4_t sp;
	vec3_t linear_mouse_delta;
	vec3_t transform_axis;
	float transform_delta;

	struct pick_record_t record;

	struct entity_handle_t entity;
	struct entity_t *entity_ptr;
	struct entity_prop_t *life_prop;
	int life;

	int i;

	int lshift;
	int is_last_selection;
	float d;

	/*float handle_3d_screen_x;
	float handle_3d_screen_y;

	float mouse_screen_x;
	float mouse_screen_y;*/

	float screen_x;
	float screen_y;

	float screen_dx;
	float screen_dy;

	static float grab_screen_offset_x;
	static float grab_screen_offset_y;

	static float prev_dx;
	static float prev_dy;

	float amount;
	float z;




	if(engine_GetEngineState() & ENGINE_PAUSED)
	{
        lshift = input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED;

        if(lshift)
		{
			if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
			{
				if(level_editor_editing_mode == EDITING_MODE_OBJECT)
				{
					editor_LevelEditorOpenAddToWorldMenu();
				}
			}

			else if(input_GetKeyStatus(SDL_SCANCODE_C) & KEY_JUST_PRESSED)
			{
				editor_LevelEditorToggleMapCompilerWindow();
			}

			else if(input_GetKeyStatus(SDL_SCANCODE_D) & KEY_JUST_PRESSED)
            {
                editor_LevelEditorCopySelections(&level_editor_state.selection_state);
                level_editor_state.transforming_selection = 1;
                editor_LevelEditorTransformOp(&level_editor_state.selection_state, LEVEL_EDITOR_TRANSFORM_OPERATION_TRANSLATION);
            }
		}
		else
        {
            if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
            {
                editor_ClearSelection(&level_editor_state.selection_state.pick_list);
            }
        }



		if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
        {
            record = editor_PickObject(normalized_mouse_x, normalized_mouse_y);

            if(record.type != PICK_NONE)
            {
                if(editor_IsSelectionInList(&record, &level_editor_state.selection_state.pick_list))
                {
                    is_last_selection = editor_IsLastSelection(&record, &level_editor_state.selection_state.pick_list);

                    editor_DropSelection(&record, &level_editor_state.selection_state.pick_list);

                    if(is_last_selection)
                    {
                        if(lshift || level_editor_state.selection_state.pick_list.record_count == 0)
                        {
                            record.type = PICK_NONE;
                        }
                        else
                        {
                            editor_ClearSelection(&level_editor_state.selection_state.pick_list);
                        }
                    }
                }
                else
                {
                    if(!lshift)
                    {
                        editor_ClearSelection(&level_editor_state.selection_state.pick_list);
                    }
                }

                editor_AddSelection(&record, &level_editor_state.selection_state.pick_list);
            }

        }

        else if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
        {
            if(level_editor_state.transforming_selection)
            {
                level_editor_state.transforming_selection = 0;
            }
            else if(!(bm_mouse & MOUSE_OVER_WIDGET))
            {
                level_editor_state.selection_state.cursor_position = editor_3DCursorPosition(normalized_mouse_x, normalized_mouse_y);
            }
        }



        if(level_editor_state.selection_state.pick_list.record_count)
        {
            if(input_GetKeyStatus(SDL_SCANCODE_G) & KEY_JUST_PRESSED)
            {
                level_editor_state.transforming_selection = 1;
                editor_LevelEditorTransformOp(&level_editor_state.selection_state, LEVEL_EDITOR_TRANSFORM_OPERATION_TRANSLATION);
            }
            else if(input_GetKeyStatus(SDL_SCANCODE_R) & KEY_JUST_PRESSED)
            {
                level_editor_state.transforming_selection = 1;
                editor_LevelEditorTransformOp(&level_editor_state.selection_state, LEVEL_EDITOR_TRANSFORM_OPERATION_ROTATION);
            }
            else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
            {
                level_editor_state.transforming_selection = 1;
               editor_LevelEditorTransformOp(&level_editor_state.selection_state, LEVEL_EDITOR_TRANSFORM_OPERATION_SCALING);
            }

            if(level_editor_state.transforming_selection)
            {
                if(input_GetKeyStatus(SDL_SCANCODE_X) & KEY_JUST_PRESSED)
                {
                    editor_LevelEditorTransformAxis(&level_editor_state.selection_state, SELECTION_TRANSFORM_CONSTRAINT_X_AXIS);
                }

                if(input_GetKeyStatus(SDL_SCANCODE_Y) & KEY_JUST_PRESSED)
                {
                    editor_LevelEditorTransformAxis(&level_editor_state.selection_state, SELECTION_TRANSFORM_CONSTRAINT_Y_AXIS);
                }

                if(input_GetKeyStatus(SDL_SCANCODE_Z) & KEY_JUST_PRESSED)
                {
                    editor_LevelEditorTransformAxis(&level_editor_state.selection_state, SELECTION_TRANSFORM_CONSTRAINT_Z_AXIS);
                }

                editor_LevelEditorCalculateMouseDelta(&level_editor_state.selection_state);

                switch(level_editor_state.selection_state.transform_operation)
                {
                    case LEVEL_EDITOR_TRANSFORM_OPERATION_TRANSLATION:
                        editor_LevelEditorTranslateSelections(&level_editor_state.selection_state,
                                                               level_editor_state.selection_state.translation_delta);
                    break;

                    case LEVEL_EDITOR_TRANSFORM_OPERATION_ROTATION:
                        editor_LevelEditorRotateSelections(&level_editor_state.selection_state,
                                                            level_editor_state.selection_state.rotation_axis,
                                                            level_editor_state.selection_state.rotation_delta);
                    break;

                    case LEVEL_EDITOR_TRANSFORM_OPERATION_SCALING:
                        editor_LevelEditorScaleSelections(&level_editor_state.selection_state,
                                                           level_editor_state.selection_state.scaling_axis,
                                                           level_editor_state.selection_state.scaling_delta);
                    break;
                }
            }
        }


        if(!level_editor_state.transforming_selection)
        {
            editor_LevelEditorTransformAxis(&level_editor_state.selection_state, 0);
        }



      //  editor_LevelEditorTransformSelection(&level_editor_selection_state);

//		if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
//		{
//			editor_LevelEditorCheck3dHandle(normalized_mouse_x, normalized_mouse_y);
//
//			if(!level_editor_3d_handle_flags)
//			{
//				editor_LevelEditorSet3dCursorPosition(normalized_mouse_x, normalized_mouse_y);
//			}
//		}
//
//		if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
//		{
//			level_editor_3d_handle_flags = 0;
//		}
//		else if(level_editor_3d_handle_flags)
//		{
//			p.vec3 = level_editor_3d_handle_position;
//			p.w = 1.0;
//			mat4_t_mult_fast(&model_view_projection_matrix, &main_view->view_data.view_matrix, &main_view->view_data.projection_matrix);
//			mat4_t_vec4_t_mult(&model_view_projection_matrix, &p);
//
//
//			p.x /= p.w;
//			p.y /= p.w;
//			z = p.z;
//
//			if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
//			{
//				if(level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE || level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
//				{
//					grab_screen_offset_x = normalized_mouse_x - p.x;
//					grab_screen_offset_y = normalized_mouse_y - p.y;
//				}
//				else
//				{
//					grab_screen_offset_x = 0.0;
//					grab_screen_offset_y = 0.0;
//				}
//
//			}
//
//			screen_dx = normalized_mouse_x - p.x - grab_screen_offset_x;
//			screen_dy = normalized_mouse_y - p.y - grab_screen_offset_y;
//
//			if(level_editor_3d_handle_flags & ED_3D_HANDLE_X_AXIS_GRABBED)
//			{
//				direction = vec3_t_c(1.0, 0.0, 0.0);
//			}
//			else if(level_editor_3d_handle_flags & ED_3D_HANDLE_Y_AXIS_GRABBED)
//			{
//				direction = vec3_t_c(0.0, 1.0, 0.0);
//			}
//			else if(level_editor_3d_handle_flags & ED_3D_HANDLE_Z_AXIS_GRABBED)
//			{
//				direction = vec3_t_c(0.0, 0.0, 1.0);
//			}
//
//			if(level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE || level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION)
//			{
//				sp = p;
//				p.vec3 = direction;
//				p.w = 0.0;
//
//				mat4_t_vec4_t_mult(&main_view->view_data.view_matrix, &p);
//
//				screen_x = p.x;
//				screen_y = p.y;
//
//				amount = sqrt(screen_x * screen_x + screen_y * screen_y);
//
//				screen_x /= amount;
//				screen_y /= amount;
//
//				amount = (screen_dx * screen_x + screen_dy * screen_y) * z;
//
//
//				if(level_editor_linear_snap_value > 0.0)
//				{
//					d = amount / level_editor_linear_snap_value;
//			 		amount = level_editor_linear_snap_value * (int)d;
//				}
//
//
//				if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
//				{
//					if(level_editor_3d_handle_transform_mode == ED_3D_HANDLE_TRANSFORM_MODE_SCALE)
//					{
//						if(fabs(amount) > 0.0)
//						{
//							grab_screen_offset_x = normalized_mouse_x - sp.x;
//							grab_screen_offset_y = normalized_mouse_y - sp.y;
//						}
//
//					}
//				}
//
//			}
//			else
//			{
//
//				amount = sqrt(screen_dx * screen_dx + screen_dy * screen_dy);
//
//				screen_dx /= amount;
//				screen_dy /= amount;
//
//				if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
//				{
//					prev_dx = screen_dx;
//					prev_dy = screen_dy;
//				}
//
//				amount = asin(prev_dx * screen_dy - prev_dy * screen_dx);
//
//				d = dot3(direction, main_view->world_orientation.f_axis);
//
//				if(d < 0)
//				{
//					amount = -amount;
//				}
//				else if(d == 0)
//				{
//					d = dot3(direction, main_view->world_orientation.r_axis);
//
//					if(d < 0)
//					{
//						amount = -amount;
//					}
//				}
//
//				if(level_editor_angular_snap_value > 0.0)
//				{
//					d = amount / level_editor_angular_snap_value;
//					amount = level_editor_angular_snap_value * (int)d;
//				}
//
//				if(amount != 0.0)
//				{
//					if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
//					{
//						prev_dx = screen_dx;
//						prev_dy = screen_dy;
//					}
//				}
//			}
//
//			if(level_editor_editing_mode == EDITING_MODE_OBJECT)
//			{
//				switch(level_editor_3d_handle_transform_mode)
//				{
//					case ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION:
//						editor_LevelEditorTranslateSelections(direction, amount);
//					break;
//
//					case ED_3D_HANDLE_TRANSFORM_MODE_ROTATION:
//						editor_LevelEditorRotateSelections(direction, amount);
//					break;
//
//					case ED_3D_HANDLE_TRANSFORM_MODE_SCALE:
//						editor_LevelEditorScaleSelections(direction, amount);
//					break;
//				}
//
//			}
//
//		}
//
//		if(level_editor_pick_list.record_count)
//		{
//			level_editor_draw_3d_handle = 1;
//			editor_LevelEditorUpdate3dHandlePosition();
//		}
//		else
//		{
//			level_editor_draw_3d_handle = 0;
//		}
//
//		if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
//		{
//			editor_LevelEditorPickObject(normalized_mouse_x, normalized_mouse_y);
//		}





		//if(editor_state == EDITOR_EDITING)
		//{
//		if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
//		{
//			if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
//			{
//				if(level_editor_editing_mode == EDITING_MODE_OBJECT)
//				{
//					editor_LevelEditorOpenAddToWorldMenu(mouse_x, r_renderer.r_window_height - mouse_y);
//				}
//			}
//			else if(input_GetKeyStatus(SDL_SCANCODE_M) & KEY_JUST_PRESSED)
//			{
//				//editor_OpenMaterialWindow();
//				//editor_ToggleMaterialWindow();
//				editor_LevelEditorToggleMaterialsWindow();
//			}
//			else if(input_GetKeyStatus(SDL_SCANCODE_T) & KEY_JUST_PRESSED)
//			{
//				//editor_ToggleTextureWindow();
//			}
//			else if(input_GetKeyStatus(SDL_SCANCODE_V) & KEY_JUST_PRESSED)
//			{
//				//editor_ToggleEntityDefViewerWindow();
//			}
//			else if(input_GetKeyStatus(SDL_SCANCODE_D) & KEY_JUST_PRESSED)
//			{
//				if(level_editor_editing_mode == EDITING_MODE_OBJECT)
//				{
//					editor_LevelEditorCopySelections();
//				}
//
//			}
//			else if(input_GetKeyStatus(SDL_SCANCODE_C) & KEY_JUST_PRESSED)
//			{
//				editor_LevelEditorToggleMapCompilerWindow();
//			}
//			else if(input_GetKeyStatus(SDL_SCANCODE_L) & KEY_JUST_PRESSED)
//			{
//				editor_LevelEditorOpenWaypointOptionMenu(mouse_x, r_renderer.r_window_height - mouse_y);
//			}
//			else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
//			{
//				editor_LevelEditorOpenSnap3dCursorMenu(mouse_x, r_renderer.r_window_height - mouse_y);
//			}
//
//			if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
//			{
//				renderer_ToggleFullscreen();
//			}
//		}
//		else if(input_GetKeyStatus(SDL_SCANCODE_LCTRL) & KEY_PRESSED)
//		{
//            if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
//			{
//                editor_LevelEditorQuickSave();
//			}
//		}
//		else if(input_GetKeyStatus(SDL_SCANCODE_G) & KEY_JUST_PRESSED)
//		{
//			editor_LevelEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION);
//		}
//		else if(input_GetKeyStatus(SDL_SCANCODE_R) & KEY_JUST_PRESSED)
//		{
//			editor_LevelEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_ROTATION);
//		}
//		else if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_JUST_PRESSED)
//		{
//			editor_LevelEditorSet3dHandleTransformMode(ED_3D_HANDLE_TRANSFORM_MODE_SCALE);
//		}
//		else if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_JUST_PRESSED)
//		{
//			if(level_editor_editing_mode == EDITING_MODE_OBJECT)
//			{
//				editor_LevelEditorClearSelections();
//			}
//		}
//		else if(input_GetKeyStatus(SDL_SCANCODE_DELETE) & KEY_JUST_PRESSED)
//		{
//			if(level_editor_editing_mode == EDITING_MODE_OBJECT)
//			{
//				editor_LevelEditorOpenDeleteSelectionsMenu(mouse_x, r_renderer.r_window_height - mouse_y);
//			}
//		}
//		else if(input_GetKeyStatus(SDL_SCANCODE_TAB) & KEY_JUST_PRESSED)
//		{
//			record = editor_GetLastSelection(&level_editor_pick_list);
//
//			if(record.type == PICK_BRUSH)
//			{
//				if(level_editor_editing_mode == EDITING_MODE_BRUSH || level_editor_editing_mode == EDITING_MODE_UV)
//				{
//					editor_LevelEditorSetEditingMode(EDITING_MODE_OBJECT);
//				}
//				else
//				{
//					editor_LevelEditorSetEditingMode(EDITING_MODE_BRUSH);
//				}
//			}
//
//
//			/*if(ed_editing_mode == EDITING_MODE_BRUSH || ed_editing_mode == EDITING_MODE_UV)
//			{
//				editor_SetEditingMode(EDITING_MODE_OBJECT);
//				ed_selected_brush_polygon_index = -1;
//			}
//			else
//			{
//				if(ed_selection_count)
//				{
//					if(ed_selections[ed_selection_count - 1].type == PICK_BRUSH)
//					{
//						editor_SetEditingMode(EDITING_MODE_BRUSH);
//					}
//				}
//			}*/
//		}
//	}
//	else
//	{
//
//		//entity = entity_GetEntityPointer("player entity", 0);
//
//		entity = entity_GetEntityHandle("player entity", 0);
//		entity_ptr = entity_GetEntityPointerHandle(entity);
//
//        if(entity_ptr)
//		{
//
//			//life_prop = entity_GetPropPointer()
//
//			entity_GetProp(entity, "life", &life);
//
//			gui_ImGuiSetNextWindowPos(vec2(0.0, r_renderer.r_window_height - 60.0), ImGuiCond_Always, vec2(0.0, 0.0));
//			gui_ImGuiBegin("Life", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
//
//			gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));
//
//			gui_ImGuiText("Life: %d", life);
//
//			gui_ImGuiPopFont();
//			gui_ImGuiEnd();
//		}
	}
//



	if(input_GetKeyStatus(SDL_SCANCODE_P) & KEY_JUST_PRESSED)
	{
		editor_LevelEditorStartPIE();
	}

	else if(input_GetKeyStatus(SDL_SCANCODE_ESCAPE) & KEY_JUST_PRESSED)
	{
		editor_LevelEditorStopPIE();
	}



	//}
}



void editor_LevelEditorFly(float delta_time)
{

	vec3_t forward_vector;
	vec3_t right_vector;
	vec3_t translation = {0.0, 0.0, 0.0};
	vec3_t velocity = {0.0, 0.0, 0.0};

	struct view_def_t *view;


	if(engine_GetEngineState() & ENGINE_PAUSED)
	{
		level_editor_state.editor_camera_yaw -= (mouse_dx) * 0.5;
		level_editor_state.editor_camera_pitch += (mouse_dy) * 0.5;


		if(level_editor_state.editor_camera_pitch > 0.5) level_editor_state.editor_camera_pitch = 0.5;
		else if(level_editor_state.editor_camera_pitch < -0.5) level_editor_state.editor_camera_pitch = -0.5;

		if(level_editor_state.editor_camera_yaw > 1.0) level_editor_state.editor_camera_yaw = -1.0 + (level_editor_state.editor_camera_yaw - 1.0);
		else if(level_editor_state.editor_camera_yaw < -1.0) level_editor_state.editor_camera_yaw = 1.0 + (level_editor_state.editor_camera_yaw + 1.0);

		renderer_PitchYawView(level_editor_state.editor_view, level_editor_state.editor_camera_yaw, level_editor_state.editor_camera_pitch);

		view = renderer_GetViewPointer(level_editor_state.editor_view);

		forward_vector = view->world_orientation.f_axis;
		right_vector = view->world_orientation.r_axis;

		if(input_GetKeyStatus(SDL_SCANCODE_W) & KEY_PRESSED)
		{
			translation.x -= forward_vector.x;
			translation.y -= forward_vector.y;
			translation.z -= forward_vector.z;
		}

		if(input_GetKeyStatus(SDL_SCANCODE_S) & KEY_PRESSED)
		{
			translation.x += forward_vector.x;
			translation.y += forward_vector.y;
			translation.z += forward_vector.z;
		}

		if(input_GetKeyStatus(SDL_SCANCODE_A) & KEY_PRESSED)
		{
			translation.x -= right_vector.x;
			translation.y -= right_vector.y;
			translation.z -= right_vector.z;
		}

		if(input_GetKeyStatus(SDL_SCANCODE_D) & KEY_PRESSED)
		{
			translation.x += right_vector.x;
			translation.y += right_vector.y;
			translation.z += right_vector.z;
		}

		if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_PRESSED)
		{
			translation.y += 0.8;
		}

		if(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED)
		{
			translation.y -= 0.8;
		}

		if(input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED)
		{
			velocity.y = 0.08;
		}

		if(bm_mouse & MOUSE_WHEEL_UP)
		{
			level_editor_state.editor_camera_speed_multiplier += 0.05;
		}
		else if(bm_mouse & MOUSE_WHEEL_DOWN)
		{
			level_editor_state.editor_camera_speed_multiplier -= 0.05;
			if(level_editor_state.editor_camera_speed_multiplier < 0.05) level_editor_state.editor_camera_speed_multiplier = 0.05;
		}

		translation.x *= level_editor_state.editor_camera_speed_multiplier * delta_time * 0.045;
		translation.y *= level_editor_state.editor_camera_speed_multiplier * delta_time * 0.045;
		translation.z *= level_editor_state.editor_camera_speed_multiplier * delta_time * 0.045;

		velocity.x = translation.x;
		velocity.y = translation.y;
		velocity.z = translation.z;

		renderer_TranslateView(level_editor_state.editor_view, velocity, 1.0, 0);
	}


}


/*
====================================================================
====================================================================
====================================================================
*/

//void editor_LevelEditorAddSelection(pick_record_t *record)
//{
//	editor_AddSelection(record, &level_editor_pick_list);
//}
//
//void editor_LevelEditorDropSelection(pick_record_t *record)
//{
//	editor_DropSelection(record, &level_editor_pick_list);
//}
//
//void editor_LevelEditorClearSelections()
//{
//	editor_ClearSelection(&level_editor_pick_list);
//}
//
//void editor_LevelEditorCopySelections()
//{
//	editor_CopySelections(&level_editor_pick_list);
//}
//
//void editor_LevelEditorDestroySelections()
//{
//	editor_DestroySelection(&level_editor_pick_list);
//}

//pick_record_t editor_LevelEditorGetLastSelection()
//{
////	if(level_editor_pick_list.record_count)
////	{
////		return level_editor_pick_list.records[level_editor_pick_list.record_count - 1];
////	}
////
////	return (pick_record_t){PICK_NONE, 0, NULL, 0, 0};
//
//}

/*
====================================================================
====================================================================
====================================================================
*/

//void editor_LevelEditorTranslateSelections(vec3_t direction, float amount)
//{
//	//editor_TranslateSelections(&level_editor_pick_list, direction, amount);
//	level_editor_need_to_copy_data = 1;
//}
//
//void editor_LevelEditorRotateSelections(vec3_t axis, float amount)
//{
//	//editor_RotateSelections(&level_editor_pick_list, axis, amount, level_editor_3d_handle_position);
//	level_editor_need_to_copy_data = 1;
//}
//
//void editor_LevelEditorScaleSelections(vec3_t axis, float amount)
//{
//	//editor_ScaleSelections(&level_editor_pick_list, axis, amount);
//	level_editor_need_to_copy_data = 1;
//}

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorStartPIE()
{
	editor_LevelEditorCopyLevelData();
	engine_SetEngineState(ENGINE_PLAYING);
	level_editor_state.editor_state = EDITOR_PIE;

//	entity_ResetEntitySpawnTimes();
}

void editor_LevelEditorStopPIE()
{
	engine_SetEngineState(ENGINE_PAUSED);
	editor_LevelEditorClearLevelData();
	editor_LevelEditorRestoreLevelData();
	level_editor_state.editor_state = EDITOR_EDITING;
//	camera_SetCamera(level_editor_camera);
    renderer_SetMainView(level_editor_state.editor_view);

	sound_StopAllSounds();
}

/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorCopyLevelData()
{
	int i;

	struct world_level_t *current_level;
	void *read_buffer;

	if(level_editor_need_to_copy_data)
	{
		level_editor_brushes = brushes;
		level_editor_last_brush = last_brush;
		level_editor_brush_count = brush_count;

		world_SerializeWorld(&level_editor_world_buffer, &level_editor_world_buffer_size);
        read_buffer = level_editor_world_buffer;
		current_level = world_GetCurrentLevel();
		serializer_FreeSerializer(&current_level->serializer, 1);
		current_level->serializer = world_DeserializeWorld(&read_buffer);

		memory_Free(level_editor_world_buffer);

		level_editor_has_copied_data = 1;
		level_editor_need_to_copy_data = 0;
	}
}

void editor_LevelEditorClearLevelData()
{
	if(level_editor_has_copied_data && (!level_editor_need_to_copy_data))
	{
		brushes = NULL;
		last_brush = NULL;
		brush_count = 0;

        world_Clear(WORLD_CLEAR_FLAG_ALL & (~WORLD_CLEAR_FLAG_ENTITY_DEFS));
	}
}

void editor_LevelEditorRestoreLevelData()
{
	int i;

	struct entity_def_t *entity_def;
	struct entity_t *entity;
	struct collider_def_t *collider_def;
	struct collider_t *collider;

	struct world_level_t *current_level;

	void *read_buffer;

	if(level_editor_has_copied_data && (!level_editor_need_to_copy_data))
	{
		brushes = level_editor_brushes;
		last_brush = level_editor_last_brush;
		brush_count = level_editor_brush_count;

		world_ChangeLevel("current_level");
	}
}


/*
====================================================================
====================================================================
====================================================================
*/

static char level_editor_buffer_start_tag[] = "[buffer start]";

struct level_editor_buffer_start_t
{
    char tag[(sizeof(level_editor_buffer_start_tag) + 3) & (~3)];
};

static char level_editor_buffer_end_tag[] = "[buffer end]";

struct level_editor_buffer_end_t
{
	char tag[(sizeof(level_editor_buffer_end_tag) + 3) & (~3)];
};

void editor_LevelEditorSerialize(void **buffer, int *buffer_size)
{
	char *out = NULL;
	int out_size = 0;

	struct serializer_t serializer;

    void *brush_buffer;
    int brush_buffer_size;

	void *world_buffer;
	int world_buffer_size;

    brush_SerializeBrushes(&brush_buffer, &brush_buffer_size);
    world_SerializeWorld(&world_buffer, &world_buffer_size);

    memset(&serializer, 0, sizeof(struct serializer_t));

    serializer_AddEntry(&serializer, "brush buffer", brush_buffer_size, brush_buffer);
    serializer_AddEntry(&serializer, "world buffer", world_buffer_size, world_buffer);
    serializer_Serialize(&serializer, buffer, buffer_size);
    serializer_FreeSerializer(&serializer, 1);
}


extern char waypoint_section_start_tag[];
extern char brush_section_start_tag[];
extern char light_section_start_tag[];


void editor_LevelEditorDeserialize(void **buffer, char *file_name)
{
    struct serializer_t serializer;

    memset(&serializer, 0, sizeof(struct serializer_t));

    void *read_buffer = *(char **)buffer;

    serializer_Deserialize(&serializer, &read_buffer);


    struct serializer_entry_t *brush_entry;
    struct serializer_entry_t *world_entry;

    brush_entry = serializer_GetEntry(&serializer, "brush buffer");
    world_entry = serializer_GetEntry(&serializer, "world buffer");

    if(brush_entry)
    {
        read_buffer = brush_entry->entry_buffer;
        brush_DeserializeBrushes(&read_buffer);
    }

    if(world_entry)
    {
        read_buffer = world_entry->entry_buffer;
        struct world_level_t *level = world_LoadLevelFromMemory("current_level", &read_buffer);
        world_ChangeLevel("current_level");
    }
}



void editor_LevelEditorNewLevel()
{
    struct serializer_t serializer;
    struct world_level_t *level;
    struct world_script_t *script;

    editor_LevelEditorClearLevel();

    serializer.entries = NULL;
    serializer.entry_count = 0;
    serializer.last_entry = NULL;

    level = world_CreateLevel("current_level", serializer, NULL);
    world_ChangeLevel("current_level");
}

void editor_LevelEditorClearLevel()
{
    world_Clear(WORLD_CLEAR_FLAG_ALL);
    brush_DestroyAllBrushes();
    script_DestroyAllScripts();

    level_editor_has_copied_data = 0;
    level_editor_need_to_copy_data = 0;

    editor_LevelEditorSetNameAndPath(NULL, NULL);

    world_DestroyLevel("current_level");
}

//int editor_LevelEditorWriteLevelFile(char *file_path, char *file_name)
//{
//    FILE *file;
//
//    char path[PATH_MAX];
//
//    void *buffer = NULL;
//    int buffer_size = 0;
//
//    int i;
//
//	editor_LevelEditorSetNameAndPath(file_name, file_path);
//
//    editor_LevelEditorSerialize(&buffer, &buffer_size);
//
//    strcpy(path, level_editor_level_path);
//    strcat(path, "/");
//    strcat(path, level_editor_level_name);
//
//    file = fopen(path, "wb");
//    fwrite(buffer, buffer_size, 1, file);
//    fclose(file);
//
//    memory_Free(buffer);
//
//	return 0;
//}



int editor_LevelEditorSaveLevel(char *file_path, char *file_name, void **file_buffer, int *file_buffer_size)
{
    FILE *file;
    FILE *path_file;

    char path[PATH_MAX];
    char sub_path[PATH_MAX];
    char copy_path[PATH_MAX];

    void *buffer = NULL;
    int buffer_size = 0;

    struct entity_handle_t *entity_defs;
    int entity_defs_count;

    struct texture_t *texture;
    struct model_t *model;

    struct world_script_t *world_script;

    int i;
    int j;

	editor_LevelEditorSetNameAndPath(path_AddExtToName(file_name, "wtf"), file_path);

	strcpy(path, level_editor_level_path);
	strcat(path, "/");
	strcat(path, path_GetNameNoExt(level_editor_level_name));

	if(!path_CheckDir(path))
    {
        path_MakeDir(path);
    }

    strcpy(sub_path, path);
    strcat(sub_path, "/");
    strcat(sub_path, "path.cfg");

    path_file = fopen(sub_path, "w");

    i = 0;

    for(i = LEVEL_EDITOR_FOLDER_FIRST; i < LEVEL_EDITOR_FOLDER_LAST; i++)
    {
        strcpy(sub_path, path);
        strcat(sub_path, "/");
        strcat(sub_path, level_editor_level_folder_names[i]);

        if(!path_CheckDir(sub_path))
        {
            path_MakeDir(sub_path);
        }

        fprintf(path_file, "[%s]\n", level_editor_level_folder_names[i]);

        switch(i)
        {
            case LEVEL_EDITOR_FOLDER_TEXTURES:
                for(j = 0; j < tex_textures.element_count; j++)
                {
                    texture = texture_GetTexturePointer(j);

                    if(texture)
                    {
                        strcpy(copy_path, sub_path);
                        strcat(copy_path, "/");
                        strcat(copy_path, texture->texture_info->file_name);
                        path_CopyFile(texture->texture_info->full_path, copy_path);
                    }
                }
            break;

            case LEVEL_EDITOR_FOLDER_MODELS:
                for(j = 0; j < mdl_models.element_count; j++)
                {
                    model = model_GetModelPointerIndex(j);

                    if(model)
                    {
                        strcpy(copy_path, sub_path);
                        strcat(copy_path, "/");
                        strcat(copy_path, model->file_name);
                        path_CopyFile(model->full_path, copy_path);
                    }
                }
            break;

            case LEVEL_EDITOR_FOLDER_SOUNDS:

            break;

            case LEVEL_EDITOR_FOLDER_SCRIPTS:

            break;

            case LEVEL_EDITOR_FOLDER_ENTITIES:

                entity_defs = entity_GetEntityDefs(&entity_defs_count);

                for(j = 0; j < entity_defs_count; j++)
                {
                    entity_SaveEntityDef(sub_path, entity_defs[j]);
                }
            break;
        }
    }

    fclose(path_file);

    strcat(path, "/");
    strcat(path, level_editor_level_name);

    editor_LevelEditorSerialize(&buffer, &buffer_size);

    file = fopen(path, "wb");
    fwrite(buffer, buffer_size, 1, file);
    fclose(file);

    memory_Free(buffer);

	return 0;
}

int editor_LevelEditorQuickSave()
{
    if(!(level_editor_level_name[0] && level_editor_level_path[0]))
    {
        editor_SetExplorerWriteFileCallback(editor_LevelEditorSaveLevel);
        editor_OpenExplorerWindow(NULL, EXPLORER_FILE_MODE_WRITE);
    }
    else
    {
        editor_LevelEditorSaveLevel(level_editor_level_path, level_editor_level_name, NULL, NULL);
    }
}

int editor_LevelEditorLoadLevel(char *path, char *file_name)
{
	char file_path[PATH_MAX];
	char folder_path[PATH_MAX];
	unsigned long file_size;
	void *file_buffer;
	void *read_buffer;

	FILE *file;

	/*strcpy(level_editor_level_name, path);
	strcat(level_editor_level_name, "/");
	strcat(level_editor_level_name, path_AddExtToName(file_name, ".wtf"));*/

	if(!strcmp(path_GetFileExtension(file_name), "wtf"))
    {
        editor_LevelEditorClearLevel();

        if(!strcmp(path_GetNameNoExt(file_name), path_GetLastPathSegment(path)))
        {
            strcpy(folder_path, path_DropPathSegment(path, 0));
        }
        else
        {
            strcpy(folder_path, path);
        }

        editor_LevelEditorSetNameAndPath(path_AddExtToName(file_name, ".wtf"), folder_path);

        /*strcpy(file_path, path);
        strcat(file_path, "/");
        strcat(file_path, file_name);*/
        strcat(folder_path, "/");
        strcat(folder_path, path_GetNameNoExt(file_name));

        //strcpy(file_path, folder_path);
        //strcat(file_path, "/");
        //strcat(file_path, "path.cfg");

        path_ReadCfg(folder_path);

        strcpy(file_path, folder_path);
        strcat(file_path, "/");
        strcat(file_path, level_editor_level_name);


        file = fopen(file_path, "rb");

        if(file)
        {
            file_size = path_GetFileSize(file);

            file_buffer = memory_Calloc(file_size, 1);
            fread(file_buffer, file_size, 1, file);
            fclose(file);

            read_buffer = file_buffer;

            editor_LevelEditorDeserialize(&read_buffer, file_name);

            memory_Free(file_buffer);

            level_editor_need_to_copy_data = 1;

            return 1;


        }
    }

	return 0;
}

int editor_LevelEditorExportBsp(char *file_path, char *file_name, void **file_buffer, int *file_buffer_size)
{
	char full_path[PATH_MAX];

	strcpy(full_path, file_path);
	strcat(full_path, "/");
	strcat(full_path, path_AddExtToName(file_name, "bsp"));
	bsp_SaveBsp(full_path);

	return 0;
}

int editor_LevelEditorImportBsp(char *file_path, char *file_name)
{

}


void editor_LevelEditorSetNameAndPath(char *level_name, char *level_path)
{
    if(level_name && level_path)
    {
        strcpy(level_editor_level_name, level_name);
        strcpy(level_editor_level_path, level_path);
    }
    else
    {
        /*level_name = "untitled";
        level_path = "";*/
        strcpy(level_editor_level_name, "untitled");
        level_editor_level_path[0] = '\0';
    }
}

int editor_LevelEditorIsPathComplete()
{
    return level_editor_level_name[0] && level_editor_level_path[0];
}














