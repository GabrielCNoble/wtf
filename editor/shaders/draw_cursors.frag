
uniform sampler2D UNIFORM_texture_sampler0;

varying vec3 color;
varying vec2 tex_coords;
void main()
{
	//gl_FragColor = vec4(color, 1.0);	
	//gl_FragColor = vec4(1.0, 1.0, 0.0 ,0.0);
	//gl_FragColor = gl_FrontMaterial.diffuse;
	gl_FragColor = texture2D(UNIFORM_texture_sampler0, tex_coords);
}
