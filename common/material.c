#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "material.h"
#include "vector.h"
#include "c_memory.h"
//#include "bsp_file.h"
#include "texture.h"



static int mat_material_list_size = 0;
static int mat_material_list_cursor = 0;
int mat_material_count = 0;
static int mat_free_position_stack_top = 0;
static int *mat_free_position_stack = 0;
static int mat_frame_count = 0;

material_t *mat_materials = NULL;
char **mat_material_names = NULL;

int mat_material_name_record_count = 0;
material_name_record_t *mat_material_name_records = NULL;


material_t *default_material;
char *default_material_name = "default material";




/* from r_main.c */
extern int r_z_pre_pass_shader;
extern int r_forward_pass_shader;
extern int r_frame;


#ifdef __cplusplus
extern "C"
{
#endif

int material_Init()
{
	mat_material_list_size = MAX_MATERIALS;
	mat_material_count = 0;
	mat_free_position_stack_top = -1;

	mat_material_name_records = memory_Malloc(sizeof(material_name_record_t) * mat_material_list_size);
	mat_materials = memory_Malloc(sizeof(material_t) * (mat_material_list_size + 1));
	mat_material_names = memory_Malloc(sizeof(char **) * (mat_material_list_size + 1));
	mat_free_position_stack = memory_Malloc(sizeof(int) * mat_material_list_size);

	default_material = mat_materials;

	mat_materials++;
	mat_material_names++;

	mat_material_names[-1] = "default";
	default_material_name = mat_material_names[-1];

	default_material->r = 255;
	default_material->g = 255;
	default_material->b = 255;
	default_material->a = 255;

	default_material->roughness = 10;
	default_material->metalness = 0;

	default_material->shader_index = r_forward_pass_shader;
	default_material->diffuse_texture = -1;
	default_material->normal_texture = -1;
	default_material->height_texture = -1;
	default_material->metalness_texture = -1;
	default_material->roughness_texture = -1;
	default_material->ref_count = 0;
	default_material->flags = 0;
	default_material->draw_group = 0;


	return 1;
}

void material_Finish()
{

	int i;
	for(i = -1; i < mat_material_list_cursor; i++)
	{
		material_DestroyMaterialIndex(i);
	}

	mat_materials--;
	mat_material_names--;

	memory_Free(mat_materials);
	memory_Free(mat_material_names);
	memory_Free(mat_material_name_records);
	memory_Free(mat_free_position_stack);
}


void material_GetNameBaseAndSuffix(char *name, char *base, char *suffix, int *suffix_pos, int *suffix_byte_index, int *suffix_bit_index)
{
	int i;

	int j;
	int point_index = 0;
	int name_len;
	int suffix_index = 0;
	int byte_index = 0;
	int bit_index = 0;

	name_len = strlen(name);

	for(point_index = name_len; point_index > 0; point_index--)
	{
		if(name[point_index] == '.')
		{
			break;
		}
	}

	if(point_index)
	{
		for(i = 0; i < point_index; i++)
		{
			base[i] = name[i];
		}

		base[point_index] = '\0';


		suffix_index = 0;
		for(point_index++; point_index <= name_len; point_index++)
		{
			suffix[suffix_index] = name[point_index];
			suffix_index++;
		}

		suffix_index = atoi(suffix);
		byte_index = suffix_index >> 3;
		bit_index = suffix_index % 8;

		if(!suffix_index)
		{
			/* name.0000 is not allowed, given
			that the base name already represents
			that... */
			strcpy(base, name);
		}
	}
	else
	{
		strcpy(base, name);
	}

	*suffix_pos = point_index;
	*suffix_byte_index = byte_index;
	*suffix_bit_index = bit_index;
}


char base_name[512];
char suffix[8];

char *material_AddNameRecord(char *name)
{
	int i;

	int j;
	int point_index = 0;
	int name_len;
	int suffix_index = 0;
	int suffix_byte_index = 0;
	int suffix_bit_index = 0;
	material_name_record_t *record;

	material_GetNameBaseAndSuffix(name, base_name, suffix, &point_index, &suffix_byte_index, &suffix_bit_index);

	for(i = 0; i < mat_material_name_record_count; i++)
	{
		if(!strcmp(mat_material_name_records[i].base_name, base_name))
		{
			break;
		}
	}

	if(i >= mat_material_name_record_count)
	{
		/* name doesn't exist, so create a new record... */
		record = &mat_material_name_records[mat_material_name_record_count];
		mat_material_name_record_count++;

		record->base_name = memory_Strdup(base_name);

		for(j = 0; j < MAX_MATERIALS >> 3; j++)
		{
			record->used_suffixes[j] = 0;
		}

		strcpy(base_name, name);
	}
	else
	{
		record = &mat_material_name_records[i];

		/* check if the name is already in use... */
		if(!(record->used_suffixes[suffix_byte_index] & (1 << suffix_bit_index)))
		{
			/* not in use, so just return the original name... */
			strcpy(base_name, name);
		}
		else
		{
			/* this name is already in use, so go over
			the bitvector and find the first unused
			suffix... */
			for(j = 0; j < MAX_MATERIALS >> 3; j++)
			{
				suffix_byte_index = j >> 3;
				suffix_bit_index = j % 8;

				if(!(record->used_suffixes[suffix_byte_index] & (1 << suffix_bit_index)))
				{
					/* found an unused suffix... */
					break;
				}
			}

			/* add the new suffix to
			this name, and return it... */
			sprintf(suffix, ".%04d", j);
			strcat(base_name, suffix);
		}
	}

	/* mark this suffix as being used... */
	record->used_suffixes[suffix_byte_index] |= 1 << suffix_bit_index;

	return base_name;

}

void material_RemoveNameRecord(char *name)
{
	material_name_record_t *record;
	int point_index;
	int suffix_byte_index;
	int suffix_bit_index;

	int i;

	material_GetNameBaseAndSuffix(name, base_name, suffix, &point_index, &suffix_byte_index, &suffix_bit_index);

	for(i = 0; i < mat_material_name_record_count; i++)
	{
		if(!strcmp(mat_material_name_records[i].base_name, base_name))
		{
			break;
		}
	}

	if(i >= mat_material_name_record_count)
	{
		/* no material name record with this base
		name, so do nothing... */
		return;
	}
	else
	{
		/* mark this suffix as not used... */
		record = &mat_material_name_records[i];
		record->used_suffixes[suffix_byte_index] &= ~(1 << suffix_bit_index);

		/* check to see if there's any reference to this name... */
		for(i = 0; i < MAX_MATERIALS >> 3; i++)
		{
			if(record->used_suffixes[i])
			{
				return;
			}
		}

		/* this record has no references, so get rid of it... */
		memory_Free(record->base_name);

		if(i < mat_material_name_record_count - 1)
		{
			mat_material_name_records[i] = mat_material_name_records[mat_material_name_record_count - 1];
		}
		mat_material_name_record_count--;
	}
}





int material_CreateMaterial(char *name, vec4_t base_color, float metalness, float roughness, short shader_index, short diffuse_texture, short normal_texture)
{
	char **material_name;
	material_t *material;
	int *itemp;
	int material_index;
	int material_name_len;
	int i;
	int j;


	if(mat_free_position_stack_top >= 0)
	{
		material_index = mat_free_position_stack[mat_free_position_stack_top--];
	}
	else
	{
		material_index = mat_material_list_cursor++;

		if(material_index >= MAX_MATERIALS)
		{
			printf("material_CreateMaterial: no more materials!\n");
			return -1;
		}

		/*if(material_index > material_list_size)
		{
			free(free_position_stack);

			material = malloc(sizeof(material_t) * (material_list_size + 16));
			material_name = malloc(sizeof(char *) * (material_list_size + 16));
			free_position_stack = malloc(sizeof(int) * (material_list_size + 16));

			memcpy(material, materials, sizeof(material_t) * material_list_size);
			memcpy(material_name, material_names, sizeof(char *) * material_list_size);

			free(materials);
			free(material_names);

			materials = material;
			material_names = material_name;

			material_list_size += 16;

		}*/
	}

	material = &mat_materials[material_index];
	material_name = &mat_material_names[material_index];
	name = material_AddNameRecord(name);


	/*for(i = 0; i < mat_material_count - 1; i++)
	{
		if(!(mat_materials[i].flags & MATERIAL_INVALID))
		{
			if(!strcmp(mat_material_names[i], name))
			{

				material_name_len = strlen(mat_material_names[i]);
				j = material_name_len;


				while(mat_material_names[i][j] != '.' && j > 0)
				{
					j--;
				}

				if(j)
				{
					if(mat_material_names[i][j + 1] >= '0' && mat_material_names[i][j + 1] <= '9')
					{
						if(mat_material_names[i][j + 2] >= '0' && mat_material_names[i][j + 2] <= '9')
						{
							if(mat_material_names[i][j + 3] >= '0' && mat_material_names[i][j + 3] <= '9')
							{
								if(mat_material_names[i][j + 4] >= '0' && mat_material_names[i][j + 4] <= '9')
								{
									unique_material_name[j] = '\0';
								}
							}
						}
					}
				}

				strcat(unique_material_name, ".");
				strcat(unique_material_name, repetition_value_str);

			}
		}
	}*/

	if(base_color.r > 1.0) base_color.r = 1.0;
	else if(base_color.r < 0.0) base_color.r = 0.0;

	if(base_color.g > 1.0) base_color.g = 1.0;
	else if(base_color.g < 0.0) base_color.g = 0.0;

	if(base_color.b > 1.0) base_color.b = 1.0;
	else if(base_color.b < 0.0) base_color.b = 0.0;

	if(base_color.a > 1.0) base_color.a = 1.0;
	else if(base_color.a < 0.0) base_color.a = 0.0;

	if(roughness < 0.0) roughness = 0.0;
	else if(roughness > 1.0) roughness = 1.0;

	if(metalness < 0.0) metalness = 0.0;
	else if(metalness > 1.0) metalness = 1.0;


	material->r = 0xff * base_color.r;
	material->g = 0xff * base_color.g;
	material->b = 0xff * base_color.b;
	material->a = 0xff * base_color.a;

	material->roughness = 0xff * roughness;
	material->metalness = 0xff * metalness;

	material->ref_count = 0;

	material->diffuse_texture = diffuse_texture;
	material->normal_texture = normal_texture;
	material->height_texture = -1;
	material->metalness_texture = -1;
	material->roughness_texture = -1;

	//material->shader_index = shader_index;
	material->shader_index = r_forward_pass_shader;
	material->draw_group = -1;
	material->flags = 0;
	//material->last_referenced = -1;
	material->frame_ref_count = 0;

	//material->highest_frame_ref_count_view = NULL;

	if(diffuse_texture > -1)
	{
		material->flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
	}

	if(normal_texture > -1)
	{
		material->flags |= MATERIAL_USE_NORMAL_TEXTURE;
	}

	/*if(height_texture > -1)
	{
		material->flags |= MATERIAL_USE_HEIGHT_TEXTURE;
	}

	if(metalness_texture > -1)
	{
		material->flags |= MATERIAL_USE_METALNESS_TEXTURE;
	}

	if(roughness_texture > -1)
	{
		material->flags |= MATERIAL_USE_ROUGHNESS_TEXTURE;
	}*/


	if(base_color.a < 1.0)
	{
		material->flags |= MATERIAL_TRANSLUCENT;
	}


	*material_name = memory_Strdup(name);

	mat_material_count++;
	return material_index;

}


int material_MaterialIndex(char *material_name)
{
	int i;

	for(i = -1; i < mat_material_list_cursor; i++)
	{
		if(mat_materials[i].flags & MATERIAL_INVALID)
			continue;

		if(!strcmp(mat_material_names[i], material_name))
		{
			return i;
		}
	}

	return -1;
}


int material_MaterialIndexRef(char *material_name)
{
	int i;

	for(i = -1; i < mat_material_list_cursor; i++)
	{
		if(mat_materials[i].flags & MATERIAL_INVALID)
			continue;

		if(!strcmp(mat_material_names[i], material_name))
		{
			material_IncRefCount(i);
			return i;
		}
	}

	/* didn't find the material, so return the
	default one, and increment its ref counter... */
	material_IncRefCount(-1);
	return -1;
}


int material_IncRefCount(int material_index)
{
	if(material_index >= -1 && material_index < mat_material_list_cursor)
	{
		if(!(mat_materials[material_index].flags & MATERIAL_INVALID))
		{
			mat_materials[material_index].ref_count++;
			return 1;
		}
	}

	return 0;
}


int material_DecRefCount(int material_index)
{
	if(material_index >= -1 && material_index < mat_material_list_cursor)
	{
		if(!(mat_materials[material_index].flags & MATERIAL_INVALID))
		{
			if(mat_materials[material_index].ref_count)
			{
				mat_materials[material_index].ref_count--;
				return 1;
			}
		}
	}

	return 0;
}

int material_OpRefCount(int material_index, int count)
{
	if(material_index >= -1 && material_index < mat_material_list_cursor)
	{
		if(!(mat_materials[material_index].flags & MATERIAL_INVALID))
		{
			mat_materials[material_index].ref_count += count;
			if(mat_materials[material_index].ref_count < 0) mat_materials[material_index].ref_count = 0;
			return 1;
		}
	}
	return 0;
}


int material_IncCurrentFrameRefCount(int material_index)
{
	if(material_index >= -1 && material_index < mat_material_list_cursor)
	{
		if(!(mat_materials[material_index].flags & MATERIAL_INVALID))
		{

			if(mat_materials[material_index].last_ref_frame != r_frame)
			{
				mat_materials[material_index].last_ref_frame = r_frame;
				mat_materials[material_index].frame_ref_count = 0;
			}

			mat_materials[material_index].frame_ref_count++;
			return 1;
		}
	}

	return 0;
}


int material_IncCurrentFrameRefCountView(int material_index, camera_t *view)
{
	/*if(material_index >= -1 && material_index < mat_material_list_cursor && view)
	{
		if(!(mat_materials[material_index].flags & MATERIAL_INVALID))
		{

			if(mat_materials[material_index].last_ref_frame != r_frame || mat_materials[material_index].highest_frame_ref_count_view != view)
			{
				mat_materials[material_index].last_ref_frame = r_frame;
				mat_materials[material_index].frame_ref_count = 0;
			}

			mat_materials[material_index].frame_ref_count++;

			if(mat_materials[material_index].frame_ref_count > mat_materials[material_index].)

			return 1;
		}
	}

	return 0;*/
}



void material_DestroyMaterialIndex(int material_index)
{
	/* don't allow the default material to be destroyed... */
	if(material_index >= 0 && material_index < mat_material_list_cursor)
	{
		if(!(mat_materials[material_index].flags & MATERIAL_INVALID))
		{
			material_RemoveNameRecord(mat_material_names[material_index]);
			memory_Free(mat_material_names[material_index]);

			material_OpRefCount(-1, mat_materials[material_index].ref_count);

			mat_materials[material_index].ref_count = 0;
			mat_materials[material_index].flags |= MATERIAL_INVALID;

			mat_free_position_stack_top++;
			mat_free_position_stack[mat_free_position_stack_top] = material_index;
			mat_material_count--;
		}
	}
}



int material_SetMaterialName(char *name, int material_index)
{
	int i;

	if(material_index >= 0 && material_index < mat_material_list_cursor)
	{
		if(mat_materials[material_index].flags & MATERIAL_INVALID)
		{
			return 0;
		}
	}

	material_RemoveNameRecord(mat_material_names[material_index]);
	memory_Free(mat_material_names[material_index]);
	mat_material_names[material_index] = memory_Strdup(material_AddNameRecord(name));
	return 1;
}


char *material_GetMaterialName(int material_index)
{
	if(material_index >= -1 && material_index < mat_material_list_cursor)
	{
		if(mat_materials[material_index].flags & MATERIAL_INVALID)
		{
			return NULL;
		}

		return mat_material_names[material_index];
	}
}

material_t *material_GetMaterialPointer(char *material_name)
{

}

material_t *material_GetMaterialPointerIndex(int material_index)
{
	if(material_index >= -1 && material_index < mat_material_list_cursor)
	{
		if(!(mat_materials[material_index].flags & MATERIAL_INVALID))
		{
			return &mat_materials[material_index];
		}
	}

	return NULL;
}


void material_DestroyAllMaterials()
{

	int i;

	for(i = 0; i < mat_material_list_cursor; i++)
	{
		material_DestroyMaterialIndex(i);
	}

	mat_material_count = 0;
	mat_material_list_cursor;
	mat_free_position_stack_top = -1;
}


/*
====================
material_WriteMaterialRecord

although it may seem unnecessary this
function permits writting arbitrary
loaders/savers without having to
replicate code from
material_SerializeMaterials, given
that it will serialize only the
materials that exist inside the
engine...

====================
*/
void material_WriteMaterialRecord(material_t *material, char *material_name, void **buffer)
{
	#if 0
	char *out;
	char *name;
	int name_offset = 0;
	material_record_t *record;

	struct texture_t *texture;

	out = *buffer;

	record = (material_record_t *)out;

	record->base.r = (float)material->r / 255.0;
	record->base.g = (float)material->g / 255.0;
	record->base.b = (float)material->b / 255.0;
	record->base.a = (float)material->a / 255.0;
	record->roughness = (float)material->roughness / 255.0;
	record->metalness = (float)material->metalness / 255.0;

	record->bm_flags = material->flags;

	strcpy(record->names, material_name);
	name_offset += strlen(material_name) + 1;

	if(record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
	{
		texture = texture_GetTexturePointer(material->diffuse_texture);
		//name = texture_GetTextureName(material->diffuse_texture);
		strcpy(record->names + name_offset, texture->texture_info->name);
		name_offset += strlen(name) + 1;
	}

	if(record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
	{
		//name = texture_GetTextureName(material->normal_texture);
		texture =
		//strcpy(record->names + name_offset, name);
		name_offset += strlen(name) + 1;
	}

	if(record->bm_flags & MATERIAL_USE_HEIGHT_TEXTURE)
	{
		name = texture_GetTextureName(material->height_texture);
		strcpy(record->names + name_offset, name);
		name_offset += strlen(name) + 1;
	}

	if(record->bm_flags & MATERIAL_USE_METALNESS_TEXTURE)
	{
		name = texture_GetTextureName(material->metalness_texture);
		strcpy(record->names + name_offset, name);
		name_offset += strlen(name) + 1;
	}

	if(record->bm_flags & MATERIAL_USE_ROUGHNESS_TEXTURE)
	{
		name = texture_GetTextureName(material->roughness_texture);
		strcpy(record->names + name_offset, name);
		name_offset += strlen(name) + 1;
	}

	record->reserved0 = 0;
	record->reserved1 = 0;
	record->reserved2 = 0;
	record->reserved3 = 0;
	record->reserved4 = 0;
	record->reserved5 = 0;
	record->reserved6 = 0;
	record->reserved7 = 0;

	out += sizeof(material_record_t) - (sizeof(material_record_t) - name_offset);

	*buffer = out;

	#endif
}

void material_SerializeMaterials(void **buffer, int *buffer_size)
{
	#if 0
	int i;
	material_section_header_t *header;
	material_record_t *record;
	char *out;
	int size;
	char *name;
	int name_offset = 0;

	size = sizeof(material_section_header_t) + sizeof(material_record_t) * mat_material_count;
	out = memory_Malloc(size);

	*buffer = out;
	*buffer_size = size;

	header = (material_section_header_t *)out;
	out += sizeof(material_section_header_t );

	header->tag[0]  = '[';
	header->tag[1]  = 'm';
	header->tag[2]  = 'a';
	header->tag[3]  = 't';
	header->tag[4]  = 'e';
	header->tag[5]  = 'r';
	header->tag[6]  = 'i';
	header->tag[7]  = 'a';
	header->tag[8]  = 'l';
	header->tag[9]  = '_';
	header->tag[10] = 's';
	header->tag[11] = 'e';
	header->tag[12] = 'c';
	header->tag[13] = 't';
	header->tag[14] = 'i';
	header->tag[15] = 'o';
	header->tag[16] = 'n';
	header->tag[17] = ']';
	header->tag[18] = '\0';
	header->tag[19] = '\0';
	/* make sure we always stay 4 byte aligned... */

	header->material_count = mat_material_count;
	header->reserved0 = 0;
	header->reserved1 = 0;
	header->reserved2 = 0;
	header->reserved3 = 0;
	header->reserved4 = 0;
	header->reserved5 = 0;
	header->reserved6 = 0;
	header->reserved7 = 0;

	for(i = 0; i < mat_material_list_cursor; i++)
	{
		if(mat_materials[i].flags & MATERIAL_INVALID)
			continue;

		material_WriteMaterialRecord(&mat_materials[i], mat_material_names[i], (void **)&out);
	}

	*buffer = out;

	#endif

}


/*
====================
material_ReadMaterialRecord

pretty much the same as
material_WriteMaterialRecord...

====================
*/
void material_ReadMaterialRecord(material_record_t *record, void **buffer)
{

	#if 0
	material_record_t *in_record;
	char *material_name;
	char *texture_name;
	char *in;

	int i = 0;
	int j = 0;

	in = *buffer;
	in_record = (material_record_t *)in;

	record->base = in_record->base;
	record->bm_flags = in_record->bm_flags;
	record->roughness = in_record->roughness;
	record->metalness = in_record->metalness;

	record->reserved0 = 0;
	record->reserved1 = 0;
	record->reserved2 = 0;
	record->reserved3 = 0;
	record->reserved4 = 0;
	record->reserved5 = 0;
	record->reserved6 = 0;
	record->reserved7 = 0;


	for(j = 0; in_record->names[i] != '\0'; i++, j++)
	{
		record->separate_names.material_name[j] = in_record->names[i];
	}
	record->separate_names.material_name[j] = '\0';

	if(in_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
	{
		for(j = 0; in_record->names[i] != '\0'; i++, j++)
		{
			record->separate_names.diffuse_texture_name[j] = in_record->names[i];
		}
		record->separate_names.diffuse_texture_name[j] = '\0';
	}

	if(in_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
	{
		for(j = 0; in_record->names[i] != '\0'; i++, j++)
		{
			record->separate_names.normal_texture_name[j] = in_record->names[i];
		}
		record->separate_names.normal_texture_name[j] = '\0';
	}

	if(in_record->bm_flags & MATERIAL_USE_HEIGHT_TEXTURE)
	{
		for(j = 0; in_record->names[i] != '\0'; i++, j++)
		{
			record->separate_names.height_texture_name[j] = in_record->names[i];
		}
		record->separate_names.height_texture_name[j] = '\0';
	}

	if(in_record->bm_flags & MATERIAL_USE_METALNESS_TEXTURE)
	{
		for(j = 0; in_record->names[i] != '\0'; i++, j++)
		{
			record->separate_names.metalness_texture_name[j] = in_record->names[i];
		}
		record->separate_names.metalness_texture_name[j] = '\0';
	}

	if(in_record->bm_flags & MATERIAL_USE_ROUGHNESS_TEXTURE)
	{
		for(j = 0; in_record->names[i] != '\0'; i++, j++)
		{
			record->separate_names.roughness_texture_name[j] = in_record->names[i];
		}
		record->separate_names.roughness_texture_name[j] = '\0';
	}

	in += sizeof(material_record_t) - (sizeof(material_record_t) - i);

	*buffer = in;

	#endif
}

void material_DeserializeMaterials(void **buffer)
{

	#if 0
	material_section_header_t *header;
	material_record_t record;
	char *in;
	char *material_name;
	char *name;
	int i;
	unsigned short diffuse_texture;
	unsigned short normal_texture;

	in = *buffer;

	header = (material_section_header_t *)in;
	in += sizeof(material_section_header_t );


	for(i = 0; i < header->material_count; i++)
	{
		material_ReadMaterialRecord(&record, (void **)&in);
		/*record = (material_record_t *)in;
		in += sizeof(material_record_t) - 1;

		diffuse_texture = -1;
		normal_texture = -1;

		material_name = record->names;
		in += strlen(material_name) + 1;*/

		if(record.bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
		{
			//strcpy(name, in);
			//in += strlen(name) + 1;
			diffuse_texture = texture_GetTexture(record.separate_names.diffuse_texture_name);
		}

		if(record.bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
		{
			//strcpy(name, in);
			//in += strlen(name) + 1;
			normal_texture = texture_GetTexture(record.separate_names.normal_texture_name);
		}

		if(record.bm_flags & MATERIAL_USE_HEIGHT_TEXTURE)
		{
			//strcpy(name, in);
			//in += strlen(name) + 1;
			//normal_texture = texture_GetTexture(name);
		}

		if(record.bm_flags & MATERIAL_USE_METALNESS_TEXTURE)
		{
			//strcpy(name, in);
			//in += strlen(name) + 1;
			//normal_texture = texture_GetTexture(name);
		}

		if(record.bm_flags & MATERIAL_USE_ROUGHNESS_TEXTURE)
		{
			//strcpy(name, in);
			//in += strlen(name) + 1;
			//normal_texture = texture_GetTexture(name);
		}

		material_CreateMaterial(record.separate_names.material_name, record.base, record.metalness, record.roughness, r_forward_pass_shader, diffuse_texture, normal_texture);

	}

	*buffer = in;

	#endif
}


#ifdef __cplusplus
}
#endif













