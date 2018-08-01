


void OnSpawn(particle_system_t @particle_system, array<vec4_t> @particle_positions, array<particle_t> @particles, array<int> @particle_frames)
{
	particle_system.max_particles = 360;
	particle_system.respawn_time = 1;
	particle_system.max_life = 30;
	
	int i;
	
	for(i = 0; i < particle_system.particle_count; i++)
	{
		particles[i].velocity = vec3_t(0.0, 0.0, 0.0);
		particle_positions[i].w = 0.01;
	}
	
	
	//entity_Print("SPAWN!!\n");
}

void OnDie(particle_system_t @particle_system)
{
//	entity_Print("DEAD!!\n");
}

void OnUpdate(particle_system_t @particle_system, array<vec4_t> @particle_positions, array<particle_t> @particles, array<int> @particle_frames)
{
	int i;
	int particle_count;
	particle_count = particle_system.particle_count;

	for(i = 0; i < particle_count; i++)
	{
		if(particles[i].life == 1)
		{
			particles[i].velocity.x = (randfloat() * 2.0 - 1.0) * 0.5;
			particles[i].velocity.y = 0.0;
			particles[i].velocity.z = (randfloat() * 2.0 - 1.0) * 0.5;
			particle_positions[i].w = 0.01;
		}
			
		if(particles[i].life >= particle_system.max_life - 1)
		{
			particle_frames[i] = 1;
		}
		else
		{
			particle_frames[i] = 0;
		}
			
		particles[i].velocity.y += 0.025;
			
		particle_positions[i].x += particles[i].velocity.x;
		particle_positions[i].y += particles[i].velocity.y;
		particle_positions[i].z += particles[i].velocity.z;
		
		particle_positions[i].w += 0.075;		
		particles[i].velocity.x *= 0.95;
		particles[i].velocity.z *= 0.95;
	}
	
	if(particle_GetLife() > 30)
	{
		particle_Die();
	}
}

