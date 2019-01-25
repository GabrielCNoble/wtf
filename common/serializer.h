#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdint.h>



struct serializer_entry_data_t
{
    char label[32];
    uint64_t offset;
    uint64_t size;
};




struct serializer_entry_t
{
    struct serializer_entry_t *next;
    struct serializer_entry_data_t data;
    void *entry_buffer;
};




static char serializer_header_tag[] = "[serializer header]";

struct serializer_header_t
{
    char tag[(sizeof(serializer_header_tag) + 3) & (~3)];
    int32_t entry_count;
};




static char serializer_tail_tag[] = "[serializer tail]";

struct serializer_tail_t
{
    char tag[(sizeof(serializer_tail_tag) + 3) & (~3)];
};




struct serializer_t
{
    struct serializer_entry_t *entries;
    struct serializer_entry_t *last_entry;
    int entry_count;
};




void serializer_AddEntry(struct serializer_t *serializer, char *label, int size, void *buffer);

void serializer_RemoveEntry(struct serializer_t *serializer, char *label);

struct serializer_entry_t *serializer_GetEntry(struct serializer_t *serializer, char *label);

void serializer_Serialize(struct serializer_t *serializer, void **buffer, int *buffer_size);

void serializer_Deserialize(struct serializer_t *serializer, void **buffer);

void serializer_FreeSerializer(struct serializer_t *serializer, int free_buffers);

#endif // SERIALIZER_H













