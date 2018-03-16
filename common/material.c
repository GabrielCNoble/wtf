#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "material.h"
#include "vector.h"



static int material_list_size;
int material_count;
static int free_position_stack_top;
static int *free_position_stack;

material_t *material_list_base;
char **material_list_name_base;

material_t *materials;
char **material_names;
 

material_t *default_material;
//char *default_material_name = "default material";

int repetition_number = 0;

/* from renderer.c */
extern int z_pre_pass_shader;
extern int forward_pass_shader;

int material_Init()
{
	material_list_size = MAX_MATERIALS;
	material_count = 0;
	free_position_stack_top = -1;
	material_list_base = malloc(sizeof(material_t) * (material_list_size + 1));
	materials = material_list_base + 1;
	
	material_list_name_base = malloc(sizeof(char **) * (material_list_size + 1));
	material_names = material_list_name_base + 1;
	//materials = malloc(sizeof(material_t) * material_list_size);
	//material_names = malloc(sizeof(char *) * material_list_size);
	free_position_stack = malloc(sizeof(int) * material_list_size);
	
	
	default_material = material_list_base;
	
	material_names[-1] = strdup("_default_material_");
	
	default_material->r = 255;
	default_material->g = 255;
	default_material->b = 255;
	default_material->a = 255;
	
	default_material->roughness = 10;
	default_material->metalness = 0;
	
	default_material->shader_index = forward_pass_shader;
	default_material->diffuse_texture = -1;
	default_material->normal_texture = -1;
	default_material->height_texture = -1;
	default_material->metalness_texture = -1;
	default_material->roughness_texture = -1;
	default_material->ref_count = 0;
	default_material->flags = 0;
	

	
	//default_material = material_CreateMaterial("default_material", vec4(1.0, 1.0, 1.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);	
	//red_default_material = material_CreateMaterial("red default material", vec4(1.0, 0.0, 0.0, 1.0), 1.0, 1.0, forward_pass_shader, -1, -1);
	
	return 1;
}

void material_Finish()
{
	
	int i;
	for(i = -1; i < material_count; i++)
	{
		free(material_names[i]);
	}
	free(material_list_base);
	free(free_position_stack);
}


static char unique_material_name[512];
static char repetition_value_str[512];

int material_CreateMaterial(char *name, vec4_t base_color, float metalness, float roughness, short shader_index, short diffuse_texture, short normal_texture)
{
	char **material_name;
	material_t *material;
	int *itemp;
	int material_index;
	int material_name_len;
	int i;
	int j;
	
	
	if(free_position_stack_top >= 0)
	{
		material_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		material_index = material_count++;
		
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
	
	material = &materials[material_index];
	material_name = &material_names[material_index];
	
	strcpy(unique_material_name, name);
		
	//printf("%s\n", name);
	for(i = 0; i < material_count - 1; i++)
	{
		//if(materials[i].material_index > -1)
		if(!(materials[i].flags & MATERIAL_INVALID))
		{
			if(!strcmp(material_names[i], name))
			{
				
				material_name_len = strlen(material_names[i]);
				j = material_name_len;
				
				sprintf(repetition_value_str, "%04d", repetition_number);
				
				while(material_names[i][j] != '.' && j > 0)
				{
					j--;
				}
				
				if(j)
				{
					if(material_names[i][j + 1] >= '0' && material_names[i][j + 1] <= '9')
					{
						if(material_names[i][j + 2] >= '0' && material_names[i][j + 2] <= '9')
						{
							if(material_names[i][j + 3] >= '0' && material_names[i][j + 3] <= '9')
							{
								if(material_names[i][j + 4] >= '0' && material_names[i][j + 4] <= '9')
								{
									unique_material_name[j] = '\0';
								}
							}
						}
					}
				}				
				
				strcat(unique_material_name, ".");
				strcat(unique_material_name, repetition_value_str);	
				repetition_number++;
				
				//printf("material [%s] already exists!\n", name);
				//return -1;
			}
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
	
	material->shader_index = shader_index;
	material->draw_group = -1;
	material->flags = 0;
	
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
		
	
	*material_name = strdup(unique_material_name);

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


int material_MaterialIndex(char *material_name)
{
	int i;
	
	/*if(!strcmp(material_name, "default material"))
		return -1;*/
	
	for(i = -1; i < material_count; i++)
	{
		//if(materials[i].material_index < 0)
		if(materials[i].flags & MATERIAL_INVALID)
			continue;
			
		if(!strcmp(material_names[i], material_name))
		{
			return i;
		}	
	}
	
	return -1;
}


int material_MaterialIndexRef(char *material_name)
{
	int i;
	
	for(i = -1; i < material_count; i++)
	{
		if(materials[i].flags & MATERIAL_INVALID)
			continue;
			
		if(!strcmp(material_names[i], material_name))
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

 
void material_IncRefCount(int material_index)
{
	if(material_index >= -1 && material_index < material_count)
	{
		if(!(materials[material_index].flags & MATERIAL_INVALID))
		{
			materials[material_index].ref_count++;
		}
	}
}


void material_DecRefCount(int material_index)
{
	if(material_index >= -1 && material_index < material_count)
	{
		//if(materials[material_index].material_index > -1)
		if(!(materials[material_index].flags & MATERIAL_INVALID))
		{
			if(materials[material_index].ref_count)
			{
				materials[material_index].ref_count--;
			}
		}
	}
}


void material_DestroyMaterialIndex(int material_index)
{
	/* don't allow the default material to be destroyed... */
	if(material_index >= 0 && material_index < material_count)
	{
		//if(materials[material_index].material_index > -1)
		if(!(materials[material_index].flags & MATERIAL_INVALID))
		{
			free(material_names[material_index]);
			//materials[material_index].material_index = -1;
			materials[material_index].ref_count = 0;
			materials[material_index].flags |= MATERIAL_INVALID;
			
			free_position_stack_top++;
			
			free_position_stack[free_position_stack_top] = material_index;
		}
	}
}


void material_SetMaterial(int material_index)
{
	
}

int material_SetMaterialName(char *name, int material_index)
{
	int i;
	
	if(material_index >= 0 && material_index < material_count)
	{
		if(materials[material_index].flags & MATERIAL_INVALID)
		{
			return 0;
		}
	}
	
	for(i = 0; i < material_count; i++)
	{
		if(!(materials[i].flags & MATERIAL_INVALID))
		{
			if(!strcmp(material_names[i], name))
			{
				return 0;
			}
		}
	}
	
	free(material_names[material_index]);
	
	material_names[material_index] = strdup(name);
	
	return 1;
	
}

void material_DestroyAllMaterials()
{
	
	int i;
	
	for(i = 0; i < material_count; i++)
	{
		/*if(!(materials[i].flags & MATERIAL_INVALID))
		{
			free(material_names[i]);
			materials[i].ref_count = 0;
		}*/
		material_DestroyMaterialIndex(i);
	}
	
	material_count = 0;
	free_position_stack_top = -1;
	repetition_number = 0;
}

















