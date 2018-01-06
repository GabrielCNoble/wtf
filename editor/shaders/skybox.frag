uniform samplerCube texture_cube_sampler0;
varying vec3 coords;


void main()
{
	gl_FragColor = textureCube(texture_cube_sampler0, coords);
	//gl_FragColor = vec4(abs(coords), 1.0);
}
