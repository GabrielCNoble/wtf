#version 400 compatibility

in vec4 vertex_position;

uniform mat4 UNIFORM_model_view_projection_matrix;

void main()
{
    gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
}
