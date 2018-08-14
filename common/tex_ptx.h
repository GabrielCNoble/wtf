#ifndef TEX_PTR_H
#define TEX_PTX_H



struct ptx_header_t
{
	unsigned short frame_count;
	unsigned short frame_width;
	unsigned short frame_height;
	unsigned short align;
};

struct ptx_frame_t
{
    unsigned int offset;
    unsigned int size;
};

struct ptx_data_t
{
    struct ptx_header_t header;
    unsigned char *pixels;
};

void ptx_write(char *output_name, char **files, int file_count);

void ptx_read(char *file_name, struct ptx_data_t *data);

#endif // TEX_PTR_H
