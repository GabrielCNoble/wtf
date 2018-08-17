#version 400 compatibility  

#extension GL_EXT_gpu_shader4 : require



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
uniform unsigned int UNIFORM_material_flags;


in vec2 tex_coords;

void main()
{
	
	vec4 color;

	if(bool(UNIFORM_material_flags & MATERIAL_USE_DIFFUSE_TEXTURE))
	{
		color = texture2D(UNIFORM_texture_sampler0, tex_coords);
	}
	else
	{
		color = vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
	}
	
	gl_FragColor = color;

}





