#version 400 compatibility
#extension GL_EXT_gpu_shader4 : require


#define LIGHT_CACHE_SIZE 32

#define SHADOW_MAP_RESOLUTION 512		
#define SHARED_SHADOW_MAP_HEIGHT 8192
#define SHARED_SHADOW_MAP_WIDTH 8192

#define SHADOW_MAP_WIDTH 0.0625
#define SHADOW_MAP_HEIGHT 0.0833333333333333


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





struct light_params_fields
{
	vec4 forward_axis;
	vec4 position_radius;
	vec4 color_energy;
	int bm_flags;
	unsigned int x_y;
	int align0;
	int align1;
};



layout(std140) uniform light_params_uniform_block
{
	light_params_fields light_params[LIGHT_CACHE_SIZE];
};

uniform int UNIFORM_r_width;
uniform int UNIFORM_r_height;
uniform int UNIFORM_r_near;
uniform int UNIFORM_r_far;

uniform int UNIFORM_r_frame;


uniform sampler2D UNIFORM_texture_sampler0;
uniform sampler2D UNIFORM_texture_sampler1;
uniform sampler2D UNIFORM_texture_sampler2;
uniform samplerCube UNIFORM_texture_cube_sampler0;
uniform unsigned int UNIFORM_material_flags;
uniform vec4 UNIFORM_active_camera_position;
uniform usampler3D UNIFORM_cluster_texture;



#define CLUSTERS_PER_ROW 32
#define CLUSTER_ROWS 16
#define CLUSTER_LAYERS 24



int cluster_index(float x, float y, float view_z, float znear, float zfar, float width, float height, out ivec3 debug)
{
	int cluster;
	int row;
	int layer;

	//3.6989700043360188047862611052755
	
	cluster = min(int((x / width) * float(CLUSTERS_PER_ROW)), CLUSTERS_PER_ROW);
	row = min(int((y / height) * float(CLUSTER_ROWS)), CLUSTER_ROWS);
	layer = int((log(view_z / znear) / log(zfar / znear)) * float(CLUSTER_LAYERS));

	//layer = int((log(-view_z / znear) / 3.6989700043360188047862611052755) * float(CLUSTER_LAYERS));
	
	layer = max(min(layer, CLUSTER_LAYERS), 0);
	
	debug.x = cluster;
	debug.y = row;
	debug.z = layer;
	
	return cluster + layer * CLUSTERS_PER_ROW * CLUSTER_ROWS + row * CLUSTERS_PER_ROW;	
}

void main()
{
	
	vec4 color;
	vec4 eye_space_normal;
	vec4 eye_space_position;
	ivec2 dims;
	ivec3 cluster;
	
	vec3 light_position;
	vec3 light_vec;
	vec3 eye_vec;
	vec3 half_vec;
	
	float distance;
	float energy;
	float radius;
	float attenuation;
	float s;
	float d;
	float shadow;
	float depth;
	float shininess;
	
	float ratio;
	
	vec3 diffuse;
	vec3 specular;
	vec3 v;
	vec3 light_color;
	vec4 accum = vec4(0.0);
	vec2 uv;
		
	int i;
	unsigned int bm;
	
	
	uv = vec2(gl_FragCoord.x / float(UNIFORM_r_width), gl_FragCoord.y / float(UNIFORM_r_height));
	color = texture2D(UNIFORM_texture_sampler0, uv);
	eye_space_normal = texture2D(UNIFORM_texture_sampler1, uv);
	
	
	ratio = float(UNIFORM_r_width) / float(UNIFORM_r_height);
	
	eye_space_position.x = (-uv.x * 2.0 + 1.0) * ratio;
	eye_space_position.y = (-uv.y * 2.0 + 1.0);
	eye_space_position.z = eye_space_normal.w;
	eye_space_position.w = 1.0;
		
	eye_space_position.x *= eye_space_position.z;
	eye_space_position.y *= eye_space_position.z;
	
	
	eye_space_normal.xyz = normalize(eye_space_normal.xyz);
	
			
	shininess = 512.0;
	
	/* fucking stupid GL drivers... */
	cluster_index(gl_FragCoord.x, gl_FragCoord.y, -eye_space_position.z, 1.0, 500.0, float(UNIFORM_r_width), float(UNIFORM_r_height), cluster);
	bm = texelFetch(UNIFORM_cluster_texture, cluster, 0).r;
	
	eye_vec = normalize(-eye_space_position.xyz);
	
	
	
	for(i = 0; i < LIGHT_CACHE_SIZE; i++)
	{
		
		if((bm & uint(1)) == uint(1))
		{
			light_vec = light_params[i].position_radius.xyz - eye_space_position.xyz;
			light_color = light_params[i].color_energy.rgb;
			
			distance = length(light_vec);
			
			energy = light_params[i].color_energy.a;
			radius = light_params[i].position_radius.w;
			
			light_vec /= distance;
			half_vec = normalize(eye_vec + light_vec);

			attenuation = (1.0 / (distance)) * max(1.0 - (distance / radius), 0.0);
			diffuse = ((color.rgb / 3.14159265) * max(dot(light_vec, eye_space_normal.xyz), 0.0)) * attenuation * energy;
			specular = light_color * pow(max(dot(half_vec, eye_space_normal.xyz), 0.0), shininess) * attenuation * energy * 1.5;
			//specular = vec3(abs(normal));
			
			accum += (vec4(diffuse * light_color, 1.0) + vec4(specular, 1.0));
		}
		
		bm >>= 1;
	}

	gl_FragColor = accum;
	//gl_FragColor = vec4(eye_space_position.xyz, 1.0);
}








