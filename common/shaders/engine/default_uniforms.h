#ifndef MASSACRE_DEFAULT_UNIFORMS
#define MASSACRE_DEFAULT_UNIFORMS

#extension GL_EXT_gpu_shader4 : require

#define MAX_PARTICLES 512

#ifdef MASSACRE_FRAGMENT_SHADER

	#define R_LIGHT_UNIFORM_BUFFER_SIZE 32
	#define R_BSP_UNIFORM_BUFFER_SIZE 512

	#define SHADOW_MAP_RESOLUTION 512
	#define SHARED_SHADOW_MAP_HEIGHT 8192
	#define SHARED_SHADOW_MAP_WIDTH 8192
	#define SHADOW_MAP_WIDTH 0.0625
	#define SHADOW_MAP_HEIGHT 0.0833333333333333


	#define LIGHT_CACHED 1
	#define LIGHT_MOVED (1 << 1)
	#define LIGHT_NEEDS_REUPLOAD (1 << 2)
	#define LIGHT_GENERATE_SHADOWS (1 << 3)
	#define LIGHT_INVALID (1 << 4)

	#define CLUSTERS_PER_ROW 32
	#define CLUSTER_ROWS 16
	#define CLUSTER_LAYERS 24


	#define MATERIAL_USE_DIFFUSE_TEXTURE (1<<1)
	#define MATERIAL_USE_NORMAL_TEXTURE (1<<2)
	#define MATERIAL_USE_HEIGHT_TEXTURE (1<<3)
	#define MATERIAL_USE_ROUGHNESS_TEXTURE (1<<4)
	#define MATERIAL_USE_METALNESS_TEXTURE (1<<5)

	#define MATERIAL_INVERT_NORMAL_X (1<<6)
	#define MATERIAL_INVERT_NORMAL_Y (1<<7)
	#define MATERIAL_USE_CUSTOM_SHADER (1<<8)
	#define MATERIAL_TRANSLUCENT (1<<9)


	struct gpu_light_t
	{
		vec4 forward_axis;
		vec4 position_radius;
		vec4 color_energy;
		int bm_flags;
		float proj_param_a;
		float proj_param_b;
		int shadow_map;
	};


    struct gpu_bsp_node_t
    {
        vec4 normal_dist;
        unsigned int children[2];
        int node_count;
        int align1;
    };


	layout(std140) uniform r_lights_uniform_block
	{
		gpu_light_t r_lights[R_LIGHT_UNIFORM_BUFFER_SIZE];
	};

	layout(std140) uniform r_bsp_uniform_block
	{
        gpu_bsp_node_t r_bsp[R_BSP_UNIFORM_BUFFER_SIZE];
	};

	uniform int UNIFORM_r_bsp_node_count;

	uniform sampler2D UNIFORM_texture_sampler0;
	uniform sampler2D UNIFORM_texture_sampler1;
	uniform sampler2D UNIFORM_texture_sampler2;
	uniform samplerCube UNIFORM_texture_cube_sampler0;
	uniform sampler2DArray UNIFORM_texture_array_sampler0;

	uniform samplerCubeArray UNIFORM_texture_cube_array_sampler0;

	uniform int UNIFORM_r_bloom_radius;
	uniform unsigned int UNIFORM_material_flags;
	uniform usampler3D UNIFORM_cluster_texture;

	uniform int UNIFORM_particle_frames[MAX_PARTICLES];

#else

	uniform vec4 UNIFORM_particle_positions[MAX_PARTICLES];

#endif

uniform mat4 UNIFORM_projection_matrix;
uniform mat4 UNIFORM_model_view_projection_matrix;
uniform mat4 UNIFORM_view_projection_matrix;
uniform mat4 UNIFORM_model_matrix;
uniform mat4 UNIFORM_view_matrix;
uniform mat4 UNIFORM_model_view_matrix;




uniform int UNIFORM_r_frame;
uniform int UNIFORM_r_width;
uniform int UNIFORM_r_height;
uniform int UNIFORM_r_clusters_per_row;
uniform int UNIFORM_r_cluster_rows;
uniform int UNIFORM_r_cluster_layers;
uniform float UNIFORM_r_near;
uniform float UNIFORM_r_far;


#endif


