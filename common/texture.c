#include "texture.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

#include "GL/glew.h"
#include "SOIL.h"

/*#include "IL/ilu.h"
#include "IL/ilut.h"
#include "IL/il.h"*/




static char texture_path[256];

 
static int texture_list_size;
int texture_count;
texture_t *textures;
texture_info_t *texture_info;
//char **texture_names;


static int free_position_stack_top;
static int *free_position_stack;

static char *suffixes[] = 
{
	".cubemap_pos_x",
	".cubemap_neg_x",
	".cubemap_pos_y",
	".cubemap_neg_y",
	".cubemap_pos_z",
	".cubemap_neg_z"
};

#define MISSING_TEXTURE_SIZE 64

texture_t missing_texture;
texture_info_t missing_texture_info;

int texture_Init()
{
	int x;
	int y;
	free_position_stack_top = -1;
	texture_list_size = 64;
	texture_count = 0;
	textures = malloc(sizeof(texture_t) * texture_list_size);
	texture_info = malloc(sizeof(texture_info_t ) * texture_list_size);
	free_position_stack = malloc(sizeof(int) * texture_list_size);
	
	char *missing_texture_bytes = malloc(MISSING_TEXTURE_SIZE * MISSING_TEXTURE_SIZE * 4);
	
	
	for(y = 0; y < MISSING_TEXTURE_SIZE; y++)
	{
		for(x = 0; x < MISSING_TEXTURE_SIZE; x++)
		{
			if((x + y) % 2)
			{
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4] = 0xff;
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4 + 1] = 0;
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4 + 2] = 0xff;
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4 + 3] = 0;
			}
			else
			{
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4] = 0;
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4 + 1] = 0;
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4 + 2] = 0;
				missing_texture_bytes[y * 4 * MISSING_TEXTURE_SIZE + x * 4 + 3] = 0;
			}
				
		}
	}
	
	
	
	missing_texture_info.name = strdup("missing texture");
	missing_texture_info.file_name = NULL;
	missing_texture_info.width = MISSING_TEXTURE_SIZE;
	missing_texture_info.height = MISSING_TEXTURE_SIZE;
	missing_texture_info.internal_format = GL_RGBA8;
	missing_texture_info.format = GL_RGBA;
	
	missing_texture.bm_flags = 0;
	
	glGenTextures(1, &missing_texture.gl_handle);
	glBindTexture(GL_TEXTURE_2D, missing_texture.gl_handle);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, MISSING_TEXTURE_SIZE, MISSING_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, missing_texture_bytes);
	
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	free(missing_texture_bytes);
	
	
	return 1;
}

void texture_Finish()
{
	int i;
	for(i = 0; i < texture_count; i++)
	{
		//free(texture_names[i]);
		if(textures[i].gl_handle != 0)
		{
			free(texture_info[i].name);
			free(texture_info[i].file_name);
			free(texture_info[i].full_path);
			glDeleteTextures(1, &textures[i].gl_handle);
		}
		
	}
	
	free(texture_info);
	free(textures);
	free(free_position_stack);
	
	glDeleteTextures(1, &missing_texture.gl_handle);
	
	//ilShutdown();
}

/*void texture_SetPath(char *path)
{
	strcpy(texture_path, path);
}*/

int texture_LoadTexture(char *file_name, char *name, int bm_flags)
{
	//unsigned int il_tex_handle;
	int i;
	
	unsigned int gl_tex_handle;
	int texture_index;
	texture_t *texture;
	texture_info_t *info;
	
	void *tex_data;
	char **c_temp;
	char *full_path;
	char *tex_path;
	char *tex_file_name;
	int tex_file_name_start;
	//int tex_path_start;
	int tex_path_len;
	int full_path_len;
	
	int channels;
	int width;
	int height;
	
	int internal_format;
	int format;
	
	
	//ilGenImages(1, &il_tex_handle);
	//ilBindImage(il_tex_handle);
	
	//strcpy(tex_name, name);
	//tex_name_len = strlen(tex_name) + 1;
	
	/*while(tex_name_len % 4)
	{
		tex_name[tex_name_len] = '\0';
		tex_name_len++;
	}*/
	
	texture_index = texture_GetTexture(name);
	
	if(texture_index != -1)
	{
		/* this texture was already loaded, so 
		just return the existing texture... */
		printf("texture_LoadTexture: texture [%s] already exists!\n", name);
		return texture_index;
	}
	
	
	
	
	/* try to load the file by its file name alone first. This enables
	importing texture files from places outside the project directory... */
	tex_data = SOIL_load_image(file_name, &width, &height, &channels, SOIL_LOAD_AUTO);
	
	
	/* didn't find the file outside the current directory, so check the search paths... */
	if(!tex_data)
	{
		full_path = path_GetPathToFile(file_name);
	
		if(!full_path)
		{
			printf("couldn't fild file [%s] for texture [%s]!\n", file_name, name);
			return;
		}
		
		tex_data = SOIL_load_image(full_path, &width, &height, &channels, SOIL_LOAD_AUTO);
		
		if(!tex_data)
		{
			printf("couldn't load %s!\nFailure reason: %s\n", name, SOIL_last_result());
			return;
		}
	}
	else
	{
		full_path = file_name;
	}
	
	
	full_path_len = strlen(full_path);
	
	/* strip away the file name, keeping only
	the path to the file... */	
	for(tex_file_name_start = full_path_len; tex_file_name_start > 0; tex_file_name_start--)
	{
		if(full_path[tex_file_name_start] == '\\' || full_path[tex_file_name_start] == '/')
		{
			tex_file_name_start ++;
			break;
		}
	}
	
	
	
	
	if(free_position_stack_top >= 0)
	{
		texture_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		texture_index = texture_count++;
		
		if(texture_index >= texture_list_size)
		{
			
			free(free_position_stack);
			
			texture = malloc(sizeof(texture_t) * (texture_list_size + 16));
			texture_info = malloc(sizeof(texture_info_t) * (texture_list_size + 16));
			//c_temp = malloc(sizeof(char *) * (texture_list_size + 16));
			free_position_stack = malloc(sizeof(int) * (texture_list_size + 16));
			
			memcpy(texture, textures, sizeof(texture_t) * texture_list_size);
			memcpy(info, texture_info, sizeof(texture_info_t) * texture_list_size);
		
			
			free(textures);
			free(texture_info);
			
			textures = texture;
			texture_info = info;
		}
	}
	
	
	
	texture = &textures[texture_index];
	info = &texture_info[texture_index];
	
	
	/*texture->gl_handle = gl_tex_handle;
	texture->bm_flags = bm_flags & (~TEXTURE_INVALID);
	
	info->name = strdup(name);
	info->file_name = strdup(full_path + tex_file_name_start);
	info->full_path = strdup(full_path);
	info->format = format;
	info->internal_format = internal_format;
	info->width = width;
	info->height = height;
	info->ref_count = 0;*/
	
	
	
	/* the texture is within the same directory as the project file (and it shouldn't...) */
	//if(!tex_file_name_start)
	//{
		//tex_path = malloc(full_path_len + 1);
		//tex_file_name = malloc(full_path_len + 1);
		//tex_path_len = full_path_len;
	//	tex_file_name_start = 0;
	//}
	//else
	//{
		//tex_path = malloc(tex_path_len + 1);
		//tex_file_name = malloc(full_path_len - tex_path_len + 1);
		//tex_file_name_start = tex_path_len + 1;
	//}
	
	//memcpy(tex_path, full_path, tex_path_len);
	//tex_path[tex_path_len] = '\0';	
	
	//strcpy(tex_file_name, full_path + tex_file_name_start);
	//memcpy(tex_file_name, full_path + )
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &gl_tex_handle);
	glBindTexture(GL_TEXTURE_2D, gl_tex_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
	
	texture->gl_handle = gl_tex_handle;
	texture->bm_flags = bm_flags & (~TEXTURE_INVALID);
	
	//printf("%d\n", channels);
	
	switch(channels)
	{
		case 1:
			internal_format = GL_LUMINANCE8;
			format = GL_LUMINANCE;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		break;
		
		case 3:
			internal_format = GL_RGB8;
			format = GL_RGB;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		break;
		
		case 4:
			internal_format = GL_RGBA8;
			format = GL_RGBA;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		break;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
	
	info->name = strdup(name);
	info->file_name = strdup(full_path + tex_file_name_start);
	info->full_path = strdup(full_path);
	info->format = format;
	info->internal_format = internal_format;
	info->width = width;
	info->height = height;
	info->base_level = 0;
	info->max_level = 4;
	info->min_filter = GL_LINEAR_MIPMAP_LINEAR;
	info->mag_filter = GL_LINEAR_MIPMAP_LINEAR;
	info->wrap_s = GL_REPEAT;
	info->wrap_t = GL_REPEAT;
	info->target = GL_TEXTURE_2D;
	info->ref_count = 0;
	
	//printf("%s %d %x\n", name, channels, glGetError());
	
	
	//info->bm_flags = bm_flags;
	
	//printf("texture_LoadTexture: texture [%s] with name [%s] loaded!\n", info->file_name, info->name);
	
	free(tex_data);
	
	return texture_index;
	
	//ilDeleteImages(1, &il_tex_handle);	
}


int texture_LoadCubeTexture(char *file_name, char *name)
{
	#if 0
	int i;
	int c;
	int k;
	int width;
	int height;
	int channels;
	int format;
	int internal_format;
	int texture_index;
	texture_t *texture;
	unsigned int gl_tex_handle;
	char tex_names[6][64];
	char tex_name[64];
	char ext[6];
	//static char suffixes
	char full_path[256];
	unsigned char *pixel_data[6];
	
	i = 0;
	c = 0;
	
	/*for(i = 0; i < 6; i++)
	{
		strcpy(tex_names[i], file_name);
		strcat
	}*/
	
	while(file_name[i] != '.')
	{
		tex_name[i] = file_name[i];
		
		i++;
	}
	
	tex_name[i] = '\0';
	
	while(file_name[i])
	{
		ext[c++] = file_name[i++];	
	}
	
	ext[c] = '\0';
	
	for(i = 0; i < 6; i++)
	{
		//strcpy(full_path, texture_path);
		//strcat(full_path, "/");
		strcpy(tex_names[i], tex_name);
		strcat(tex_names[i], suffixes[i]);
		strcat(tex_names[i], ext);
		
		printf("%s\n", tex_names[i]);
	}
	
	
	
	/*while(files[i] != '\0')
	{
		while(files[i] == ' ') i++;
		
		k = 0;
		while(files[i] != ';' && files[i] != ' ')
		{
			tex_names[c][k] = files[i];
			
			k++;
			i++;
		}
	
		while(files[i] != ';') i++;	
		i++;
		tex_names[c][k] = '\0';
		//printf("%s\n", tex_names[c]);
		c++;
	}*/
	//return;
	
	/*if(c < 6)
	{
		printf("not enough files to form a cube texture!\n");
		return -1;
	}*/
	

	
	//printf("%s\n", file_name);
	
	//tex_data = SOIL_load_image(file_name, &width, &height, &channels, SOIL_LOAD_AUTO);
	
	
	for(i = 0; i < 6; i++)
	{
		
		strcpy(full_path, texture_path);
		strcat(full_path, "/");
		strcat(full_path, tex_names[i]);
		
		//pixel_data[i] = SOIL_load_image(tex_names[i], &width, &height, &channels, SOIL_LOAD_AUTO);
		pixel_data[i] = SOIL_load_image(full_path, &width, &height, &channels, SOIL_LOAD_AUTO);
		
		if(!pixel_data[i])
		{
			printf("couldn't load %s!\n", tex_names[i]);
			for(i--; i >= 0; i--)
			{
				free(pixel_data[i]);
			}
			return -1;
		}

	}
	
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &gl_tex_handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, gl_tex_handle);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
	
	
	switch(channels)
	{
		case 1:
			internal_format = GL_LUMINANCE8;
			format = GL_LUMINANCE;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		break;
		
		case 3:
			internal_format = GL_RGB;
			format = GL_RGB;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		break;
		
		case 4:
			internal_format = GL_RGBA8;
			format = GL_RGBA;
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		break;
	}
	
	for(i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, width, 0, format, GL_UNSIGNED_BYTE, pixel_data[i]); 	
		//glTexImage3D(GL_TEXTURE_CUBE_MAP, )
	}
	
	/*glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);*/
	
	
	if(free_position_stack_top >= 0)
	{
		texture_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		texture_index = texture_count++;
		
		/*if(texture_index >= texture_list_size)
		{
			
			free(free_position_stack);
			
			texture = malloc(sizeof(texture_t) * (texture_list_size + 16));
			c_temp = malloc(sizeof(char *) * (texture_list_size + 16));
			free_position_stack = malloc(sizeof(int) * (texture_list_size + 16));
			
			memcpy(texture, textures, sizeof(texture_t) * texture_list_size);
			memcpy(c_temp, texture_names, sizeof(char *) * texture_list_size);
			
			free(textures);
			free(texture_names);
			
			textures = texture;
			texture_names = c_temp;
		}*/
	}
	
	
	/*
	texture = &textures[texture_index];
	texture_names[texture_index].name = strdup(name);
	texture_names[texture_index].file_name = strdup(file_name);*/
	
	texture->gl_handle = gl_tex_handle;
	texture->tex_type = GL_TEXTURE_CUBE_MAP;
	
	for(i = 0; i < 6; i++)
	{
		free(pixel_data[i]);
	}
	//free(tex_data);
	
	return texture_index;
	
	#endif
	
	
}


void texture_DeleteTextureByIndex(int texture_index)
{
	if(texture_index >= 0 && texture_index < texture_count)
	{
		if(textures[texture_index].gl_handle)
		{
			glDeleteTextures(1, &textures[texture_index].gl_handle);
			textures[texture_index].gl_handle = 0;
			free_position_stack_top++;
			free_position_stack[free_position_stack_top] = texture_index;
		}
	}
}

int texture_GetTexture(char *name)
{
	int i;
	
	for(i = 0; i < texture_count; i++)
	{
		if(!strcmp(name, texture_info[i].name))
		{
			return i;
		}
	}
	
	return -1;
}

void texture_UploadTexture(int texture_index)
{
	
}

void texture_TexParameteri(int texture_index, int param, int value)
{
	int target;
	unsigned int handle;
	if(texture_index >= 0 && texture_index < texture_count)
	{
		if(textures[texture_index].gl_handle)
		{
			target = texture_info[texture_index].target;
			handle = textures[texture_index].gl_handle;
			
			switch(param)
			{
				case GL_TEXTURE_MIN_FILTER:
				case GL_TEXTURE_MAG_FILTER:
				case GL_TEXTURE_BASE_LEVEL:
				case GL_TEXTURE_MAX_LEVEL:
				case GL_TEXTURE_WRAP_S:
				case GL_TEXTURE_WRAP_T:
					glBindTexture(target, handle);
					glTexParameteri(target, param, value);
					
					if(param == GL_TEXTURE_BASE_LEVEL || param == GL_TEXTURE_MAX_LEVEL)
					{
						glGenerateMipmap(target);
					}
				break;
				
				default:
					return;
				
			}
			
			
			
		}
	}	
}













