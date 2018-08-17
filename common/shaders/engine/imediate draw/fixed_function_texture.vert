attribute vec4 vertex_position;
attribute vec4 vertex_tex_coords;
attribute vec4 vertex_color;


uniform mat4 UNIFORM_model_view_projection_matrix;

varying vec2 tex_coords;
varying vec4 color;

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
	tex_coords = vertex_tex_coords.xy;
	color = vertex_color;
}