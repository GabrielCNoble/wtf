#version 400 compatibility  

in vec4 vertex_position;
in vec4 vertex_normal;
in vec4 vertex_tangent;
in vec2 vertex_tex_coords;



out vec2 tex_coords;
out vec3 eye_space_normal;
out vec3 eye_space_position;
out vec3 eye_space_tangent;

uniform mat4 UNIFORM_model_view_projection_matrix;
uniform mat4 UNIFORM_view_projection_matrix;
uniform mat4 UNIFORM_model_matrix;
uniform mat4 UNIFORM_model_view_matrix;

uniform int UNIFORM_r_width;
uniform int UNIFORM_r_height;

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vec4(vertex_position.xyz, 1.0);
	tex_coords = vertex_tex_coords;
	eye_space_normal = vec3(UNIFORM_model_view_matrix * vec4(vertex_normal.xyz, 0.0));
	eye_space_tangent = vec3(UNIFORM_model_view_matrix * vec4(vertex_tangent.xyz, 0.0));
	eye_space_position = vec3(UNIFORM_model_view_matrix * vec4(vertex_position.xyz, 1.0));
	
	//eye_space_normal.z = -eye_space_normal.z;


}
