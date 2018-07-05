#version 400 compatibility


//uniform int pick_type;
uniform float pick_index;

void main()
{
	gl_FragColor.r = pick_index;
}
