#ifndef L_COMMON_H
#define L_COMMON_H

#define LIGHT_MAX_RADIUS 250.0
#define LIGHT_MIN_RADIUS 1.0

#define LIGHT_MAX_ENERGY 500.0
#define LIGHT_MIN_ENERGY 1.0

#define MAX_LIGHTS 128
#define MAX_WORLD_LIGHTS 512

#define LIGHT_CACHE_SIZE 128
#define LIGHT_INDEX_BUFFER_SIZE 0x10000
#define LIGHT_SHADOW_MAP_MAX_DATA 0x7fff

#define LIGHT_RADIUS(radius) (LIGHT_MIN_RADIUS+(LIGHT_MAX_RADIUS-LIGHT_MIN_RADIUS)*((float)((unsigned short)radius)/0xffff))
#define LIGHT_ENERGY(energy) (LIGHT_MIN_ENERGY+(LIGHT_MAX_ENERGY-LIGHT_MIN_ENERGY)*((float)((unsigned short)energy)/0xffff))


#endif
