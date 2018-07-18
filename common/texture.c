#include "texture.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

#include "GL/glew.h"
#include "SOIL.h"
#include "memory.h"

/*#include "IL/ilu.h"
#include "IL/ilut.h"
#include "IL/il.h"*/




static char texture_path[256];

 
static int tex_texture_list_size;
int tex_texture_count;
texture_t *tex_textures;
texture_info_t *tex_texture_info;
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

#define DEFAULT_TEXTURE_SIZE 16

texture_t missing_texture;
texture_info_t missing_texture_info;

#ifdef __cplusplus
extern "C"
{
#endif

int texture_Init()
{
	int x;
	int y;
	int layer_count = 2;
	int layer_index;
	int current_layer_offset;
	int current_pixel_offset;
	texture_t *default_texture;
	
	char *default_texture_data;
	char color[4];
	
	free_position_stack_top = -1;
	tex_texture_list_size = 64;
	tex_texture_count = 0;
	tex_textures = memory_Malloc(sizeof(texture_t) * (tex_texture_list_size + 1), "texture_Init");
	tex_texture_info = memory_Malloc(sizeof(texture_info_t ) * tex_texture_list_size, "texture_Init");
	free_position_stack = memory_Malloc(sizeof(int) * tex_texture_list_size, "texture_Init");
	
	
	default_texture = tex_textures;
	tex_textures++;
	
	
	default_texture->bm_flags = 0;
	default_texture->frame_count = 1;
	default_texture->gl_handle = texture_GenGLTexture(GL_TEXTURE_2D_ARRAY, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0);
	default_texture->target = GL_TEXTURE_2D_ARRAY;
	
	default_texture_data = memory_Malloc(DEFAULT_TEXTURE_SIZE * DEFAULT_TEXTURE_SIZE * 4 * layer_count, "texture_Init");
	
	for(layer_index = 0; layer_index < layer_count; layer_index++)
	{
		
		switch(layer_index)
		{
			case 0:
				color[0] = 0xff;
				color[1] = 0;
				color[2] = 0xff;
				color[3] = 0xff;
			break;
			
			case 1:
				color[0] = 0;
				color[1] = 0xff;
				color[2] = 0;
				color[3] = 0xff;
			break;
		}
		
		current_layer_offset = layer_index * DEFAULT_TEXTURE_SIZE * DEFAULT_TEXTURE_SIZE * 4;
		
		for(y = 0; y < DEFAULT_TEXTURE_SIZE; y++)
		{
			for(x = 0; x < DEFAULT_TEXTURE_SIZE; x++)
			{	
				current_pixel_offset = y * 4 * DEFAULT_TEXTURE_SIZE + x * 4 + current_layer_offset;
				
				if((x + y) % 2)
				{
					default_texture_data[current_pixel_offset] = color[0];
					default_texture_data[current_pixel_offset + 1] = color[1];
					default_texture_data[current_pixel_offset + 2] = color[2];
					default_texture_data[current_pixel_offset + 3] = color[3];
				}
				else
				{
					default_texture_data[current_pixel_offset] = 0;
					default_texture_data[current_pixel_offset + 1] = 0;
					default_texture_data[current_pixel_offset + 2] = 0;
					default_texture_data[current_pixel_offset + 3] = 0;
				}		
			}
		}
	}
		
	
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, default_texture->gl_handle);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, default_texture_data);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, DEFAULT_TEXTURE_SIZE, DEFAULT_TEXTURE_SIZE, layer_count, 0, GL_RGBA, GL_UNSIGNED_BYTE, default_texture_data);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	
	memory_Free(default_texture_data);
		
	return 1;
}

void texture_Finish()
{
	int i; 
	for(i = 0; i < tex_texture_count; i++)
	{
		//free(texture_names[i]);
		if(tex_textures[i].gl_handle != 0)
		{
			
			memory_Free(tex_texture_info[i].name);
			memory_Free(tex_texture_info[i].file_name);
			//free(texture_info[i].full_path);
			glDeleteTextures(1, &tex_textures[i].gl_handle);
		}
		
	}
	
	tex_textures--;
	glDeleteTextures(1, &tex_textures[0].gl_handle);
	
	memory_Free(tex_texture_info);
	memory_Free(tex_textures);
	memory_Free(free_position_stack);
	
	//glDeleteTextures(1, &missing_texture.gl_handle);
	
	//ilShutdown();
}

/*void texture_SetPath(char *path)
{
	strcpy(texture_path, path);
}*/

unsigned int texture_GenGLTexture(int target, int min_filter, int mag_filter, int wrap_s, int wrap_t, int wrap_r, int base_level, int max_level)
{
	unsigned int handle;
	int temp;
	int i;
	int wrap_mode_test_count;
	int wrap_mode;
	
	int filter_mode;
	
	switch(target)
	{
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
			wrap_mode_test_count = 1;
		break;	
			
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
			wrap_mode_test_count = 2;
		break;
			
		case GL_TEXTURE_3D:
			wrap_mode_test_count = 3;
			
		break;	
		
		default:
			printf("texture_GetEmptyGLTexture: invalid target!\n");
			return 0;
		break;
	}
	
	for(i = 0; i < 2; i++)
	{
		switch(i)
		{
			case 0:
				filter_mode = min_filter;
			break;
			
			case 1:
				filter_mode = mag_filter;
			break;
		}
		
		
		switch(filter_mode )
		{
			case GL_NEAREST:
			case GL_NEAREST_MIPMAP_NEAREST:
			case GL_NEAREST_MIPMAP_LINEAR:
			
			case GL_LINEAR:
			case GL_LINEAR_MIPMAP_NEAREST:
			case GL_LINEAR_MIPMAP_LINEAR:
				
			break;
			
			default:
				printf("texture_GenEmptyGLTexture: invalid filtering mode!\n");
				return 0;	
				
		}
		
		
	}
	
	
	for(i = 0; i < wrap_mode_test_count; i++)
	{
		switch(i)
		{
			case 0:
				wrap_mode = wrap_s;
			break;
			
			case 1:
				wrap_mode = wrap_t;
			break;
			
			case 2:
				wrap_mode = wrap_r;
			break;
		}
		
		switch(wrap_mode)
		{
			/*case GL_NEAREST:
			case GL_NEAREST_MIPMAP_NEAREST:
			case GL_NEAREST_MIPMAP_LINEAR:
			
			case GL_LINEAR:
			case GL_LINEAR_MIPMAP_NEAREST:
			case GL_LINEAR_MIPMAP_LINEAR:*/
			
			case GL_CLAMP:
			case GL_CLAMP_TO_BORDER:
			case GL_REPEAT:
			
			break;
			
			default:
				printf("texture_GenEmptyGLTexture: invalid wrap mode!\n");
				return 0;	
				
		}
		
	}
	
	
	if(base_level < 0)
	{
		printf("texture_GenEmptyGLTexture: negative base level. Clamping to zero...\n");
		base_level = 0;
	}
	
	if(max_level < 0)
	{
		printf("texture_GenEmptyGLTexture: negative max level. Clamping to zero...\n");
		max_level = 0;
	}
	
	if(base_level > max_level)
	{
		printf("texture_GenEmptyGLTexture: base level greater than max level...\n");
		temp = base_level;
		base_level = max_level;
		max_level = temp;
	}
	
	
	glGenTextures(1, &handle);
	glBindTexture(target, handle);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap_r);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, base_level);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, max_level);
	
	if(max_level)
	{
		glGenerateMipmap(target);
	}
	
	glBindTexture(target, 0);
	
	
	return handle;
}

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
	
		/* didn't find a path to the file... */
		if(!full_path)
		{
			printf("couldn't fild file [%s] for texture [%s]!\n", file_name, name);
			return -1;
		}
		
		tex_data = SOIL_load_image(full_path, &width, &height, &channels, SOIL_LOAD_AUTO);
		
		/* couldn't open the file... */
		if(!tex_data)
		{
			printf("couldn't load %s!\nFailure reason: %s\n", name, SOIL_last_result());
			return -1;
		}
	}
	else
	{
		/* file_name containst the full path to the file... */
		full_path = file_name;
	}
	
	
	if(free_position_stack_top >= 0)
	{
		texture_index = free_position_stack[free_position_stack_top--];
	}
	else
	{
		texture_index = tex_texture_count++;
		
		if(texture_index >= tex_texture_list_size)
		{
			
			memory_Free(free_position_stack);
			
			texture = memory_Malloc(sizeof(texture_t) * (tex_texture_list_size + 16), "texture_LoadTexture");
			tex_texture_info = memory_Malloc(sizeof(texture_info_t) * (tex_texture_list_size + 16), "texture_LoadTexture");
			//c_temp = malloc(sizeof(char *) * (texture_list_size + 16));
			free_position_stack = memory_Malloc(sizeof(int) * (tex_texture_list_size + 16), "texture_LoadTexture");
			
			memcpy(texture, tex_textures, sizeof(texture_t) * tex_texture_list_size);
			memcpy(info, tex_texture_info, sizeof(texture_info_t) * tex_texture_list_size);
		
			
			memory_Free(tex_textures);
			memory_Free(tex_texture_info);
			
			tex_textures = texture;
			tex_texture_info = info;
		}
	}
	
	
	
	texture = &tex_textures[texture_index];
	info = &tex_texture_info[texture_index];
	
	
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
	
	/*glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &gl_tex_handle);
	glBindTexture(GL_TEXTURE_2D, gl_tex_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);*/
	
	gl_tex_handle = texture_GenGLTexture(GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 4);
	
	texture->gl_handle = gl_tex_handle;
	texture->bm_flags = bm_flags & (~TEXTURE_INVALID);
	texture->frame_count = 1;
	texture->target = GL_TEXTURE_2D;
	
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
	glBindTexture(GL_TEXTURE_2D, gl_tex_handle);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
	
	info->name = memory_Strdup(name, "texture_LoadTexture");
	
	/* keep the full path to the file to enable
	the original file to be copied into the proper
	folder if needed... */
	info->file_name = memory_Strdup(full_path, "texture_LoadTexture");
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
	//info->target = GL_TEXTURE_2D;
	info->ref_count = 0;

	
	/* this got returned by soil, which uses malloc and free... */
	free(tex_data);
	return texture_index;

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
	if(texture_index >= 0 && texture_index < tex_texture_count)
	{
		if(tex_textures[texture_index].gl_handle)
		{
			glDeleteTextures(1, &tex_textures[texture_index].gl_handle);
			tex_textures[texture_index].gl_handle = 0;
			free_position_stack_top++;
			free_position_stack[free_position_stack_top] = texture_index;
		}
	}
}

int texture_GetTexture(char *name)
{
	int i;
	
	if(name)
	{
		for(i = -1; i < tex_texture_count; i++)
		{
			if(!strcmp(name, tex_texture_info[i].name))
			{
				return i;
			}
		}
	}
	
	return -1;
}

char *texture_GetTextureName(int texture_index)
{
	if(texture_index >= -1 && texture_index < tex_texture_count)
	{
		if(tex_textures[texture_index].bm_flags & TEXTURE_INVALID)
		{
			return NULL;
		}
		
		return tex_texture_info[texture_index].name;
	}
} 

void texture_UploadTexture(int texture_index)
{
	
}

void texture_TexParameteri(int texture_index, int param, int value)
{
	int target;
	unsigned int handle;
	if(texture_index >= 0 && texture_index < tex_texture_count)
	{
		if(tex_textures[texture_index].gl_handle)
		{
			//target = tex_texture_info[texture_index].target;
			target = tex_textures[texture_index].target;
			handle = tex_textures[texture_index].gl_handle;
			
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




void texture_SerializeTextures(void **buffer, int *buffer_size)
{
	
}

void texture_DeserializeTextures(void **buffer)
{
	
}

#ifdef __cplusplus
}
#endif







