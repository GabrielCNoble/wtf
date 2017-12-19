#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "material.h"
#include "vector.h"



static int material_list_size;
int material_count;
static int free_position_stack_top;
static int *free_position_stack;
material_t *materials;
char **material_names;


/* from renderer.c */
extern int z_pre_pass_shader;
extern int forward_pass_shader;

int default_material;

void material_Init()
{
	material_list_size = 16;
	material_count = 0;
	free_position_stack_top = -1;
	materials = malloc(sizeof(material_t) * material_list_size);
	material_names = malloc(sizeof(char *) * material_list_size);
	free_position_stack = malloc(sizeof(int) * material_list_size);
	
	default_material = material_CreateMaterial("default_material", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);	
}

void material_Finish()
{
	
	int i;
	for(i = 0; i < material_count; i++)
	{
		free(material_names[i]);
	}
	
	free(materials);
	free(free_position_stack);
	free(material_names);
}

int material_CreateMaterial(char *name, vec4_t base_color, float glossiness, float roughness, short shader_index, short diffuse_texture, short normal_texture)
{
	char **material_name;
	material_t *material;
	int *itemp;
	int material_index;
	
	
	
	if(free_position_stack_top >= 0)
	{
		material_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		material_index = material_count++;
		
		if(material_index > material_list_size)
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
				
		}
	}
	
	
	if(base_color.r > 1.0) base_color.r = 1.0;
	else if(base_color.r < 0.0) base_color.r = 0.0;
	
	if(base_color.g > 1.0) base_color.g = 1.0;
	else if(base_color.g < 0.0) base_color.g = 0.0;
	
	if(base_color.b > 1.0) base_color.b = 1.0;
	else if(base_color.b < 0.0) base_color.b = 0.0;
	
	if(base_color.a > 1.0) base_color.a = 1.0;
	else if(base_color.a < 0.0) base_color.a = 0.0;
	
	
	material = &materials[material_index];
	material_name = &material_names[material_index];
	
	material->r = 0xff * base_color.r;
	material->g = 0xff * base_color.g;
	material->b = 0xff * base_color.b;
	material->a = 0xff * base_color.a;
	
	material->diffuse_texture = diffuse_texture;
	material->normal_texture = normal_texture;
	material->shader_index = shader_index;
	
	*material_name = strdup(name);
	

	
	return material_index;
	
}

int material_LoadMaterial(char *file_name)
{
	FILE *f;
	int i;
	int c;
	int k;
	int name_len;
	unsigned long file_size;
	char *file_str;
	
	int save_material;
	char *material_name;
	
	int val_index;
	char val_str[128];
	
	vec4_t diffuse_color;
	
	if(!(f = fopen(file_name, "r")))
	{
		printf("couldn't open [%s\n]", file_name);
		return -1;
	}
	
	
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);
	
	file_str = calloc(file_size + 1, 1);
	fread(file_str, 1, file_size, f);
	fclose(f);
	
	free(file_str);
	
	i = 0;
	
	save_material = 0;
	diffuse_color.a = 1.0;
	
	while(file_str[i] != '\0')
	{
		if(file_str[i] == 'n' &&
		   file_str[i + 1] == 'e' &&
		   file_str[i + 2] == 'w' &&
		   file_str[i + 3] == 'm' &&
		   file_str[i + 4] == 't' &&
		   file_str[i + 5] == 'l')
		{
			
			if(save_material)
			{
				material_CreateMaterial(material_name, diffuse_color, 1.0, 1.0, forward_pass_shader, -1, -1);
			}
			
			
			i += 6;
			while(file_str[i] == ' ') i++;
			
			val_index = 0;
			while(file_str[i] != ' ' && file_str[i] != '\n' && file_str[i] != '\0')
			{
				val_str[val_index] = file_str[i];
				val_index++;
				i++;
			}
			
			val_str[val_index] = '\0';
			material_name = strdup(val_str);
			save_material = 1;
		}
		
		else if(file_str[i] == 'K')
		{
			i++;
			if(file_str[i] == 'a')
			{
				i++;
				
				for(c = 0; c < 3; c++)
				{
					while(file_str[i] == ' ') i++;
				
					val_index = 0;
					while(file_str[i] != ' ' && file_str[i] != '\n' && file_str[i] != '\0')
					{
						val_str[val_index] = file_str[i];
						val_index++;
						i++;
					}
					
					val_str[val_index] = '\0';
					diffuse_color.floats[c] = atof(val_str);					
				}
				
				
				
				
				
			}
		}
	}
	
	
}

void material_DeleteMaterialByIndex(int material_index)
{
	
}




















