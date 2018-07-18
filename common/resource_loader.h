#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H



enum RESOURCE_TYPE
{
	RESOURCE_TYPE_TEXTURE = 0,
	RESOURCE_TYPE_SOUND,
	RESOURCE_TYPE_MATERIAL,
	RESOURCE_TYPE_MODEL,
	RESOURCE_TYPE_ENTITY,
	RESOURCE_TYPE_MAP,
	RESOURCE_TYPE_LAST,
	RESOURCE_TYPE_NONE = RESOURCE_TYPE_LAST

};

struct resource_handle_t
{
	unsigned type : 4;
	unsigned resource : 28;
};

struct resource_t
{
	int type;
	char *file_name;
	char *path;
};


#ifdef __cplusplus
extern "C"
{
#endif

int resource_Init();

void resource_Finish();

void resource_LoadResource(char *file_name);

#ifdef __cplusplus
}
#endif




#endif
