#include "texture.h"
#include "tex_ptx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

#include "GL/glew.h"
#include "SOIL.h"
#include "c_memory.h"
#include "r_gl.h"

#include "containers/stack_list.h"

/*#include "IL/ilu.h"
#include "IL/ilut.h"
#include "IL/il.h"*/




static char texture_path[256];


//static int tex_texture_list_size;
//int tex_texture_count;
//texture_t *tex_textures;
//texture_info_t *tex_texture_info;
//char **texture_names;


//static int free_position_stack_top;
//static int *free_position_stack;

struct stack_list_t tex_textures;




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

//texture_t missing_texture;
//texture_info_t missing_texture_info;

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
	struct texture_t *default_texture;
	struct texture_info_t *default_texture_info;

	char *default_texture_data;
	char color[4];

	/*free_position_stack_top = -1;
	tex_texture_list_size = 64;
	tex_texture_count = 0;
	tex_textures = memory_Malloc(sizeof(texture_t) * (tex_texture_list_size + 1));
	tex_texture_info = memory_Malloc(sizeof(texture_info_t ) * tex_texture_list_size);
	free_position_stack = memory_Malloc(sizeof(int) * tex_texture_list_size);*/

	tex_textures = stack_list_create(sizeof(struct texture_t), 512 + 1, NULL);


    //default_texture = (struct texture_t *)tex_textures.elements;

    x = stack_list_add(&tex_textures, NULL);
    default_texture = stack_list_get(&tex_textures, x);

	/* this is not ideal... */
    //(struct texture_t *)tex_textures.elements++;
    //tex_textures.free_stack++;

	//default_texture = tex_textures;


	//tex_textures++;

	default_texture_data = memory_Calloc(DEFAULT_TEXTURE_SIZE * DEFAULT_TEXTURE_SIZE * layer_count, 4);


	default_texture->bm_flags = 0;
	default_texture->frame_count = 1;
	default_texture->gl_handle = renderer_GenGLTexture(GL_TEXTURE_2D_ARRAY, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0);
	default_texture->target = GL_TEXTURE_2D_ARRAY;


	for(layer_index = 0; layer_index < layer_count; layer_index++)
	{

		switch(layer_index)
		{
			case 0:
				color[0] = 0xff;
				color[1] = 0;
				color[2] = 0xff;
				color[3] = 0x7f;
			break;

			case 1:
				color[0] = 0;
				color[1] = 0xff;
				color[2] = 0;
				color[3] = 0x7f;
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
	//for(i = 0; i < tex_texture_count; i++)
	//{
		//free(texture_names[i]);
	//	if(tex_textures[i].gl_handle != 0)
	//	{

	//		memory_Free(tex_texture_info[i].name);
	//		memory_Free(tex_texture_info[i].file_name);
			//free(texture_info[i].full_path);
	//		glDeleteTextures(1, &tex_textures[i].gl_handle);
	//	}

	//}

	//tex_textures--;
	//glDeleteTextures(1, &tex_textures[0].gl_handle);

	stack_list_destroy(&tex_textures);
	//memory_Free(tex_texture_info);
	//memory_Free(tex_textures);
	//memory_Free(free_position_stack);

	//glDeleteTextures(1, &missing_texture.gl_handle);

	//ilShutdown();
}

/*void texture_SetPath(char *path)
{
	strcpy(texture_path, path);
}*/

int texture_CreateEmtpyTexture(char *name)
{
    struct texture_t *texture;
	int texture_index;

	texture_index = stack_list_add(&tex_textures, NULL);

	texture = stack_list_get(&tex_textures, texture_index);

	if(!texture->texture_info)
	{
		texture->texture_info = memory_Calloc(1, sizeof(struct texture_info_t));
	}

	return texture_index - 1;
}

int texture_LoadTexture(char *file_name, char *name, int bm_flags)
{
	//unsigned int il_tex_handle;
	int i;

	unsigned int gl_tex_handle;
	int texture_index;
	struct texture_t *texture;
	struct texture_info_t *info;

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
	int frame_count;
	int generate_mips;
	int mip_max_level;
	int min_filter;
	int mag_filter;
	int target;

	int internal_format;
	int format;



	struct ptx_data_t data;


	texture_index = texture_GetTexture(name);

	if(texture_index != -1)
	{
		/* this texture was already loaded, so
		just return the existing texture... */
		printf("texture_LoadTexture: texture [%s] already exists!\n", name);
		return texture_index;
	}


	if(!strcmp(path_GetFileExtension(file_name), "ptx"))
	{
		ptx_read(file_name, &data);

        if(data.pixels)
		{
			width = data.header.frame_width;
			height = data.header.frame_height;
			frame_count = data.header.frame_count;

			channels = 4;
			mip_max_level = 0;
			tex_data = data.pixels;

			target = GL_TEXTURE_2D_ARRAY;

			min_filter = GL_LINEAR;
			mag_filter = GL_LINEAR;
		}
		else
		{
            printf("texture_LoadTexture: couldn't load file for texture [%s]\n", name);
			return -1;
		}
	}
	else
	{

		full_path = path_GetPathToFile(file_name);

		if(!full_path)
		{
			printf("texture_LoadTexture: couldn't find file for texture [%s]\n", name);
		}

		tex_data = SOIL_load_image(full_path, &width, &height, &channels, SOIL_LOAD_AUTO);

		frame_count = 1;

		mip_max_level = 4;
		min_filter = GL_LINEAR_MIPMAP_LINEAR;
		mag_filter = GL_LINEAR_MIPMAP_LINEAR;
		target = GL_TEXTURE_2D;

		if(!tex_data)
		{
			printf("texture_LoadTexture: couldn't load [%s]\nFailure reason: %s\n", name, SOIL_last_result());
			return -1;
		}

		/* try to load the file by its file name alone first. This enables
		importing texture files from places outside the project directory... */
		//tex_data = SOIL_load_image(file_name, &width, &height, &channels, SOIL_LOAD_AUTO);


		/* didn't find the file outside the current directory, so check the search paths... */
		//if(!tex_data)
		//{
		//	full_path = path_GetPathToFile(file_name);

			/* didn't find a path to the file... */
		//	if(!full_path)
		//	{
		//		printf("couldn't fild file [%s] for texture [%s]!\n", file_name, name);
		//		return -1;
		//	}

		//	tex_data = SOIL_load_image(full_path, &width, &height, &channels, SOIL_LOAD_AUTO);

			/* couldn't open the file... */
		//	if(!tex_data)
		//	{
		//		printf("couldn't load %s!\nFailure reason: %s\n", name, SOIL_last_result());
		//		return -1;
		//	}
		//}
		//else
		//{
			/* file_name containst the full path to the file... */
		//	full_path = file_name;
		//}


		//if(free_position_stack_top >= 0)
		//{
		//	texture_index = free_position_stack[free_position_stack_top--];
		//}
		//else
		//{
		//	texture_index = tex_texture_count++;

		//	if(texture_index >= tex_texture_list_size)
		//	{

		//		memory_Free(free_position_stack);

		//		texture = memory_Malloc(sizeof(texture_t) * (tex_texture_list_size + 16));
		//		tex_texture_info = memory_Malloc(sizeof(texture_info_t) * (tex_texture_list_size + 16));
				//c_temp = malloc(sizeof(char *) * (texture_list_size + 16));
		//		free_position_stack = memory_Malloc(sizeof(int) * (tex_texture_list_size + 16));

		//		memcpy(texture, tex_textures, sizeof(texture_t) * tex_texture_list_size);
		//		memcpy(info, tex_texture_info, sizeof(texture_info_t) * tex_texture_list_size);


		//		memory_Free(tex_textures);
		//		memory_Free(tex_texture_info);

		//		tex_textures = texture;
		//		tex_texture_info = info;
		//	}
		//}



		//texture = &tex_textures[texture_index];
		//info = &tex_texture_info[texture_index];
	}

    texture_index = texture_CreateEmtpyTexture(NULL);
    texture = texture_GetTexturePointer(texture_index);

	gl_tex_handle = renderer_GenGLTexture(target, min_filter, mag_filter, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, mip_max_level);

	texture->gl_handle = gl_tex_handle;
	texture->bm_flags = bm_flags & (~TEXTURE_INVALID);
	texture->frame_count = frame_count;
	texture->target = target;

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


	glBindTexture(texture->target, gl_tex_handle);

	if(texture->target == GL_TEXTURE_2D)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, tex_data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internal_format, width, height, frame_count, 0, format, GL_UNSIGNED_BYTE, tex_data);
	}



	glBindTexture(texture->target, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	info = texture->texture_info;

	info->name = memory_Strdup(name);

	/* keep the full path to the file to enable
	the original file to be copied into the proper
	folder if needed... */
	info->file_name = memory_Strdup(full_path);
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


void texture_DestroyTextureIndex(int texture_index)
{
	struct texture_t *texture;

	texture_index++;

	if(texture_index >= 1 && texture_index < tex_textures.element_count)
	{
		texture = stack_list_get(&tex_textures, texture_index);

		if(texture->gl_handle)
		{
			glDeleteTextures(1, &texture->gl_handle);
			texture->gl_handle = 0;

			stack_list_remove(&tex_textures, texture_index);
		}
	}
}

int texture_GetTexture(char *name)
{
	int i;

	struct texture_t *texture;

	if(name)
	{
		for(i = 0; i < tex_textures.element_count; i++)
		{
			texture = (struct texture_t *)tex_textures.elements + i;

            if(texture->bm_flags & TEXTURE_INVALID)
			{
				continue;
			}

			if(texture->texture_info)
			{
				if(!strcmp(name, texture->texture_info->name))
				{
					return i - 1;
				}
			}
		}
	}

	return -1;
}

//char *texture_GetTextureName(int texture_index)
//{
	/*if(texture_index >= -1 && texture_index < tex_texture_count)
	{
		if(tex_textures[texture_index].bm_flags & TEXTURE_INVALID)
		{
			return NULL;
		}

		return tex_texture_info[texture_index].name;
	}*/
//}

struct texture_t *texture_GetTexturePointer(int texture_index)
{
    struct texture_t *texture;

	texture_index++;

    if(texture_index < 0 || texture_index >= tex_textures.element_count)
	{
		texture_index = 0;
	}

    texture = stack_list_get(&tex_textures, texture_index);

    if(texture->bm_flags & TEXTURE_INVALID)
	{
		return NULL;
	}

	return texture;
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







