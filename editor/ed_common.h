#ifndef ED_COMMON_H
#define ED_COMMON_H

#include "vector.h"

#define ROTATION_HANDLE_DIVS 32

enum PICK_TYPE
{
	PICK_NONE = 0,
	PICK_HANDLE,				/* 3d manipulation handle */
	PICK_BRUSH,
	PICK_LIGHT,
	PICK_SPAWN_POINT,
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
	HANDLE_3D_TRANSLATION = 0,
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



#endif
