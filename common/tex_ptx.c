#include "tex_ptx.h"
#include "c_memory.h"
#include "path.h"

#include <stdlib.h>
#include <string.h>

#include "soil/stb_image_aug.h"



void ptx_write(char *output_name, char **files, int file_count)
{
    unsigned char **frames;
    int *frame_sizes;

    int file_buffer_size = 0;
    int file_size;
    char *file_buffer;
    char *out;

    int frame_width = 0;
    int frame_height = 0;
    int frame_size;

    int width;
    int height;
    int components;

    struct ptx_header_t *header;
    struct ptx_frame_t *frame;

    frames = memory_Calloc(file_count, sizeof(char *));
    //frame_sizes = memory_Calloc(file_count, sizeof(int));

    int frame_count = 0;

    FILE *file;
    int i;

    file_buffer_size += sizeof(struct ptx_header_t);

    for(i = 0; i < file_count; i++)
	{
		//file = fopen(files[i], "rb");

		//if(file)
		//{
		frames[frame_count] = stbi_load(files[i], &width, &height, &components, 4);

        if(frames[frame_count])
		{
            if(!frame_count)
			{
                frame_width = width;
				frame_height = height;
			}
			else
			{
				if(frame_width != width || frame_height != height)
				{
                    printf("ptx_write: frame %d has different resolution (is %d x %d; should be %d x %d\n", width, height, frame_width, frame_height);
					free(frames[frame_count]);
					continue;
				}
			}

			frame_count++;
		}
		//}
	}

	if(frame_count)
	{
		frame_size = frame_width * frame_height * 4;

		file_buffer_size += frame_count * (sizeof(struct ptx_frame_t) + frame_size);

		file_buffer = memory_Calloc(1, file_buffer_size);

		out = file_buffer;

        header = (struct ptx_header_t *)out;
        out += sizeof(struct ptx_header_t);

        header->frame_width = frame_width;
        header->frame_height = frame_height;
        header->frame_count = frame_count;

        for(i = 0; i < frame_count; i++)
		{
			memcpy(out, frames[i], frame_size);
			out += frame_size;
		}


		file = fopen(output_name, "wb");
		fwrite(file_buffer, file_buffer_size, 1, file);
		fclose(file);
	}
}

void ptx_read(char *file_name, struct ptx_data_t *data)
{
	char *file_buffer;
	char *in;
	struct ptx_header_t *header;
	unsigned long int file_size;
	int pixels_size;
	FILE *file;


	file = fopen(file_name, "rb");

	if(file)
	{
		file_size = path_GetFileSize(file);
		file_buffer = memory_Calloc(1, file_size);
		fread(file_buffer, 1, file_size, file);
		fclose(file);


		in = file_buffer;
        header = (struct ptx_header_t *)in;
        in += sizeof(struct ptx_header_t);

        data->header.frame_width = header->frame_width;
        data->header.frame_height = header->frame_height;
        data->header.frame_count = header->frame_count;

		pixels_size = data->header.frame_width * data->header.frame_height * data->header.frame_count * 4;

		data->pixels = memory_Calloc(1, pixels_size);

		memcpy(data->pixels, in, pixels_size);

		memory_Free(file_buffer);
	}
	else
	{
		data->header.frame_count = 0;
		data->header.frame_width = 0;
		data->header.frame_height = 0;
		data->pixels = NULL;
	}
}






