#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "matrix.h"

#include "gpu.h"
#include "particle.h"
#include "memory.h"

#include "script.h"
#include "stack_list.h"



int ps_particle_system_defs_count = 0;
int ps_max_particle_system_defs = 0;
particle_system_def_t *ps_particle_system_defs = NULL;
int ps_particle_system_defs_stack_top = -1;
int *ps_particle_system_defs_stack = NULL;


struct stack_list_t ps_particle_systems;

//int ps_particle_system_count = 0;
//int ps_max_particle_systems = 0;
//particle_system_t *ps_particle_systems = NULL;
//int ps_particle_systems_stack_top = -1;
//int *ps_particle_systems_stack = NULL;
 
int ps_particle_quad_handle;
int ps_particle_quad_start;


#define PS_FRAME_TIME 16.6

/* particle systems runs at about 60hz, regardless
of engine speed... */
int ps_frame = 0;
float ps_elapsed = 0.0;

#ifdef __cplusplus
extern "C"
{
#endif


void particle_DisposeParticleSystemCallback(void *particle_system)
{
	struct particle_system_t *ps;
	
	ps = (struct particle_system_t *)particle_system;
	
	if(ps->particles)
	{
		memory_Free(ps->particles);
		memory_Free(ps->particle_frames);
		memory_Free(ps->particle_positions);
	}
	
}


int particle_Init()
{
	int i;
	
	vec4_t particle_quad[4];
	
	ps_max_particle_system_defs = 32;
	ps_particle_system_defs = memory_Malloc(sizeof(particle_system_def_t) * ps_max_particle_system_defs, "particle_Init");
	ps_particle_system_defs_stack = memory_Malloc(sizeof(int) * ps_max_particle_system_defs, "particle_Init");
	
	for(i = 0; i < ps_max_particle_system_defs; i++)
	{
		ps_particle_system_defs[i].name = NULL;
	}
	
	//ps_max_particle_systems = 512;
	//ps_particle_systems = memory_Malloc(sizeof(particle_system_t) * ps_max_particle_systems, "particle_Init");
	//ps_particle_systems_stack = memory_Malloc(sizeof(int) * ps_max_particle_systems, "particle_Init");
	
	ps_particle_systems = stack_list_create(sizeof(struct particle_system_t), 512, particle_DisposeParticleSystemCallback);
	
	ps_particle_quad_handle = gpu_AllocAlign(sizeof(vec4_t) * 4, sizeof(vec4_t), 0);
	ps_particle_quad_start = gpu_GetAllocStart(ps_particle_quad_handle) / sizeof(vec4_t);
	
	particle_quad[0].x = -1.0;
	particle_quad[0].y = 1.0;
	particle_quad[0].z = 0.0;
	particle_quad[0].w = 1.0;
	
	particle_quad[1].x = -1.0;
	particle_quad[1].y = -1.0;
	particle_quad[1].z = 0.0;
	particle_quad[1].w = 0.0;
	
	particle_quad[2].x = 1.0;
	particle_quad[2].y = -1.0;
	particle_quad[2].z = 1.0;
	particle_quad[2].w = 0.0;
	
	particle_quad[3].x = 1.0;
	particle_quad[3].y = 1.0;
	particle_quad[3].z = 1.0;
	particle_quad[3].w = 1.0;
	
	gpu_Write(ps_particle_quad_handle, 0, particle_quad, sizeof(vec4_t) * 4);
	
	
	
	return 1;
}

void particle_Finish()
{
	int i;
	
	for(i = 0; i < ps_max_particle_system_defs; i++)
	{
		if(ps_particle_system_defs[i].name)
		{
			memory_Free(ps_particle_system_defs[i].name);
		}	
	}
	
	memory_Free(ps_particle_system_defs);
	memory_Free(ps_particle_system_defs_stack);
	
	stack_list_destroy(&ps_particle_systems);
	
	//memory_Free(ps_particle_systems);
	//memory_Free(ps_particle_systems_stack);
}

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

int particle_CreateParticleSystemDef(char *name, short max_particles, short max_life, short respawn_time, short flags, unsigned int texture, struct particle_system_script_t *script)
{
	particle_system_def_t *def;
	int def_index;
	int i;
	
	if(max_particles > MAX_PARTICLE_SYSTEM_PARTICLES)
	{
		max_particles = MAX_PARTICLE_SYSTEM_PARTICLES;
	}
	
	if(max_life > MAX_PARTICLE_SYSTEM_PARTICLE_LIFE)
	{
		max_life = MAX_PARTICLE_SYSTEM_PARTICLE_LIFE;
	}
	
	flags &= ~PARTICLE_SYSTEM_FLAG_INVALID;
	
	
	if(ps_particle_system_defs_stack_top >= 0)
	{
		def_index = ps_particle_system_defs_stack[ps_particle_system_defs_stack_top];
		ps_particle_system_defs_stack_top--;
	}
	else
	{
		def_index = ps_particle_system_defs_count;
		ps_particle_system_defs_count++;
		
		
		if(def_index >= ps_max_particle_system_defs)
		{
			memory_Free(ps_particle_system_defs_stack);
			ps_particle_system_defs_stack = memory_Malloc(sizeof(int) * (ps_max_particle_system_defs + 8), "particle_CreateParticleSystemDef");
			
			def = memory_Malloc(sizeof(particle_system_def_t) * (ps_max_particle_system_defs + 8), "particle_CreateParticleSystemDef");
			memcpy(def, ps_particle_system_defs, sizeof(particle_system_def_t) * ps_max_particle_system_defs);
			
			memory_Free(ps_particle_system_defs);
			
			ps_particle_system_defs = def;
			ps_max_particle_system_defs += 8;
			
			for(i = def_index; i < ps_max_particle_system_defs; i++)
			{
				ps_particle_system_defs[i].name = NULL;
			}
		}
	}
	
	def = &ps_particle_system_defs[def_index];
	
	def->max_life = max_life;
	def->max_particles = max_particles;
	def->respawn_time = respawn_time;
	def->texture = texture;
	def->script = script;
	def->flags = flags;
	def->name = memory_Strdup(name, "particle_CreateParticleSystemDef");
	
	return def_index;
}

void particle_DestroyParticleSystemDef(int def_index)
{
	if(particle_GetParticleSystemDefPointerIndex(def_index))
	{
		memory_Free(ps_particle_system_defs[def_index].name);
			
		ps_particle_system_defs_stack_top++;
			
		ps_particle_system_defs_stack[ps_particle_system_defs_stack_top] = def_index;
	}	
}

particle_system_def_t *particle_GetParticleSystemDefPointerIndex(int def_index)
{
	if(def_index >= 0 && def_index < ps_particle_system_defs_count)
	{
		if(ps_particle_system_defs[def_index].name)
		{
			return ps_particle_system_defs + def_index;
		}
	}
	
	return NULL;
}

particle_system_def_t *particle_GetParticleSystemDefPointerName(char *particle_def_name)
{
	int i;
	
	for(i = 0; i < ps_particle_system_defs_count; i++)
	{
		if(!strcmp(ps_particle_system_defs[i].name, particle_def_name))
		{
			return ps_particle_system_defs + i;
		}
	}
	
	return NULL;
}

int particle_GetParticleSystemDef(char *def_name)
{
	int i;
	
	for(i = 0; i < ps_particle_system_defs_count; i++)
	{
		if(!strcmp(ps_particle_system_defs[i].name, def_name))
		{
			return i;
		}
	}
	
	return -1;
}


/*
===========================================================================================
===========================================================================================
===========================================================================================
*/


int particle_SpawnParticleSystem(vec3_t position, vec3_t scale, mat3_t *orientation, int particle_system_def)
{
	particle_system_def_t *def;
	struct particle_system_t *ps;
	int ps_index;
	
	
	def = particle_GetParticleSystemDefPointerIndex(particle_system_def);
	
	if(!def)
	{
		printf("particle_SpawnParticleSystem: bad def index!\n");
		return -1;
	}
	
	
	ps_index = stack_list_add(&ps_particle_systems, NULL);
	
	
	/*if(ps_particle_systems_stack_top >= 0)
	{
		ps_index = ps_particle_systems_stack[ps_particle_systems_stack_top];
		ps_particle_systems_stack_top--;
	}
	else
	{
		ps_index = ps_particle_system_count++;
		
		if(ps_index >= ps_max_particle_systems)
		{
			memory_Free(ps_particle_systems_stack);
			ps_particle_systems_stack = memory_Malloc(sizeof(int) * (ps_max_particle_systems + 64), "particle_SpawnParticleSystem");
			
			ps = memory_Malloc(sizeof(particle_system_t) * (ps_max_particle_systems + 64), "particle_SpawnParticleSystem");
			memcpy(ps, ps_particle_systems, sizeof(particle_system_t) * ps_max_particle_systems);
			
			memory_Free(ps_particle_systems);
			ps_particle_systems = ps;
			ps_max_particle_systems += 64;
		}
	}*/
	
	
	//ps = ps_particle_systems + ps_index;
	//ps = particle_GetParticleSystemPointer(ps_index);
	ps = stack_list_get(&ps_particle_systems, ps_index);
	ps->def = particle_system_def;
	
	ps->flags = def->flags | PARTICLE_SYSTEM_FLAG_JUST_SPAWNED;
	
	
	if(!ps->particles)
	{
		ps->particles = memory_Malloc(sizeof(particle_t) * def->max_particles, "particle_SpawnParticleSystem");
		ps->particle_positions = memory_Malloc(sizeof(vec4_t) * def->max_particles, "particle_SpawnParticleSystem");
		ps->particle_frames = memory_Malloc(sizeof(int) * def->max_particles, "particle_SpawnParticleSystem");
	}
	else
	{
		if(ps->max_particles < def->max_particles)
		{
			memory_Free(ps->particles);
			memory_Free(ps->particle_positions);
			memory_Free(ps->particle_frames);
			
			ps->particles = memory_Malloc(sizeof(particle_t) * ps->max_particles, "particle_SpawnParticleSystem");
			ps->particle_positions = memory_Malloc(sizeof(vec4_t) * ps->max_particles, "particle_SpawnParticleSystem");
			ps->particle_frames = memory_Malloc(sizeof(int) * ps->max_particles, "particle_SpawnParticleSystem");
		}
		
		
	}
	
	
	
	
	//ps->max_frame = def->max_frame;
	ps->max_frame = 1;
	ps->max_life = def->max_life;
	ps->max_particles = def->max_particles;
	ps->respawn_time = def->respawn_time;
	ps->respawn_countdown = 0;
	ps->particle_count = 0;
	ps->position = position;
	ps->scale = scale;
	
	if(!orientation)
	{
		ps->orientation = mat3_t_id();
	}
	else
	{
		ps->orientation = *orientation;
	}
	
	
	ps->script = def->script;
	ps->spawn_frame = ps_frame;
	
	ps->texture = def->texture;
	
	/* particle system spawing is supposed to be a fast operation, and 
	malloc is everything but fast... */
	
	
	//ps->particle_velocities = memory_Malloc(sizeof(vec3_t) * ps->max_particles, "particle_SpawnParticleSystem");
	
}

void particle_RemoveParticleSystem(int particle_system)
{
	struct particle_system_t *ps;
	
	ps = particle_GetParticleSystemPointer(particle_system);
	
	if(ps)
	{
		if(!(ps->flags & PARTICLE_SYSTEM_FLAG_INVALID))
		{
			ps->flags |= PARTICLE_SYSTEM_FLAG_MARKED_INVALID;
		}
	}
	
	/*if(particle_system >= 0 && particle_system < ps_particle_systems.element_count)
	{
		ps = (particle_system_t *)ps_particle_systems.elements + particle_system_index;
		
		if(!(ps->flags & PARTICLE_SYSTEM_FLAG_INVALID))
		{
			ps->flags |= PARTICLE_SYSTEM_FLAG_MARKED_INVALID;
		}
	}*/
	
}

void particle_DeallocParticleSystem(int particle_system)
{
	struct particle_system_t *ps;
	
	ps = particle_GetParticleSystemPointer(particle_system);
	
	if(ps)
	{
		if(!(ps->flags & PARTICLE_SYSTEM_FLAG_INVALID))
		{
			ps->flags &= ~PARTICLE_SYSTEM_FLAG_MARKED_INVALID;
			ps->flags |= PARTICLE_SYSTEM_FLAG_INVALID;
			
			stack_list_remove(&ps_particle_systems, particle_system);
		}
	}
}


struct particle_system_t *particle_GetParticleSystemPointer(int particle_system)
{
	struct particle_system_t *ps;
	
	if(particle_system >= 0 && particle_system < ps_particle_systems.element_count)
	{
		ps = (struct particle_system_t *)ps_particle_systems.elements + particle_system;
		
		if(!(ps->flags & PARTICLE_SYSTEM_FLAG_INVALID))
		{
			return ps;
		}
	}
	
	return NULL;
	
}


/*
===========================================================================================
===========================================================================================
===========================================================================================
*/


void particle_UpdateParticleSystems(double delta_time)
{
	int i;
	int c;
	int j;
	int k;
	int particle_count;
	int spawn_particle_count = 0;
	int current_respawn_countdown = 0;
	int particle_life;
	
	int elapsed_frames = 0;
	
	struct particle_system_t *ps;
	struct particle_system_t *particle_systems;
	//elapsed_frames = (int)(delta_time / PS_FRAME_TIME);
	
	ps_elapsed += delta_time;
	
	elapsed_frames = (int)(ps_elapsed / PS_FRAME_TIME);
	
	if(!elapsed_frames)
	{
		/* didn't pass a whole frame since
		last update, so do nothing... */
		return;
	}
	
	ps_elapsed -= PS_FRAME_TIME * elapsed_frames;
	
	ps_frame += elapsed_frames;
	
	particle_systems = (struct particle_system_t *)ps_particle_systems.elements;
	c = ps_particle_systems.element_count;
	for(i = 0; i < c; i++)
	{
		if(particle_systems[i].flags & PARTICLE_SYSTEM_FLAG_INVALID)
		{
			continue;
		}
					
		ps = particle_systems + i;
			
		
		if(!(ps->flags & PARTICLE_SYSTEM_FLAG_MARKED_INVALID))
		{
			if(ps->respawn_time)
			{
				/* how many times we went over the respawn
				threshold since last update... */	
				
				ps->respawn_countdown += elapsed_frames;
				spawn_particle_count = ps->respawn_countdown;
				
				spawn_particle_count /= ps->respawn_time;
				ps->respawn_countdown %= ps->respawn_time;
				
				//current_respawn_countdown = ps->respawn_countdown;
			}
			else
			{
				ps->respawn_countdown = 0;
				spawn_particle_count = ps->max_particles;
			}
			
			//current_respawn_countdown = ps->respawn_countdown;
			
				
			if(spawn_particle_count)
			{	
				for(k = spawn_particle_count; k > 0; k--)
				{
					if(ps->particle_count >= ps->max_particles)
					{
						break;
					}
					
					/* each particle created starts with life == 0, and it gets
					incremented each 16.66 ms. If more time than that has passed
					since last time we checked, we need to account for that when 
					creating new particles... */
					particle_life = ps->respawn_time * (k - 1) + ps->respawn_countdown;
					
					if(particle_life >= ps->max_life)
					{
						/* don't bother creating a particle
						we know will be deleted in the next
						loop... */
						continue;
					}
					
					ps->particles[ps->particle_count].life = particle_life;
							
					ps->particle_positions[ps->particle_count].x = ps->position.x;
					ps->particle_positions[ps->particle_count].y = ps->position.y;
					ps->particle_positions[ps->particle_count].z = ps->position.z;
					ps->particle_positions[ps->particle_count].w = 1.0;
					ps->particle_frames[ps->particle_count] = 0;		
					ps->particle_count++;
				}
			}
		}
		else
		{
			if(!particle_systems[i].particle_count)
			{
				particle_DeallocParticleSystem(i);
				continue;
			}
		}
		

			 
		//ps->respawn_countdown++;			
		for(j = 0; j < ps->particle_count; j++)
		{	
			if(ps->particles[j].life >= ps->max_life)
			{
				if(j <= ps->particle_count - 1)
				{
					ps->particles[j] = ps->particles[ps->particle_count - 1];
					ps->particle_positions[j] = ps->particle_positions[ps->particle_count - 1];
					j--;
				}
					
				ps->particle_count--;	
				continue;
			}
			//ps->particles[j].life++;
			ps->particles[j].life += elapsed_frames;
		}
			
		if(ps->script)
		{
			/* do elapsed_frames updates on the particle system, so we take away from
			the scripts the responsibility of properly accounting for variable frame rate... */
			
			/* NOTE: this can lock up the engine if too many frames passes
			without update... */
			//for(j = 0; j < elapsed_frames; j++)
			{
				//script_ExecuteScriptImediate(ps->script, ps, particle_SetupParticleScriptCallback);
				script_ExecuteScriptImediate((struct script_t *)ps->script, (void *)i);
			}
		}
		
		ps->flags &= ~PARTICLE_SYSTEM_FLAG_JUST_SPAWNED;
			
	}
	//}
	
}


/*
===========================================================================================
===========================================================================================
===========================================================================================
*/


int ps_current_particle_system;

void *particle_SetupScriptDataCallback(struct script_t *script, void *particle_system)
{
	struct particle_system_script_t *ps_script = (struct particle_system_script_t *)script;
	//struct particle_system_t *ps = (struct particle_system_t *)particle_system;
	struct particle_system_t *ps;
	void *entry_point = NULL;
	
	ps = stack_list_get(&ps_particle_systems, (int)particle_system);
	ps_current_particle_system = (int)particle_system;
				
	((struct script_array_t *)ps_script->particle_array)->buffer = ps->particles;
	((struct script_array_t *)ps_script->particle_frame_array)->buffer = ps->particle_frames;
	((struct script_array_t *)ps_script->particle_position_array)->buffer = ps->particle_positions;
	*((struct particle_system_t **)ps_script->particle_system) = ps;
	
	
				
	if(ps->flags & PARTICLE_SYSTEM_FLAG_JUST_SPAWNED)
	{
		entry_point = ps_script->init;
	}
	else
	{
		entry_point = ps_script->script.main_entry_point;
	}
	
	return entry_point;
}

int particle_GetScriptDataCallback(struct script_t *script)
{
	int success = 1;
	struct particle_system_script_t *ps_script;
	
	ps_script = (struct particle_system_script_t *)script;
					
	ps_script->particle_array = script_GetGlobalVarAddress("ps_particles", script);
	ps_script->particle_frame_array = script_GetGlobalVarAddress("ps_particle_frames", script);
	ps_script->particle_position_array = script_GetGlobalVarAddress("ps_particle_positions", script);
	ps_script->particle_system = script_GetGlobalVarAddress("ps_particle_system", script);
	ps_script->init = script_GetFunctionAddress("ps_init", script);
				
	if(!ps_script->particle_array)
	{
		printf("particle_GetScriptDataCallback: script [%s] is missing global var [array<particle_t> ps_particles]\n", script->name);
		success = 0;
	}
				
	if(!ps_script->particle_frame_array)
	{
		printf("script_CompileScriptSource: script [%s] is missing global var [array<int> ps_particle_frames]\n", script->name);
		success = 0;
	}
				
	if(!ps_script->particle_position_array)
	{
		printf("script_CompileScriptSource: script [%s] is missing global var [array<vec4_t> ps_particle_positions]\n", script->name);
		success = 0;
	}
				
	if(!ps_script->particle_system)
	{
		printf("script_CompileScriptSource: script [%s] is missing global var [particle_system_t@ ps_particle_system]\n", script->name);
		success = 0;
	}
				
	if(!ps_script->init)
	{
		printf("script_CompileScriptSource: script [%s] is missing function [void ps_init()]\n", script->name);
		success = 0;
	}
	
	return success;
}


struct particle_system_script_t *particle_LoadParticleSystemScript(char *file_name, char *script_name)
{	
	return (struct particle_system_script_t *)script_LoadScript(file_name, script_name, sizeof(struct particle_system_script_t), particle_GetScriptDataCallback, particle_SetupScriptDataCallback);
}

/*
===========================================================================================
===========================================================================================
===========================================================================================
*/

//vec2_t test_ref = {0.0, 0.0};

//test_type_t test_type = {0, 0, "app", 1}; 




#ifdef __cplusplus
}
#endif













