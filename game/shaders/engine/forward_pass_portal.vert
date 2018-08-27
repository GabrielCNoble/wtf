#version 400 compatibility  

#include "engine/default_clustered_forward.h"

uniform vec4 clip_plane;

void main()
{
	gl_Position = evaluate_vertex();
	gl_ClipDistance[0] = -dot(clip_plane.xyz, eye_space_position.xyz) - clip_plane.w;
}
