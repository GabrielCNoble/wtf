#version 400 compatibility  

in vec4 vertex_position;
in vec4 vertex_tex_coords;


uniform mat4 UNIFORM_model_view_projection_matrix;

//out vec4 pos;

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
	//gl_Position = pos;
	//tex_coords = (pos.xy / pos.w) * 0.5 + 0.5;
}
