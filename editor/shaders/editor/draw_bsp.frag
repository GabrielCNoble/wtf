

#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_uniform_buffer_object : enable


#define MAX_LIGHTS 128

struct light_params_fields
{
	//mat4 projection_matrix;
	//mat4 world_to_light_matrix;
	vec4 forward_axis;
	vec4 position;
	vec4 color;
	float radius;
	float energy;
	float align0;
	float align1;
};



layout(std140) uniform light_params_uniform_block
{
	light_params_fields light_params[MAX_LIGHTS];
};

uniform usampler3D cluster_texture;
uniform sampler2D texture_sampler0;
uniform sampler2D texture_sampler1;
uniform int texture_flags;
uniform int light_count;


#define USE_DIFFUSE_TEXTURE 1
#define USE_NORMAL_TEXTURE 2


#define CLUSTER_X_DIVS 16
#define CLUSTER_Y_DIVS 8
#define CLUSTER_Z_DIVS 16



varying vec2 uv;
varying vec3 normal;
varying vec3 position;


ivec3 get_cluster(float x_coord, float y_coord, float view_z, float znear, float zfar)
{
	ivec3 pos;
	pos.x = int(floor(x_coord / CLUSTER_X_DIVS));
	pos.y = int(floor(y_coord / CLUSTER_Y_DIVS));
	pos.z = int((log(-view_z / znear) / log(zfar / znear)) * CLUSTER_Z_DIVS);
		
	if(pos.z > CLUSTER_Z_DIVS) pos.z = CLUSTER_Z_DIVS;
	else if(pos.z < 0) pos.z = 0;
	
	return pos; 
}


void main()
{
	
	vec4 color;
	vec4 accum = vec4(0);
	vec3 light_vec;
	vec3 light_color;
	
	int i;
	
	vec3 diffuse;
	vec3 specular;
	
	float radius;
	float attenuation;
	float distance;
	float energy;
	
	vec3 eye_vec;
	vec3 half_vec;
	
	ivec3 cluster = get_cluster(gl_FragCoord.x, gl_FragCoord.y, position.z, 0.01, 256.0);
	
	if(bool(texture_flags & USE_DIFFUSE_TEXTURE) == true)
	{
		color = texture2D(texture_sampler0, uv);
	}
	else
	{
		color = vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
	}
	
	/*if(cluster.z % 2 == 0)
	{
		accum.b = 0.1;
	}	
	if(cluster.x % 2 == 0)
	{
		accum.r = 0.1; 
	}
	if(cluster.y % 2 == 0)
	{
		accum.g = 0.1;
	}*/
	
	for(i = 0; i < light_count; i++)
	{
		
		light_vec = light_params[i].position.xyz - position;
		light_color = light_params[i].color.rgb;
		
		distance = length(light_vec);
		energy = light_params[i].energy;
		radius = light_params[i].radius;
		
		eye_vec = normalize(-position);
		light_vec /= distance;
		half_vec = normalize(eye_vec + light_vec);
		
		attenuation = (1.0 / (distance)) * max(1.0 - (distance / radius), 0.0);
		diffuse = ((color.rgb / 3.14159265) * max(dot(light_vec, normal), 0.0)) * attenuation * energy;
		specular = light_color * pow(max(dot(half_vec, normal), 0.0), 32.0) * attenuation * energy * 0.1;
		accum += vec4(diffuse * light_color, 1.0) + vec4(specular, 1.0);
	}
	
	gl_FragColor = accum;
	
	
	//gl_FragColor = color;
}






