varying vec2 tex_coords;
varying vec4 color;

uniform sampler2D UNIFORM_texture_sampler0;



void main()
{
	gl_FragColor = texture2D(UNIFORM_texture_sampler0, tex_coords) * color;
}