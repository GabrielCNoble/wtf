attribute vec4 vertex_position;

//varying vec3 pos;

uniform mat4 UNIFORM_model_view_projection_matrix;

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;

	//gl_Position = gl_ModelViewProjectionMatrix * vertex_position;
	//pos = vec3(gl_ModelViewMatrix * vertex_position);
	
}
