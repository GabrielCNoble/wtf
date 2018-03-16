#ifndef MPK_FILE_H
#define MPK_FILE_H


#include "bsp_file.h"

#define MPK_CONSTANT0 0x006b706d
#define MPK_CONSTANT1 0x6d706b00

#define MPK_VERSION 0

#define MPK_MAX_NAME_LEN 512


typedef struct
{
	int mpk0;
	int mpk1;
	int version;
	int vertice_count;
	int vertex_record_count;
	int material_count;
	int texture_count;
}mpk_header_t;

typedef struct
{
	int material_index;
	int vertice_count;
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
material count
texture count


===============
textures
===============
texture_record0
	* bm_texture_flags - always present, may be 0.
	* file_name - null terminated string that represents the full path to the file, and follows the bm_texture_flags field. Can be as long as 
	BSP_FILE_MAX_NAME_LEN.
	* name - null terminated string that names the texture, and follows right after file_name on memory. Can be as long as BSP_FILE_MAX_NAME_LEN.





===============
materials and vertices - those are just laid right after the header, one after another...
===============
material_0
	* base - base color (vec4_t)
	* bm_flags - flags of this material
		- if the flag MATERIAL_USE_CUSTOM_SHADER is present, the name of the shader to be used will be right after the name of the material in memory
		
		- if the flag MATERIAL_USE_DIFFUSE_TEXTURE is present, the name of the diffuse texture file will be right after the name of the custom shader
		or the name of the material in memory...
		
		- if the flag MATERIAL_USE_NORMAL_TEXTURE is present, the name of the normal texture file will follow the last field in memory, this being
		the diffuse texture name, the name field or the custom shader name field...
		
	* name - the name of the material. Although it apears as a single char, in reality the name can be as long as BSP_FILE_MAX_NAME_LEN, given that
	the material_record_t is used to extract the data from a memory buffer once it gets pulled from disk into ram. 
mpk_vertex_record_0
	* vertice_count - how many vertices under this record




material_1
mpk_vertex_record_1
	* vertice_count - how many vertices under this record
	
	
material_2
mpk_vertex_record_2
	* vertice_count - how many vertices under this record
	
	
material_3
mpk_vertex_record_3
	* vertice_count - how many vertices under this record
.
.
.

mpk_vertex_record_-1	(indigent vertices that use the default material)
	* vertice_count - how many vertices under this record



**********************************************************************/

#endif
