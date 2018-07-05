#version 400 compatibility  

#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_uniform_buffer_object : enable

#define LIGHT_CACHED 1
#define LIGHT_MOVED (1 << 1)
#define LIGHT_NEEDS_REUPLOAD (1 << 2)
#define LIGHT_GENERATE_SHADOWS (1 << 3)
#define LIGHT_INVALID (1 << 4)

#define MATERIAL_USE_DIFFUSE_TEXTURE (1<<1)
#define MATERIAL_USE_NORMAL_TEXTURE (1<<2)
#define MATERIAL_USE_HEIGHT_TEXTURE (1<<3)
#define MATERIAL_USE_ROUGHNESS_TEXTURE (1<<4)
#define MATERIAL_USE_METALNESS_TEXTURE (1<<5)
	
#define MATERIAL_INVERT_NORMAL_X (1<<6)
#define MATERIAL_INVERT_NORMAL_Y (1<<7)
#define MATERIAL_USE_CUSTOM_SHADER (1<<8)
#define MATERIAL_TRANSLUCENT (1<<9)

uniform sampler2D UNIFORM_texture_sampler0;
uniform sampler2D UNIFORM_texture_sampler1;
uniform sampler2D UNIFORM_texture_sampler2;
uniform samplerCube UNIFORM_texture_cube_sampler0;
uniform unsigned int UNIFORM_material_flags;
uniform vec4 UNIFORM_active_camera_position;
uniform usampler3D UNIFORM_cluster_texture;

in vec3 eye_space_position;
in vec3 eye_space_normal;
in vec3 eye_space_tangent;
in vec2 tex_coords;


void main()
{	
	mat3 tbn;
	vec3 normal;
	
	if(bool(UNIFORM_material_flags & MATERIAL_USE_DIFFUSE_TEXTURE))
	{
		gl_FragData[0] = texture2D(UNIFORM_texture_sampler0, tex_coords);
	}
	else
	{
		gl_FragData[0] = vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
	}
	
	if(bool(UNIFORM_material_flags & MATERIAL_USE_NORMAL_TEXTURE))
	{
		tbn[0] = (eye_space_tangent);
		tbn[1] = (cross(eye_space_tangent, eye_space_normal.xyz));
		tbn[2] = (eye_space_normal.xyz);
		
		normal = (texture2D(UNIFORM_texture_sampler1, tex_coords).rgb * 2.0 - 1.0);
		
		normal.x = mix(normal.x, -normal.x, clamp(float(bool(UNIFORM_material_flags & MATERIAL_INVERT_NORMAL_X)), 0.0, 1.0));
		normal.y = mix(normal.y, -normal.y, clamp(float(bool(UNIFORM_material_flags & MATERIAL_INVERT_NORMAL_Y)), 0.0, 1.0));

		normal = normalize(tbn * normal);
	}
	else
	{
		normal = normalize(eye_space_normal.xyz);
	}
	
	gl_FragData[1] = vec4(normal, eye_space_position.z);

}





