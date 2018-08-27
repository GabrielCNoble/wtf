
attribute vec4 vertex_position;
attribute vec4 vertex_color;
attribute vec4 vertex_tex_coords;

uniform mat4 UNIFORM_model_view_projection_matrix;


varying vec2 tex_coords;
varying vec4 color;

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
	color = vertex_color;
	tex_coords = vertex_tex_coords.xy;
}
