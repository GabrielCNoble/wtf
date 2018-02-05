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

enum EDITOR_STATE
{
	EDITOR_EDITING = 1,
	EDITOR_PIE,
};

/*float editor_snappings[] = 
{
	0.0, 
	0.001,
	0.005,
	0.01,
	0.05,
	0.1,
	0.5,
	1.0,
	5.0,
	10.0,
};*/

typedef struct
{
	int type;
	int index0;
	int index1;
	int index2;
}pick_record_t;


typedef struct
{
	unsigned short *radius;
	unsigned short *energy;
	unsigned char *r;
	unsigned char *g;
	unsigned char *b;
}light_ptr_t;

typedef struct
{
	int *type;
	int *vertex_count;
	int *polygon_count;
	int *triangle_group_count;
}brush_ptr_t;



#endif







