#version 400 compatibility

in vec4 vertex_position;


//flat varying int face_index;

uniform mat4 UNIFORM_model_view_projection_matrix;

void main()
{
    gl_Position = UNIFORM_model_view_projection_matrix * vertex_position;
    //face_index = gl_VertexID / 3;

}
