#version 400 compatibility

in vec4 vertex_position;
uniform int test_uniform;

//flat varying int face_index;

uniform mat4 UNIFORM_model_view_projection_matrix;

out int r;

void main()
{
    gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;

    r = test_uniform;
    //face_index = gl_VertexID / 3;

}
