#version 150

in vec4 vertex_position;

out vec3 color;
out vec2 tex_coords;
void main()
{
	gl_Position = vertex_position;
	tex_coords.x = vertex_position.x * 0.5 + 0.5;
	tex_coords.y = vertex_position.y * 0.5 + 0.5;
}
