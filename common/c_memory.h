#ifndef C_MEMORY_H
#define C_MEMORY_H

#include <io.h>

enum MEMORY_MODE
{
	MEMORY_MODE_NO_OVERHEAD = 0,
	MEMORY_MODE_KEEP_CALLER,
	MEMORY_MODE_KEEP_CALLER_AND_GUARD_BYTES,
};

#ifdef __cplusplus
extern "C"
{
#endif

void memory_Init(int memory_mode);

void memory_Finish();

void *memory_MallocCaller(size_t size, const char *caller);

void *memory_CallocCaller(size_t count, size_t elem_size, const char *caller);

void memory_Free(void *memory);

char *memory_StrdupCaller(char *src, const char *caller);

void memory_Report();

void memory_ReportFromCaller(char *caller);

void memory_CheckCorrupted();

#ifdef __cplusplus
}
#endif


#define memory_Malloc(size) memory_MallocCaller(size, __func__)
#define memory_Calloc(count, elem_size) memory_CallocCaller(count, elem_size, __func__)
#define memory_Strdup(src) memory_StrdupCaller(src, __func__)



#endif

