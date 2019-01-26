#include <stdio.h>
#include <string.h>
#include "serializer.h"
#include "c_memory.h"

void serializer_AddEntry(struct serializer_t *serializer, char *label, int size, void *buffer)
{
    struct serializer_entry_t *entry;

    if(size > 0)
    {
        entry = memory_Calloc(sizeof(struct serializer_entry_t ), 1);

        entry->data.size = size;
        strcpy(entry->data.label, label);
        entry->entry_buffer = buffer;


        if(!serializer->entries)
        {
            serializer->entries = entry;
        }
        else
        {
            serializer->last_entry->next = entry;
        }

        serializer->last_entry = entry;
        serializer->entry_count++;
    }
}

void serializer_RemoveEntry(struct serializer_t *serializer, char *label)
{
    struct serializer_entry_t *entry = NULL;
    struct serializer_entry_t *prev_entry = NULL;

    entry = serializer->entries;

    while(entry)
    {
        if(!strcmp(label, entry->data.label))
        {
            if(!prev_entry)
            {
                /* first entry... */
                serializer->entries = entry->next;
            }
            else
            {
                prev_entry->next = entry->next;
            }

            if(!entry->next)
            {
                /* if this entry's next pointer is NULL,
                it means it's the last entry in the list.
                In this case, set the last entry in the
                serializer to the previous entry, even if
                it's NULL (this entry was the only one)... */
                serializer->last_entry = prev_entry;
            }

            memory_Free(entry);

            serializer->entry_count--;

            return;
        }

        prev_entry = entry;
        entry = entry->next;
    }
}

struct serializer_entry_t *serializer_GetEntry(struct serializer_t *serializer, char *label)
{
    struct serializer_entry_t *entry = NULL;

    entry = serializer->entries;

    while(entry)
    {
        if(!strcmp(entry->data.label, label))
        {
            break;
        }

        entry = entry->next;
    }

    return entry;
}

void serializer_Serialize(struct serializer_t *serializer, void **buffer, int *buffer_size)
{
    struct serializer_header_t *header;
    struct serializer_tail_t *tail;
    struct serializer_entry_data_t *entry_data;

    void *serializer_buffer = NULL;
    char *out = NULL;
    int serializer_buffer_size = 0;
    int first_entry_offset;

    struct serializer_entry_t *entry;


    serializer_buffer_size = sizeof(struct serializer_header_t) +
                             sizeof(struct serializer_entry_data_t) * serializer->entry_count;

    first_entry_offset = serializer_buffer_size;

    serializer_buffer_size += sizeof(struct serializer_tail_t);

    entry = serializer->entries;

    while(entry)
    {
        /* make sure the buffer is even-sized... */
        //serializer_buffer_size += (entry->data.size + 3) & (~3);
        serializer_buffer_size += entry->data.size;
        entry = entry->next;
    }

    serializer_buffer = memory_Calloc(serializer_buffer_size, 1);
    *buffer = serializer_buffer;
    *buffer_size = serializer_buffer_size;


    out = serializer_buffer;


    header = (struct serializer_header_t *)out;
    out += sizeof(struct serializer_header_t);

    strcpy(header->tag, serializer_header_tag);
    header->buffer_size = serializer_buffer_size;


    entry_data = (struct serializer_entry_data_t *)out;
    out += sizeof(struct serializer_entry_data_t ) * serializer->entry_count;

    entry = serializer->entries;

    header->entry_count = 0;

    while(entry)
    {
        memcpy(entry_data, &entry->data, sizeof(struct serializer_entry_data_t));

        if(header->entry_count)
        {
            entry_data->offset = (entry_data - 1)->offset + (entry_data - 1)->size;
        }
        else
        {
            entry_data->offset = first_entry_offset;
        }

        memcpy(out, entry->entry_buffer, entry->data.size);

        /* make sure every entry begins at 4 byte boundary... */
        //out += (entry_data->size + 3) & (~3);
        out += entry_data->size;

        header->entry_count++;
        entry_data++;

        entry = entry->next;
    }

    tail = (struct serializer_tail_t *)out;
    out += sizeof(struct serializer_tail_t );

    strcpy(tail->tag, serializer_tail_tag);

}

void serializer_Deserialize(struct serializer_t *serializer, void **buffer)
{
    struct serializer_header_t *header;
    struct serializer_tail_t *tail;

    struct serializer_entry_data_t *entries_data;
    struct serializer_entry_data_t *entry_data;


    struct serializer_entry_t *entry;

    char *in;
    void *buffer_start;
    int i;


    in = *(char **)buffer;

    while(1)
    {
        if(!strcmp(serializer_header_tag, in))
        {
            break;
        }
        else if(!strcmp(serializer_tail_tag, in))
        {
            return;
        }

        in++;
    }

    buffer_start = in;

    header = (struct serializer_header_t *)in;
    in += sizeof(struct serializer_header_t );

    serializer->entry_count = header->entry_count;
    serializer->entries = NULL;
    serializer->last_entry = NULL;


    entries_data = (struct serializer_entry_data_t *)in;
    in += sizeof(struct serializer_entry_data_t ) * header->entry_count;

    for(i = 0; i < header->entry_count; i++)
    {
        entry_data = entries_data + i;

        entry = memory_Calloc(1, sizeof(struct serializer_entry_t));

        memcpy(&entry->data, entry_data, sizeof(struct serializer_entry_data_t));


        entry->entry_buffer = memory_Calloc(entry_data->size, 1);
        memcpy(entry->entry_buffer, buffer_start + entry_data->offset, entry_data->size);

        //entry->entry_buffer = buffer_start + entry_data->offset;

        if(!serializer->entries)
        {
            serializer->entries = entry;
        }
        else
        {
            serializer->last_entry->next = entry;
        }
        serializer->last_entry = entry;

        in += entry->data.size;
    }

    tail = (struct serializer_tail_t *)in;
    in += sizeof(struct serializer_tail_t );

    *buffer = in;
}

void serializer_FreeSerializer(struct serializer_t *serializer, int free_entry_buffers)
{
    struct serializer_entry_t *entry;
    struct serializer_entry_t *next_entry;

    entry = serializer->entries;

    while(entry)
    {
        next_entry = entry->next;

        if(free_entry_buffers)
        {
            memory_Free(entry->entry_buffer);
        }

        memory_Free(entry);

        entry = next_entry;
    }

    serializer->entries = NULL;
    serializer->last_entry = NULL;
    serializer->entry_count = 0;
}






