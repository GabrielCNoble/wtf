#version 400 compatibility

#include "engine/default_clustered_forward.h"

flat in int particle_index;

void main()
{
	gl_FragColor = texture2DArray(UNIFORM_texture_array_sampler0, vec3(tex_coords, float(UNIFORM_particle_frames[particle_index]))) * 10.0;	
}
