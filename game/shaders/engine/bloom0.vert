in vec4 vertex_position;


varying vec2 tex_coord;
void main()
{
	gl_Position = vertex_position;
	tex_coord = vertex_position.xy * 0.5 + vec2(0.5);
}
