#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL/glew.h"
#include "SOIL.h"

/*#include "IL/ilu.h"
#include "IL/ilut.h"
#include "IL/il.h"*/

#include "texture.h"


static int texture_list_size;
static int texture_count;
texture_t *textures;
static char **texture_names;

static int free_position_stack_top;
static int *free_position_stack;

void texture_Init()
{
	
	free_position_stack_top = -1;
	texture_list_size = 64;
	texture_count = 0;
	textures = malloc(sizeof(texture_t) * texture_list_size);
	texture_names = malloc(sizeof(char *) * texture_list_size);
	free_position_stack = malloc(sizeof(int) * texture_list_size);
}

void texture_Finish()
{
	int i;
	for(i = 0; i < texture_count; i++)
	{
		free(texture_names[i]);
		glDeleteTextures(1, &textures[i].gl_handle);
	}
	
	free(texture_names);
	free(textures);
	free(free_position_stack);
	
	//ilShutdown();
}

int texture_LoadTexture(char *file_name, char *name)
{
	//unsigned int il_tex_handle;
	unsigned int gl_tex_handle;
	int texture_index;
	texture_t *texture;
	
	void *tex_data;
	char **c_temp;
	char tex_name[128];
	int tex_name_len;
	
	int channels;
	int width;
	int height;
	
	int internal_format;
	int format;
	
	//ilGenImages(1, &il_tex_handle);
	//ilBindImage(il_tex_handle);
	
	strcpy(tex_name, name);
	tex_name_len = strlen(tex_name) + 1;
	
	while(tex_name_len % 4)
	{
		tex_name[tex_name_len] = '\0';
		tex_name_len++;
	}
	
	texture_index = texture_GetTexture(tex_name);
	
	if(texture_index != -1)
	{
		/* this texture was already loaded, so 
		just return the existent texture instead... */
		return texture_index;
	}
	
	//printf("%s\n", file_name);
	
	tex_data = SOIL_load_image(file_name, &width, &height, &channels, SOIL_LOAD_AUTO);
	
	/*if(!ilLoadImage(file_name))*/
	if(!tex_data)
	{
		printf("couldn't load %s!\n", name);
		//ilDeleteImages(1, &il_tex_handle);
		return;
	}
	
	
	
	//channels = ilGetInteger(IL_IMAGE_FORMAT);
	//width = ilGetInteger(IL_IMAGE_WIDTH);
	//height = ilGetInteger(IL_IMAGE_HEIGHT);
	//tex_data = ilGetData();
	
	//while(glGetError() != GL_NO_ERROR);
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &gl_tex_handle);
	glBindTexture(GL_TEXTURE_2D, gl_tex_handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
	
	
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
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, tex_data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	//printf("%s %d %x\n", name, channels, glGetError());
	
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
			c_temp = malloc(sizeof(char *) * (texture_list_size + 16));
			free_position_stack = malloc(sizeof(int) * (texture_list_size + 16));
			
			memcpy(texture, textures, sizeof(texture_t) * texture_list_size);
			memcpy(c_temp, texture_names, sizeof(char *) * texture_list_size);
			
			free(textures);
			free(texture_names);
			
			textures = texture;
			texture_names = c_temp;
		}
	}
	
	
	
	texture = &textures[texture_index];
	texture_names[texture_index] = strdup(tex_name);
	
	texture->gl_handle = gl_tex_handle;
	texture->tex_type = GL_TEXTURE_2D;
	free(tex_data);
	
	return texture_index;
	
	//ilDeleteImages(1, &il_tex_handle);	
}


int texture_LoadCubeTexture(char *files, char *name)
{
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
	unsigned char *pixel_data[6];
	
	i = 0;
	c = 0;
	while(files[i] != '\0')
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
	}
	
	
	if(c < 6)
	{
		printf("not enough files to form a cube texture!\n");
		return -1;
	}
	

	
	//printf("%s\n", file_name);
	
	//tex_data = SOIL_load_image(file_name, &width, &height, &channels, SOIL_LOAD_AUTO);
	
	
	for(i = 0; i < 6; i++)
	{
		pixel_data[i] = SOIL_load_image(tex_names[i], &width, &height, &channels, SOIL_LOAD_AUTO);
		
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
	
	
	
	texture = &textures[texture_index];
	texture_names[texture_index] = strdup(name);
	
	texture->gl_handle = gl_tex_handle;
	texture->tex_type = GL_TEXTURE_CUBE_MAP;
	
	for(i = 0; i < 6; i++)
	{
		free(pixel_data[i]);
	}
	//free(tex_data);
	
	return texture_index;
	
	
}


void texture_DeleteTextureByIndex(int texture_index)
{
	if(texture_index >= 0 && texture_index < texture_count)
	{
		if(textures[texture_index].texture_index != -1)
		{
			textures[texture_index].texture_index = -1;
			glDeleteTextures(1, &textures[texture_index].gl_handle);
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
		if(!strcmp(name, texture_names[i]))
		{
			return i;
		}
	}
	
	return -1;
}



void texture_BindTexture(int texture_index, int tex_unit)
{
	texture_t *tex = &textures[texture_index];	
	glActiveTexture(tex_unit);
	glBindTexture(tex->tex_type, tex->gl_handle);
}





