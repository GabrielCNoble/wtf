#version 400 compatibility

#include "engine/default_clustered_forward.h"

void main()
{
	//gl_FragColor = evaluate_pixel();
	gl_FragData[0] = evaluate_pixel();
	//gl_FragData[1] = vec4(eye_space_pixel_normal, 0.0);
}






