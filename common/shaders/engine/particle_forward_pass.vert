#version 400 compatibility

#include "engine/default_clustered_forward.h"

flat out int particle_index;

void main()
{
	vec4 eye_space_vert = UNIFORM_model_view_matrix * vec4(UNIFORM_particle_positions[gl_InstanceID].xyz, 1.0);
	eye_space_vert.xy += vertex_position.xy * UNIFORM_particle_positions[gl_InstanceID].w;
	gl_Position = UNIFORM_projection_matrix * eye_space_vert;
	tex_coords = vertex_position.zw;
	
	particle_index = gl_InstanceID;
}
