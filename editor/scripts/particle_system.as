
//array<vec4_t> ps_particle_positions;
//array<int> ps_particle_frames;
//array<particle_t> ps_particles;
//particle_system_t @ps_particle_system;





void OnSpawn()
{
	/*ps_particle_system.max_particles = 360;
	ps_particle_system.respawn_time = 1;
	ps_particle_system.max_life = 360;*/
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
			
		particles[i].velocity.y += randfloat() * 0.01;
			
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

void main()
{
/*	int i;
	int particle_count;
	particle_count = ps_particle_system.particle_count;

	for(i = 0; i < particle_count; i++)
	{
		if(ps_particles[i].life == 1)
		{
			ps_particles[i].velocity.x = (randfloat() * 2.0 - 1.0) * 0.5;
			ps_particles[i].velocity.y = 0.0;
			ps_particles[i].velocity.z = (randfloat() * 2.0 - 1.0) * 0.5;
			ps_particle_positions[i].w = 0.01;
		}
			
		if(ps_particles[i].life >= ps_particle_system.max_life - 1)
		{
			ps_particle_frames[i] = 1;
		}
		else
		{
			ps_particle_frames[i] = 0;
		}
			
		ps_particles[i].velocity.y += randfloat() * 0.01;
			
		ps_particle_positions[i].x += ps_particles[i].velocity.x;
		ps_particle_positions[i].y += ps_particles[i].velocity.y;
		ps_particle_positions[i].z += ps_particles[i].velocity.z;
		
		ps_particle_positions[i].w += 0.075;		
		ps_particles[i].velocity.x *= 0.95;
		ps_particles[i].velocity.z *= 0.95;
	}
	
	if(particle_GetLife() > 30)
	{
		particle_Die();
	}*/
	
}
