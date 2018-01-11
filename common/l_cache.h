#ifndef L_CACHE_H
#define L_CACHE_H

#include "l_common.h"


typedef struct light_cache_slot_t
{
	unsigned int last_touched;
	unsigned short light_index;
	unsigned short offset;
}light_cache_slot_t;

typedef struct
{
	short x;
	short y;
	short w;
	short h;
}ks_chunk_t;

void light_InitCache();

void light_FinishCache();

/* tries to put a light into the cache... */
void light_CacheLight(int light_index);

void light_DropLight(int light_index);

/* drop not recently used lights
from the cache... */
void light_EvictOld();

/* upload the cached data to the gpu... */
void light_UploadCache();

/* make the cache visible to the shaders... */
void light_BindCache();

void light_UnbindCache();

//void light_UpdateCacheGroups();

/* upload the indexes of a light to
the gpu... */
void light_UploadIndexes(int light_index);

#endif