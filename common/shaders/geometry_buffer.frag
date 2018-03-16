#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_uniform_buffer_object : enable

#define LIGHT_CACHED 1
#define LIGHT_MOVED (1 << 1)
#define LIGHT_NEEDS_REUPLOAD (1 << 2)
#define LIGHT_GENERATE_SHADOWS (1 << 3)
#define LIGHT_INVALID (1 << 4)

#define USE_DIFFUSE_TEXTURE 1
#define USE_NORMAL_TEXTURE (1 << 1)

uniform sampler2D texture_sampler0;
uniform sampler2D texture_sampler1;
uniform int texture_flags;

varying vec3 position;
varying vec3 normal;
varying vec2 tex_coords;


void main()
{
	//gl_FragData[0] = vec4(normal, 1.0);
	
	if(texture_flags & USE_DIFFUSE_TEXTURE)
		gl_FragData[0] = texture2D(texture_sampler0, tex_coords);
	else	
		gl_FragData[0] = gl_FrontMaterial.diffuse;
		
		
		
	gl_FragData[1] = vec4(normal, 1.0);
	//gl_FragData[1] = vec4(1.0);
}
