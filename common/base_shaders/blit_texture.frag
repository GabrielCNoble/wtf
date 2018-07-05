
uniform sampler2D UNIFORM_texture_sampler0;

varying vec3 color;
varying vec2 tex_coords;
void main()
{
	gl_FragColor = texture2D(UNIFORM_texture_sampler0, tex_coords);
}
