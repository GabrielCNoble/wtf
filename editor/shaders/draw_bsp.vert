varying vec3 normal;
varying vec3 position;
varying vec2 uv;
void main()
{
	position = gl_ModelViewMatrix * gl_Vertex;	
	normal = gl_NormalMatrix * gl_Normal.xyz;
	uv = vec2(1.0, 0.0);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
