#include "c_memory.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "id_cache.h"




struct id_cache_t idcache_Create()
{
    struct id_cache_t cache;
    cache.entries = list_create(sizeof(struct id_cache_entry_t), 32, NULL);

    return cache;
}

void idcache_Destroy(struct id_cache_t *cache)
{
    struct id_cache_entry_t *entries;
    int c;
    int i;


    if(cache)
    {
        entries = (struct id_cache_entry_t *)cache->entries.elements;
        c = cache->entries.element_count;

        for(i = 0; i < c; i++)
        {
            memory_Free(entries[i].base_name);
            list_destroy(&entries[i].indexes);
        }

        list_destroy(&cache->entries);
    }
}

void idcache_BreakName(char *name, char *base_name, char *suffix)
{
    base_name[0] = '\0';
    suffix[0] = '\0';
    int i;

    int dot_index = 0;

    for(i = 0; name[i]; i++)
    {
        if(name[i] == '.')
        {
            dot_index = i;
        }

        base_name[i] = name[i];
    }

    base_name[dot_index ? dot_index : i] = '\0';

    if(dot_index)
    {
        strcpy(suffix, name + dot_index + 1);
    }
}

char *idcache_AllocUniqueName(struct id_cache_t *cache, char *name)
{
    struct id_cache_entry_t *entry;
    struct id_cache_entry_t *entries;
    int *indexes;
    int c;
    int i;

    int index_index;
    int index_count;
    int bit_index;

    static char ret[1024];
    static char number_str[16];


    if(cache)
    {
        entries = (struct id_cache_entry_t *)cache->entries.elements;
        c = cache->entries.element_count;

        idcache_BreakName(name, ret, number_str);

        for(i = 0; i < c; i++)
        {
            entry = entries + i;
            if(!strcmp(entry->base_name, ret))
            {
                indexes = (int *)entry->indexes.elements;
                index_count = entry->indexes.element_count;

                if(number_str[0])
                {
                    index_index = atoi(number_str);
                    bit_index = index_index % 32;
                    index_index <<= 5;

                    if(index_index > index_count)
                    {
                        list_resize(&entry->indexes, index_index);
                        indexes = (int *)entry->indexes.elements;
                    }

                    if(!(indexes[index_index] & (1 << bit_index)))
                    {
                        strcpy(ret, name);
                        indexes[index_index] |= 1 << bit_index;
                        return ret;
                    }
                }


                for(index_index = 0; index_index < index_count; index_index++)
                {
                    for(bit_index = 0; bit_index < 32; bit_index++)
                    {
                        if(!(indexes[index_index] & (1 << bit_index)))
                        {
                            strcpy(ret, entry->base_name);

                            if(index_index || bit_index)
                            {
                                /* if the suffix number is different
                                from zero, we add it to the name.

                                Name is the same as Name.0000 ... */
                                sprintf(number_str, "%d", (index_index << 5) + bit_index);
                                strcat(ret, ".");
                                strcat(ret, number_str);

                                printf("%s\n", ret);
                            }

                            indexes[index_index] |= 1 << bit_index;

                            return ret;
                        }
                    }

                    bit_index = 0;
                }
            }
        }

        /* if we got here, we'll need to add a new entry
        to the cache... */

        i = list_add(&cache->entries, NULL);

        entry = (struct id_cache_entry_t *)list_get(&cache->entries, i);

        entry->base_name = memory_Strdup(name);
        entry->indexes = list_create(sizeof(int), 32, NULL);
        list_add(&entry->indexes, NULL);

        /* set the first bit of the first int
        to one, to signal we got the first name
        of this entry... */
        *(int *)entry->indexes.elements |= 1;

        strcpy(ret, entry->base_name);

        return ret;
    }

    return NULL;
}

void idcache_FreeUniqueName(struct id_cache_t *cache, char *name)
{
    struct id_cache_entry_t *entry;
    int i;
    int c;
    int suffix_index;
    int index_index;
    int bit_index;
    int *indexes;

    static char base_name[512];
    static char suffix[512];

    base_name[0] = '\0';
    suffix[0] = '\0';



    if(cache)
    {
        entry = (struct id_cache_entry_t *)cache->entries.elements;
        c = cache->entries.element_count;

//        for(i = 0; name[i]; i++)
//        {
//            if(name[i] == '.')
//            {
//                base_name[i] = '\0';
//
//                strcpy(suffix, name + i);
//
//                break;
//            }
//            base_name[i] = name[i];
//        }

        idcache_BreakName(name, base_name, suffix);

        suffix_index = 0;

        if(suffix[0] != '\0')
        {
            suffix_index = atoi(suffix);
        }

        index_index = suffix_index >> 5;
        bit_index = suffix_index % 32;

        for(i = 0; i < c; i++)
        {
            if(!strcmp(base_name, entry->base_name))
            {
                indexes = (int *)entry->indexes.elements;
                indexes[index_index] &= ~(1 << bit_index);
            }

            entry++;
        }
    }
}

void idcache_RemoveEntry(struct id_cache_t *cache, char *base_name)
{

}







