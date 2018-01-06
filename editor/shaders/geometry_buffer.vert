attribute vec4 vertex_position;
attribute vec4 vertex_normal;
attribute vec4 vertex_tangent;
attribute vec2 vertex_tex_coords;


varying vec3 position;
varying vec3 normal;
varying vec2 tex_coords;


void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vertex_position;
	tex_coords = vertex_tex_coords;
	position = vec3(gl_ModelViewMatrix * vertex_position);
	//normal = vertex_normal.xyz;
	normal = gl_NormalMatrix * vertex_normal.xyz;
}
