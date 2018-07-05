#version 400 compatibility  

in vec4 vertex_position;
in vec2 vertex_tex_coords;


out vec2 tex_coords;


uniform mat4 UNIFORM_model_view_projection_matrix;
uniform mat4 UNIFORM_view_projection_matrix;
uniform mat4 UNIFORM_model_matrix;
uniform mat4 UNIFORM_model_view_matrix;


void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vec4(vertex_position.xyz, 1.0);
	tex_coords = vertex_tex_coords;
}