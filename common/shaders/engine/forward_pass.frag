#version 400 compatibility

#include "engine/default_clustered_forward.h"

void main()
{
	gl_FragData[0] = evaluate_pixel();
	//gl_FragData[0] = vec4(pixel_normal(), 0.0);
	//gl_FragData[0] = vec4(eye_space_pixel_normal, 0.0);
}






