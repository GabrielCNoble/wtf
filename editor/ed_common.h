#ifndef ED_COMMON_H
#define ED_COMMON_H

#include "vector.h"

#define ROTATION_HANDLE_DIVS 32



enum ED_3D_HANDLE_FLAGS
{
	ED_3D_HANDLE_X_AXIS_GRABBED = 1,
	ED_3D_HANDLE_Y_AXIS_GRABBED = 1 << 1,
	ED_3D_HANDLE_Z_AXIS_GRABBED = 1 << 2,
};

enum ED_3D_HANDLE_PIVOT_MODE
{
	ED_3D_HANDLE_PIVOT_MODE_ACTIVE_OBJECT_ORIGIN = 0,
	ED_3D_HANDLE_PIVOT_MODE_MEDIAN_POINT
};

enum ED_3D_HANDLE_TRANSFORM_MODE
{
	ED_3D_HANDLE_TRANSFORM_MODE_TRANSLATION = 0,
	ED_3D_HANDLE_TRANSFORM_MODE_ROTATION,
	ED_3D_HANDLE_TRANSFORM_MODE_SCALE,
};

enum ED_3D_HANDLE_TRANSFORM_ORIENTATION
{
	ED_3D_HANDLE_TRANSFORM_ORIENTATION_GLOBAL = 0,
	ED_3D_HANDLE_TRANSFORM_ORIENTATION_LOCAL,
};



/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/

//enum LEVEL_EDITOR_HANDLE_TRANSFORM_TYPE
//{
//	HANDLE_TRANSFORM_TYPE_TRANSLATION = 0,
//	HANDLE_TRANSFORM_TYPE_ROTATION,
//	HANDLE_TRANSFORM_TYPE_SCALE,
//};
//
//enum LEVEL_EDITOR_HANDLE_TRANSFORM_ORIENTATION
//{
//	HANDLE_TRANSFORM_ORIENTATION_LOCAL,
//	HANDLE_TRANSFORM_ORIENTATION_GLOBAL,
//};
//
//enum LEVEL_EDITOR_HANDLE_PIVOT_MODE
//{
//	HANDLE_PIVOT_MODE_ACTIVE_OBJECT_ORIGIN = 1,
//	HANDLE_PIVOT_MODE_MEDIAN_POINT,
//	HANDLE_PIVOT_MODE_3D_CURSOR
//};

enum LEVEL_EDITOR_EDITING_MODE
{
	EDITING_MODE_OBJECT,
	EDITING_MODE_BRUSH,
	EDITING_MODE_UV,
};

/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/
/************************************************************************************************************/


enum HANDLE_3D_TRANFORM_MODE
{
	HANDLE_3D_TRANFORM_GLOBAL = 0,
	HANDLE_3D_TRANFORM_LOCAL,
};

enum EDITOR_STATE
{
	EDITOR_EDITING = 1,
	EDITOR_PIE,
};

/*enum EDITING_MODE
{
	EDITING_MODE_OBJECT = 0,
	EDITING_MODE_BRUSH,
	EDITING_MODE_UV,
};*/

enum EDITOR_TEXTURE_FLAGS
{
	TEXTURE_COPY = 1 << 28,					/* used to signal a texture should be copied to the project folder... */
};

enum EDITORS
{
	EDITOR_LEVEL_EDITOR,
	EDITOR_ENTITY_EDITOR,
	EDITOR_MATERIAL_EDITOR,
	EDITOR_PARTICLE_SYSTEM_EDITOR,
};




enum EDITOR_ACTION_RECORD_TYPE
{
	EDITOR_ACTION_NONE = 0,
	EDITOR_ACTION_ADD_OBJECT,
	EDITOR_ACTION_REMOVE_OBJECT,
	EDITOR_ACTION_TRANSFORM,
	EDITOR_ACTION_SELECTION
};

typedef struct editor_action_object_t
{
	struct editor_action_object_t *next;
	struct editor_action_object_t *prev;

	int type;
	int index;
	void *pointer;
	void *data;

}editor_action_object_t;

typedef struct editor_action_record_t
{
	struct editor_action_record_t *next;
	struct editor_action_record_t *prev;
	int type;
	editor_action_object_t *object;

}editor_action_record_t;

typedef struct editor_t
{
	struct editor_t *next;

	char *name;

	void (*init_callback)();
	void (*finish_callback)();
	void (*restart_callback)();
	void (*setup_callback)();
	void (*shutdown_callback)();
	void (*main_callback)(float);

	void *editor_data;

//	vec3_t manipulator_position;
//	vec3_t cursor_position;

	//struct pick_list_t pick_list;

}editor_t;



#endif







