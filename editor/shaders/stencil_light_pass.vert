#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_uniform_buffer_object : enable
#extension GL_ARB_arrays_of_arrays : enable

attribute vec4 vertex_position;

#define LIGHT_CACHE_SIZE 32

#define vec4_t vec4


struct light_params_fields
{
	vec4_t forward_axis;
	vec4_t position_radius;
	vec4_t color_energy;
	int bm_flags;
	int x_y;
	int align0;
	int align1;
};

layout(std140) uniform light_params_uniform_block
{
	light_params_fields light_params[LIGHT_CACHE_SIZE];
};

uniform int light_index;


void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vec4(vertex_position.xyz * (light_params[light_index].position_radius.a + 0.5) + light_params[light_index].position_radius.xyz, 1.0);	
}
