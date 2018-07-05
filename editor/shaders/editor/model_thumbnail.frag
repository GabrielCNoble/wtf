#version 400 compatibility

//uniform int UNIFORM_r_width;
//uniform int UNIFORM_r_height;
//uniform int UNIFORM_r_frame;


uniform sampler2D UNIFORM_texture_sampler0;
uniform sampler2D UNIFORM_texture_sampler1;
uniform sampler2D UNIFORM_texture_sampler2;
//uniform samplerCube UNIFORM_texture_cube_sampler0;
uniform unsigned int UNIFORM_material_flags;
uniform vec4 UNIFORM_active_camera_position;
//uniform usampler3D UNIFORM_cluster_texture;



#define MATERIAL_USE_DIFFUSE_TEXTURE (1 << 1)
#define MATERIAL_USE_NORMAL_TEXTURE (1 << 2)
#define MATERIAL_USE_HEIGHT_TEXTURE (1 << 3)
#define MATERIAL_USE_ROUGHNESS_TEXTURE (1 << 4)
#define MATERIAL_USE_METALNESS_TEXTURE (1 << 5)
	
#define MATERIAL_INVERT_NORMAL_X (1 << 6)
#define MATERIAL_INVERT_NORMAL_Y (1 << 7)
#define MATERIAL_USE_CUSTOM_SHADER (1 << 8)
#define MATERIAL_TRANSLUCENT (1 << 9)

in vec2 uv;
in vec3 eye_space_normal;
in vec3 eye_space_position;
in vec3 eye_space_tangent;

in vec3 camera_position;

void main()
{
	vec4 color;
	vec4 accum = vec4(0.0);
	vec3 light_vec;
	vec3 light_color;
	vec3 debug;
	vec3 normal;
	vec3 v;
	
	int i;
	unsigned int bm;
	ivec4 contents;
	int light_index;
	
	vec3 diffuse;
	vec3 specular;
	ivec3 cluster;
	
	float radius;
	float attenuation;
	float distance;
	float shininess;
	float energy;
	float shadow;
	
	float s;
	float d;
	
	vec3 eye_vec;
	vec3 half_vec;
	
	mat3 tbn;
	
	if(bool(UNIFORM_material_flags & MATERIAL_USE_DIFFUSE_TEXTURE))
	{
		color = texture2D(UNIFORM_texture_sampler0, uv);
	}
	else
	{
		color = vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
	}
	
	
	if(bool(UNIFORM_material_flags & MATERIAL_USE_NORMAL_TEXTURE))
	{
		tbn[0] = (eye_space_tangent);
		tbn[1] = (cross(eye_space_tangent, eye_space_normal));
		tbn[2] = (eye_space_normal);
		
		normal = (texture2D(UNIFORM_texture_sampler1, uv).rgb * 2.0 - 1.0);
		normal.x = mix(normal.x, -normal.x, clamp(float(bool(UNIFORM_material_flags & MATERIAL_INVERT_NORMAL_X)), 0.0, 1.0));
		normal.y = mix(normal.y, -normal.y, clamp(float(bool(UNIFORM_material_flags & MATERIAL_INVERT_NORMAL_Y)), 0.0, 1.0));
		normal = normalize(tbn * normal);	
	}
	else
	{
		normal = normalize(eye_space_normal);
	}
	
	//shininess = float(gl_FrontMaterial.shininess);
	shininess = 512.0;
	
	//eye_vec = normalize(UNIFORM_active_camera_position.xyz - world_space_position);
	eye_vec = normalize(eye_space_position);
	
	//light_vec = UNIFORM_active_camera_position.xyz - world_space_position;
	light_vec = -eye_space_position;
	light_color = vec3(1.0);
			
	distance = length(light_vec);
	
	energy = distance * 5.0;
	radius = 100.0;
	
			
			
	light_vec /= distance;
			//half_vec = normalize(eye_vec + light_vec);
	half_vec = normalize(eye_vec + light_vec);

			//attenuation = (1.0 / (distance)) * max(1.0 - (distance / radius), 0.0);
	attenuation = 1.0 / (distance);
	diffuse = ((color.rgb / 3.14159265) * max(dot(light_vec, normal), 0.0)) * attenuation * energy;
	specular = light_color * pow(max(dot(half_vec, normal), 0.0), shininess) * attenuation * energy * 1.5;
	accum += (vec4(diffuse * light_color, 1.0) + vec4(specular, 1.0));
	
	//gl_FragColor = vec4(eye_space_normal, 1.0);
	gl_FragColor = accum;

}


