#version 400 compatibility

uniform float pick_type;
uniform float pick_index;

void main()
{
	gl_FragColor.r = pick_type;
	gl_FragColor.g = pick_index;
}
