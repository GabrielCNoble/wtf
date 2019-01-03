#ifndef SHD_COMMON_H
#define SHD_COMMON_H

#define LIGHT_PARAMS_UNIFORM_BUFFER_BINDING 0
#define TEST_UNIFORM_BUFFER_BINDING 1

enum SHADER_UNIFORMS
{
	UNIFORM_texture_sampler0 = 0,
	UNIFORM_texture_sampler1,
	UNIFORM_texture_sampler2,
	UNIFORM_texture_cube_sampler0,

	UNIFORM_texture_array_sampler0,

	UNIFORM_texture_cube_array_sampler0,

	UNIFORM_r_frame,
	UNIFORM_r_width,
	UNIFORM_r_height,
	UNIFORM_r_near,
	UNIFORM_r_far,
	UNIFORM_r_clusters_per_row,
	UNIFORM_r_cluster_rows,
	UNIFORM_r_cluster_layers,
	UNIFORM_r_bloom_radius,
	UNIFORM_r_world_vertices_count,
	UNIFORM_r_bsp_node_count,
	UNIFORM_cluster_texture,
	UNIFORM_material_flags,
	UNIFORM_projection_matrix,
	UNIFORM_view_matrix,
	UNIFORM_model_matrix,
	UNIFORM_model_view_matrix,
	UNIFORM_view_projection_matrix,
	UNIFORM_model_view_projection_matrix,

	UNIFORM_particle_positions,
	UNIFORM_particle_frames,


	UNIFORM_LAST_UNIFORM
};

enum SHADER_ATTRIBS
{
	ATTRIB_VERTEX_POSITION = 1,
	ATTRIB_VERTEX_NORMAL,
	ATTRIB_VERTEX_TANGENT,
	ATTRIB_VERTEX_TEX_COORDS,
};

enum SHADER_FLAGS
{
	SHADER_INVALID = 1,
};

typedef struct
{
	unsigned int location;
	unsigned int type;
}uniform_t;

typedef struct
{
	uniform_t uniform;
	char *name;
}named_uniform_t;

struct shader_t
{

	char *name;
	char *file_name;

	unsigned int shader_program;
	unsigned int vertex_shader;
	unsigned int fragment_shader;

	/*unsigned char vertex_position;
	unsigned char vertex_normal;
	unsigned char vertex_tangent;
	unsigned char vertex_tex_coords;
	unsigned char vertex_color;*/


	unsigned int vertex_position;
	unsigned int vertex_normal;
	unsigned int vertex_tangent;
	unsigned int vertex_tex_coords;
	unsigned int vertex_color;

	unsigned char align0;
	unsigned char align1;
	unsigned char align2;

	uniform_t *default_uniforms;

	unsigned char named_uniforms_list_cursor;
	unsigned char named_uniforms_list_size;
	named_uniform_t *named_uniforms;

	unsigned short shader_index;
	unsigned short bm_flags;

}shader_t;


#endif
