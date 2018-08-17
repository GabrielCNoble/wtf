#version 150

uniform sampler2D UNIFORM_texture_sampler0;

in vec3 color;
in vec2 tex_coords;
void main()
{
	gl_FragColor = texture2D(UNIFORM_texture_sampler0, tex_coords);
}
