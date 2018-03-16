
varying vec3 color;
varying vec2 tex_coords;
void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	color = gl_Color.rgb;
	tex_coords.x = gl_Vertex.x * 0.5 + 0.5;
	tex_coords.y = gl_Vertex.y * 0.5 + 0.5;
}
