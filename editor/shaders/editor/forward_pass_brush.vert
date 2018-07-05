#version 400 compatibility
in vec4 vertex_position;
in vec4 vertex_normal;
in vec4 vertex_tangent;
in vec2 vertex_tex_coords;



out vec2 uv;
out vec3 world_space_normal;
out vec3 world_space_position;
out vec3 world_space_tangent;
out vec3 eye_space_position;

uniform mat4 UNIFORM_model_view_projection_matrix;
uniform mat4 UNIFORM_view_projection_matrix;
uniform mat4 UNIFORM_model_matrix;

void main()
{
	
	gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
	uv = vertex_tex_coords;
	world_space_normal = vec3(UNIFORM_model_matrix * vertex_normal);
	world_space_tangent = vec3(UNIFORM_model_matrix * vertex_tangent);
	world_space_position = vec3(UNIFORM_model_matrix * vertex_position);
	//normal = gl_NormalMatrix * vertex_normal.xyz;
	eye_space_position = vec3(UNIFORM_view_projection_matrix * vertex_position);
	
}
