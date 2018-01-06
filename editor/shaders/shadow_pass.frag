
varying vec3 pos;

void main()
{
	//gl_FragColor = vec4(0.5);
	
	gl_FragColor = vec4(length(pos));
}
