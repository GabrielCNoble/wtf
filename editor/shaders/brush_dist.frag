varying vec4 position;

void main()
{
	gl_FragColor = vec4(abs(position.z));
}