#include "par_script.h"
#include "particle.h"

#include <stdlib.h>

extern int ps_current_particle_system;
extern int ps_frame;

#ifdef __cplusplus
extern "C"
{
#endif

void particle_ScriptDie()
{
	particle_MarkForRemoval(ps_current_particle_system);
}

int particle_ScriptGetLife()
{
	struct particle_system_t *particle_system;

	particle_system = particle_GetParticleSystemPointer(ps_current_particle_system);

	return ps_frame - particle_system->spawn_frame;
}

int particle_ScriptGetParticleSystemDef(struct script_string_t *name)
{
	char *def_name;

	def_name = script_string_GetRawString(name);
	return particle_GetParticleSystemDef(def_name);
}

void particle_ScriptSpawnParticleSystem(vec3_t *position, int particle_system_def)
{
	particle_SpawnParticleSystem(*position, vec3_t_c(1.0, 1.0, 1.0), NULL, particle_system_def);
}

#ifdef __cplusplus
}
#endif
