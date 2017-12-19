attribute vec4 vertex_position;
attribute vec4 vertex_normal;
attribute vec4 vertex_tangent;
attribute vec2 vertex_tex_coords;




void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vertex_position;
}