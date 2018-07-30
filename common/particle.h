#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector.h"
#include "script.h"


#define MAX_PARTICLE_SYSTEM_PARTICLES 512
#define MIN_PARTICLE_SYSTEM_PARTICLES 1

#define MAX_PARTICLE_SYSTEM_PARTICLE_LIFE 1024
#define MIN_PARTICLE_SYSTEM_PARTICLE_LIFE 1

#define MAX_PARTICLE_SYSTEM_PARTICLE_RESPAWN_TIME 0xffff
#define MIN_PARTICLE_SYSTEM_PARTICLE_RESPAWN_TIME 0x0


enum PARTICLE_SYSTEM_FLAGS
{
	PARTICLE_SYSTEM_FLAG_SELF_DESTRUCT = 1,
	PARTICLE_SYSTEM_FLAG_INVALID = 1 << 1,
	PARTICLE_SYSTEM_FLAG_MARKED_INVALID = 1 << 2,
	PARTICLE_SYSTEM_FLAG_APPLY_ROTATION_TO_PARTICLES = 1 << 3,
	PARTICLE_SYSTEM_FLAG_JUST_SPAWNED = 1 << 4,
};


struct particle_system_script_t
{
	struct script_t script;	
	
	void *particle_position_array;
	void *particle_frame_array;
	void *particle_array;
	void *particle_system;
	
	void *on_spawn_entry_point;
};


typedef struct
{
	unsigned short max_particles;
	unsigned short max_life;
	unsigned short respawn_time;
	unsigned short flags;
	unsigned int texture;
	
	//script_t *script;
	struct particle_system_script_t *script;
	
	char *name;
}particle_system_def_t;



typedef struct
{
	//short life;
	//short frame;
	vec3_t velocity;
	int life;
}particle_t;

/*
typedef struct
{
	vec3_t velocity;
	vec3_t position;
}particle_position_t;*/

struct particle_system_t
{
	int def;
	
	short particle_count;
	short max_particles;
	short max_life;
	short max_frame;
	short respawn_time;
	short respawn_countdown;
	unsigned short flags;
	
	int spawn_frame;
	int texture;
	
	particle_t *particles;
	//vec3_t *particle_velocities;
	vec4_t *particle_positions;
	int *particle_frames;
	
	struct particle_system_script_t *script;
	
	mat3_t orientation;
	vec3_t scale;
	vec3_t position;
	
};


#ifdef __cplusplus
extern "C"
{
#endif

int particle_Init();

void particle_Finish();

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

int particle_CreateParticleSystemDef(char *name, short max_particles, short max_life, short respawn_time, short flags, unsigned int texture, struct particle_system_script_t *script);

void particle_DestroyParticleSystemDef(int particle_def_index);

particle_system_def_t *particle_GetParticleSystemDefPointerIndex(int def_index);

particle_system_def_t *particle_GetParticleSystemDefPointerName(char *def_index);

int particle_GetParticleSystemDef(char *def_name);

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

int particle_SpawnParticleSystem(vec3_t position, vec3_t scale, mat3_t *orientation, int particle_system_def);

void particle_RemoveParticleSystem(int particle_system);

struct particle_system_t *particle_GetParticleSystemPointer(int particle_system);

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

void particle_UpdateParticleSystems(double delta_time);

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

struct particle_system_script_t *particle_LoadParticleSystemScript(char *file_name, char *script_name);

#ifdef __cplusplus
}
#endif

#endif





