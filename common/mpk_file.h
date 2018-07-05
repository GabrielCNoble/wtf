#ifndef MPK_FILE_H
#define MPK_FILE_H


#include "bsp_file.h"
#include "model.h"
#include <limits.h>

#define MPK_CONSTANT0 0x006b706d
#define MPK_CONSTANT1 0x6d706b00

#define MPK_VERSION 0

typedef struct
{
	int mpk0;
	int mpk1;
	int version;
	int vertice_count;
	int vertex_record_count;
	int material_count;
	
	int reserved0;
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
	int reserved5;
	int reserved6;
	int reserved7;
}mpk_header_t;

typedef struct
{
	char material_name[PATH_MAX];
	int vertice_count;
	int offset;
}mpk_vertex_record_t;



/*********************************************************************

MPK file structure...


===============
header
===============
MPK_CONSTANT0
MPK_CONSTANT1
MPK_VERSION
vertice count


===============
data
===============

material_name0
vertice_count0
offset0


material_name1
vertice_count1
offset1


material_name2
vertice_count2
offset2

.
.
.




**********************************************************************/

void read_mpk(char *file_name);

void write_mpk(char *file_name);


#endif




