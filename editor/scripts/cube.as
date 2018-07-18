

void OnFirstRun()
{
	//entity_Print("fuck!\n");
}

void OnDie()
{
	entity_Print("I'm dead as fuck!\n");
}

void main()
{
	
	int particle_system_def;
	vec3_t position;
	
	if(entity_GetLife() >= 240)
	{
		particle_system_def = particle_GetParticleSystemDef("particle system");
		entity_GetPosition(position, 0);
		particle_SpawnParticleSystem(position, particle_system_def);
		entity_DIEYOUMOTHERFUCKER();
	}
}
