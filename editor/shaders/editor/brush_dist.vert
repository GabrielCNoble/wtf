attribute vec4 vertex_position;

varying vec4 position;

uniform mat4 UNIFORM_model_view_matrix;
uniform mat4 UNIFORM_projection_matrix;

void main()
{
	position = UNIFORM_model_view_matrix * vertex_position;
	gl_Position = UNIFORM_projection_matrix * position;
}