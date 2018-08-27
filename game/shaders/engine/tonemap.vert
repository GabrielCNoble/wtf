varying vec2 tex_coord;
void main()
{
	gl_Position = gl_Vertex;
	tex_coord = gl_Vertex.xy * 0.5 + vec2(0.5);
}
