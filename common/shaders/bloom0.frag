uniform sampler2D UNIFORM_texture_sampler0;

varying vec2 tex_coord;

void main()
{
	gl_FragColor = clamp(texture2D(UNIFORM_texture_sampler0, tex_coord) - vec4(1.0, 1.0, 1.0, 0.0) * 1.0, vec4(0), vec4(1000.0));
}
