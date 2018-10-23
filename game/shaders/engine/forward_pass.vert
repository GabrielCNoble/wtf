#version 400 compatibility

#include "engine/default_clustered_forward.h"

void main()
{
	gl_Position = evaluate_vertex();
}
