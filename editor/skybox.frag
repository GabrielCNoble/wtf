uniform samplerCube texture_cube_sampler0;
varying vec3 coords;


void main()
{
	gl_FragColor = textureCube(texture_cube_sampler0, coords);
}
