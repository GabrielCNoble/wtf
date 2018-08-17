attribute vec4 vertex_position;
attribute vec4 vertex_normal;
attribute vec4 vertex_tex_coords;
attribute vec4 vertex_color;


uniform mat4 UNIFORM_model_view_projection_matrix;


varying vec4 color;
varying vec4 tex_coords;

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
	color = vertex_color;
	tex_coords = vertex_tex_coords;
}