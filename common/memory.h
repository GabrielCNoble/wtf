#ifndef MEMORY_H
#define MEMORY_H

void *memory_Malloc(size_t size, char *caller);

void *memory_Calloc(size_t count, size_t elem_size, char *caller);

void memory_Free(void *memory);

char *memory_Strdup(char *src, char *caller);

void memory_Report();




#endif

