attribute vec4 vertex_position;


varying int face_index;


void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * vertex_position;
    face_index = gl_VertexID / 3;

}
