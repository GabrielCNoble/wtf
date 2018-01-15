#ifndef EDITOR_H
#define EDITOR_H

#include "vector.h"

#define ROTATION_HANDLE_DIVS 32

enum PICK_TYPE
{
	PICK_NONE = 0,
	PICK_HANDLE,				/* 3d manipulation handle */
	PICK_BRUSH,
	PICK_LIGHT,
};

enum HANDLE_3D_FLAGS
{
	HANDLE_3D_GRABBED_X_AXIS = 1,
	HANDLE_3D_GRABBED_Y_AXIS = 1 << 1,
	HANDLE_3D_GRABBED_Z_AXIS = 1 << 2,
};

enum HANDLE_3D_POSITION_MODE
{
	HANDLE_3D_ACTIVE_OBJECT_ORIGIN = 1,
	HANDLE_3D_MEDIAN_POINT
};

enum HANDLE_3D_MODE
{
	HANDLE_3D_TRANSLATION = 1,
	HANDLE_3D_ROTATION,
	HANDLE_3D_SCALE,
};

typedef struct
{
	int type;
	int index0;
	int index1;
	int index2;
}pick_record_t;


void editor_Init();

void editor_Finish();

void editor_Main(float delta_time);

void editor_ProcessMouse();

void editor_TranslateSelections(vec3_t direction, float amount);

void editor_RotateSelections(vec3_t axis, float amount, int individual_origins);

void editor_EnablePicking();

void editor_DisablePicking();

int editor_Pick(pick_record_t *record);

int editor_Check3dHandle();

void editor_AddSelection(pick_record_t *record);

void editor_DropSelection(pick_record_t *record);

void editor_ClearSelection();

void editor_ExportMap(char *file_name);

void editor_WindowResizeCallback();


#endif 
