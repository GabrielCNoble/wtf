#ifndef ID_CACHE_H
#define ID_CACHE_H



#include "containers/list.h"


struct id_cache_entry_t
{
    char *base_name;
    struct list_t indexes;
};


struct id_cache_t
{
    struct list_t entries;
};





struct id_cache_t idcache_Create();

void idcache_Destroy(struct id_cache_t *cache);

char *idcache_AllocUniqueName(struct id_cache_t *cache, char *name);

void idcache_FreeUniqueName(struct id_cache_t *cache, char *name);

void idcache_RemoveEntry(struct id_cache_t *cache, char *base_name);






#endif // ID_CACHE_H
