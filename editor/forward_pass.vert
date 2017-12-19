attribute vec4 vertex_position;
attribute vec4 vertex_normal;
attribute vec4 vertex_tangent;
attribute vec2 vertex_tex_coords;



varying vec2 uv;
varying vec3 normal;
varying vec3 position;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vertex_position;
	uv = vertex_tex_coords;
	normal = gl_NormalMatrix * vertex_normal.xyz;
	position = vec3(gl_ModelViewMatrix * vertex_position);
}
