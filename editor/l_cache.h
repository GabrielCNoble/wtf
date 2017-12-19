#ifndef L_CACHE_H
#define L_CACHE_H

#include "l_common.h"

void light_CacheLight(int light_index);

void light_DropLight(int light_index);

void light_EvictOld();

void light_Update();

void light_BindCache();

void light_UnbindCache();


#endif
