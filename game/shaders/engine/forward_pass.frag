#version 400 compatibility  

#include "engine/default_clustered_forward.h"

void main()
{
	gl_FragColor = evaluate_pixel();
	//gl_FragColor = vec4(tex_coords.x, tex_coords.y, 0.0, 0.0);
}






