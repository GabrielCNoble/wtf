#ifndef TEX_COMMON_H
#define TEX_COMMON_H




typedef struct
{
	unsigned int gl_handle;
	unsigned int bm_flags;

	unsigned short frame_count;
	unsigned short target;

}texture_t;

typedef struct
{
	char *name;
	char *file_name;
	char *full_path;
	short width;
	short height;
	short internal_format;
	short format;
	unsigned short wrap_s;
	unsigned short wrap_t;
	unsigned short min_filter;
	unsigned short mag_filter;
	unsigned char base_level;
	unsigned char max_level;
	unsigned short align;
	int ref_count;
}texture_info_t;




enum TEXTURE_FLAGS
{
	TEXTURE_INVALID = 1,
	TEXTURE_INVERT_X = 1 << 1,
	TEXTURE_INVERT_Y = 1 << 2,
};

enum TEXTURE_WRAP_MODE
{
	TEXTURE_REPEAT = 0,
	TEXTURE_CLAMP = 1,
};

enum TEXTURE_SCALE_FILTER
{
	TEXTURE_NEAREST = 0,
	TEXTURE_LINEAR,
	TEXTURE_NEAREST_MIPMAP_NEAREST,
	TEXTURE_LINEAR_MIPMAP_NEAREST,
	TEXTURE_NEAREST_MIPMAP_LINEAR,
	TEXTURE_LINEAR_MIPMAP_LINEAR,
};


#endif // TEX_COMMON_H
