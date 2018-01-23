attribute vec4 vertex_position;

varying vec4 position;

void main()
{
	position = gl_ModelViewMatrix * vertex_position;
	gl_Position = gl_ProjectionMatrix * position;
}