#ifndef ED_PROJ_H
#define ED_PROJ_H

#include "vector.h"
#include "matrix.h"
#include "camera.h"

#define PROJ_EXT ".wtf"
#define PROJ_VERSION 0

typedef struct
{
	int version;
	int brush_count;
	int light_count;
	int camera_count;
	char *active_camera;
}proj_header_t;

typedef struct
{
	mat3_t orientation;
	vec3_t position;
	vec3_t scale;
	short type;
	short bm_flags;
}brush_lump_t;


void editor_SaveProject(char *file_name);

void editor_LoadProject(char *file_name);

void editor_ExportBsp(char *file_name);

#endif
