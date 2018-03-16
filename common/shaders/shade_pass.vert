
varying vec2 tex_coords;

void main()
{
	gl_Position = gl_Vertex;
	tex_coords = vec2(gl_Vertex.x * 0.5 + 0.5, gl_Vertex.y * 0.5 + 0.5);
}
