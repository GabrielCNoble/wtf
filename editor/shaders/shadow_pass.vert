attribute vec4 vertex_position;

varying vec3 pos;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vertex_position;
	pos = vec3(gl_ModelViewMatrix * vertex_position);
	
}
