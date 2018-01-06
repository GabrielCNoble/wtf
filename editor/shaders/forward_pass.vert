#version 130
in vec4 vertex_position;
in vec4 vertex_normal;
in vec4 vertex_tangent;
in vec2 vertex_tex_coords;



out vec2 uv;
out vec3 world_space_normal;
out vec3 world_space_position;
out vec3 eye_space_position;

void main()
{
	
	gl_Position = gl_ModelViewProjectionMatrix * vertex_position;
	uv = vertex_tex_coords;
	world_space_normal = vertex_normal.xyz;
	world_space_position = vertex_position.xyz;
	//normal = gl_NormalMatrix * vertex_normal.xyz;
	eye_space_position = vec3(gl_ModelViewMatrix * vertex_position);
	
}
