

uniform sampler2D UNIFORM_texture_sampler0;


varying vec2 tex_coords;
varying vec4 color;

void main()
{
	gl_FragColor = texture2D(UNIFORM_texture_sampler0, tex_coords) * color;
	//gl_FragColor = texelFetch(UNIFORM_texture_sampler0, ivec2(tex_coords), 0) * color;
	//gl_FragColor = vec4(tex_coords, 0.0, 1.0);
}
