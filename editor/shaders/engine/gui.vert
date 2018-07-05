#version 400 compatibility

#include "engine/default_attribs.h"
#include "engine/default_uniforms.h"

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vec4(vertex_position.xy, 0.0, 1.0);	
}
