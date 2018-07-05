#version 120

in vec3 vertex_position;


uniform mat4 UNIFORM_model_view_projection_matrix; 

void main()
{
	//gl_Position = UNIFORM_model_view_projection_matrix * vec4(vertex_position, 1.0);
	gl_Position = UNIFORM_model_view_projection_matrix * vec4(1.0);
}
