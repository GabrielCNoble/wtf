


void main()
{
	
	int particle_system_def;
	vec3_t position;
	
	entity_handle_t current_entity = entity_GetCurrentEntity();
	
	if(entity_GetEntityProp1i(current_entity, "enemy") != -1)
	{
		if(entity_IncEntityProp1i(current_entity, "enemy") >= 120)
		{
			entity_DIEYOUMOTHERFUCKER();
		}
	}
}

void OnSpawn()
{
	entity_handle_t current_entity = entity_GetCurrentEntity();
	entity_AddEntityProp1i(current_entity, "enemy");
	entity_SetEntityProp1i(current_entity, "enemy", -1);
}

void OnDie()
{
//	int particle_system_def = particle_GetParticleSystemDef("particle system");
//	entity_handle_t current_entity = entity_GetCurrentEntity();
//	vec3_t position = entity_GetEntityPosition(current_entity, 0);
//	particle_SpawnParticleSystem(position, particle_system_def);
}


void OnCollision(array<entity_handle_t> @collided_entities)
{
	int i;
	int value;
	entity_handle_t entity;
	entity_handle_t current_entity;
	
	for(i = 0; i < collided_entities.count; i++)
	{
		entity = collided_entities[i];
		
		if(entity_EntityHasProp(entity, "bullet") != 0)
		{
			current_entity = entity_GetCurrentEntity();
					
			if(entity_GetEntityProp1i(current_entity, "enemy") < 0)
			{
				entity_SetEntityProp1i(current_entity, "enemy", 0);
			}
		}
	}	
}





