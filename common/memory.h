#ifndef MEMORY_H
#define MEMORY_H


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

void *memory_Malloc(size_t size, char *caller);

void *memory_Calloc(size_t count, size_t elem_size, char *caller);

void memory_Free(void *memory);

char *memory_Strdup(char *src, char *caller);

void memory_Report();

void memory_ReportFromCaller(char *caller);

void memory_CheckCorrupted();

#ifdef __cplusplus
}
#endif


#endif

