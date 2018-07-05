uniform sampler2D UNIFORM_texture_sampler0;

varying vec2 tex_coord;

void main()
{
	//gl_FragColor = clamp(texture2D(UNIFORM_texture_sampler0, tex_coord) * 10.0 - vec4(1.0, 1.0, 1.0, 0.0) * 0.0, vec4(0.0), vec4(10000.0));
	gl_FragColor = texture2D(UNIFORM_texture_sampler0, tex_coord) * 5.0;
	//gl_FragColor = vec4(10.0);
}
