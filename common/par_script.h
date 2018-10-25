#ifndef PAR_SCRIPT_H
#define PAR_SCRIPT_H

#include "script_types/scr_string.h"
#include "vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

void particle_ScriptParticleSystemConstructor();

void particle_ScriptDie();

int particle_ScriptGetLife();

int particle_ScriptGetParticleSystemDef(struct script_string_t *name);

void particle_ScriptSpawnParticleSystem(vec3_t *position, int particle_system_def);

#ifdef __cplusplus
}
#endif



#endif  // PAR_SCRIPT_H
