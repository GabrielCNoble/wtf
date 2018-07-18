#ifndef SCR_PARTICLE_H
#define SCR_PARTICLE_H

#include "script_types/scr_string.h"
#include "vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

void particle_ScriptDie();

int particle_ScriptGetLife();

int particle_ScriptGetParticleSystemDef(struct script_string_t *name);

void particle_ScriptSpawnParticleSystem(vec3_t *position, int particle_system_def);

#ifdef __cplusplus
}
#endif



#endif
