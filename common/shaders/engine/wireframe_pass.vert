#version 400 compatibility

#include "engine/default_attribs.h"
#include "engine/default_uniforms.h"

void main()
{
	gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
}