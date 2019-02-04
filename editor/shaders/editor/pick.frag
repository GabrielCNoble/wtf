

uniform float pick_type;
uniform float pick_index;

void main()
{
	gl_FragData[0].r = pick_type;
	gl_FragData[0].g = pick_index;
}
