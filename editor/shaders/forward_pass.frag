#version 400 compatibility

#extension GL_EXT_gpu_shader4 : require
//#extension GL_ARB_uniform_buffer_object : require
//#extension GL_EXT_uniform_buffer_object : enable


uniform int r_width;
uniform int r_height;
uniform int r_frame;
uniform usampler3D cluster_texture;




#define LIGHT_CACHE_SIZE 32

#define SHADOW_MAP_RESOLUTION 512		/* fixed size shadow maps for all lights... */
#define SHARED_SHADOW_MAP_HEIGHT 8192
#define SHARED_SHADOW_MAP_WIDTH 8192

#define SHADOW_MAP_WIDTH 0.0625
#define SHADOW_MAP_HEIGHT 0.0833333333333333


#define LIGHT_CACHED 1
#define LIGHT_MOVED (1 << 1)
#define LIGHT_NEEDS_REUPLOAD (1 << 2)
#define LIGHT_GENERATE_SHADOWS (1 << 3)
#define LIGHT_INVALID (1 << 4)


#define vec4_t vec4


struct light_params_fields
{
	vec4_t forward_axis;
	vec4_t position_radius;
	vec4_t color_energy;
	int bm_flags;
	unsigned int x_y;
	int align0;
	int align1;
};



layout(std140) uniform light_params_uniform_block
{
	light_params_fields light_params[LIGHT_CACHE_SIZE];
};

uniform sampler2D texture_sampler0;
uniform sampler2D texture_sampler1;
uniform sampler2D texture_sampler2;
uniform samplerCube texture_cube_sampler0;
uniform mat4 camera_to_light_matrix;
uniform int texture_flags;
uniform vec4 active_camera_position;




#define USE_DIFFUSE_TEXTURE 1
#define USE_NORMAL_TEXTURE 2


#define CLUSTERS_PER_ROW 32
#define CLUSTER_ROWS 16
#define CLUSTER_LAYERS 24



in vec2 uv;
in vec3 world_space_normal;
in vec3 world_space_position;
in vec3 eye_space_position;
in vec3 camera_position;




float sample_cube_map(vec3 frag_pos, int light_index, out vec3 debug_color)
{
	
	vec3 light_vec = frag_pos.xyz - light_params[light_index].position_radius.xyz;
									
	float shadow0;
	float u0;
	float v0;
	float ox0;
	float oy0;

	float x = float(light_params[light_index].x_y & uint(0x0000ffff)) / float(SHARED_SHADOW_MAP_WIDTH);
	float y = float((light_params[light_index].x_y >> 16) & uint(0x0000ffff)) / float(SHARED_SHADOW_MAP_HEIGHT);
	float w = float(SHADOW_MAP_RESOLUTION) / float(SHARED_SHADOW_MAP_WIDTH);
	float h = float(SHADOW_MAP_RESOLUTION) / float(SHARED_SHADOW_MAP_HEIGHT);
	//float w = 0.0625;
	//float h = 0.0625;
	
	float dist;
	float largest;
	
	vec3 a_light_vec = abs(light_vec);
	largest = max(a_light_vec.x, max(a_light_vec.y, a_light_vec.z));
	
	dist = length(light_vec);
	light_vec.z = -light_vec.z;
	
	
		
	if(largest == a_light_vec.x)
	{	
		u0 = -light_vec.y;
		if(light_vec.x < 0.0)
		{
			/* -X */
			v0 = -light_vec.z;
			oy0 = h;
			//debug_color = vec3(1.0, 0.0, 0.0);
		}
		else
		{
			/* +X */
			v0 = light_vec.z;	
			oy0 = 0.0;
			//debug_color = vec3(1.0, 0.0, 0.0);
		}
		ox0 = 0.0;
	}
	else if(largest == a_light_vec.y)
	{	
		v0 = light_vec.x;
		if(light_vec.y < 0.0)
		{
			/* -Y */
			u0 = light_vec.z;
			oy0 = h;
			//debug_color = vec3(0.0, 1.0, 0.0);
		}
		else
		{
			/* +Y */
			u0 = -light_vec.z;	
			oy0 = 0.0;
			//debug_color = vec3(0.0, 1.0, 0.0);
		}
		ox0 = w;	
	}
	else
	{	
		u0 = -light_vec.y;
		if(light_vec.z > 0.0)
		{
			/* -Z */
			v0 = -light_vec.x;
			oy0 = h;			
			//debug_color = vec3(0.0, 0.0, 1.0);
			
			
		}
		else
		{
			/* +Z */
			v0 = light_vec.x;	
			oy0 = 0.0;
			//debug_color = vec3(0.0, 0.0, 1.0);
		}
		ox0 = 2.0 * w;
	}
	
	
	
	u0 = 0.5 * (u0 / largest) + 0.5;
	v0 = 0.5 * (v0 / largest) + 0.5;
	
	v0 = x + ox0 + w * v0;
	u0 = y + oy0 + h * u0;

	
	shadow0 = texelFetch(texture_sampler2, ivec2(SHARED_SHADOW_MAP_WIDTH * v0, SHARED_SHADOW_MAP_HEIGHT * u0), 0).r;
	
	//if(dist > shadow0 + 0.3)
	//if(fz > shadow0 + 0.00001)
	//{
	//	return 0.0;
	//}
	//return 1.0;
	
	return float(dist < shadow0 + 0.3);
}





int cluster_index(float x, float y, float view_z, float znear, float zfar, float width, float height, out ivec3 debug)
{
	int cluster;
	int row;
	int layer;

	//3.6989700043360188047862611052755
	
	cluster = min(int((x / width) * float(CLUSTERS_PER_ROW)), CLUSTERS_PER_ROW);
	row = min(int((y / height) * float(CLUSTER_ROWS)), CLUSTER_ROWS);
	layer = int((log(-view_z / znear) / log(zfar / znear)) * float(CLUSTER_LAYERS));

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
	vec4 accum = vec4(0.0);
	vec3 light_vec;
	vec3 light_color;
	vec3 debug;
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
	float energy;
	float shadow;
	
	float s;
	float d;
	
	vec3 eye_vec;
	vec3 half_vec;
	
	/*if(bool(texture_flags & USE_DIFFUSE_TEXTURE) == false)
	{
		color = texture2D(texture_sampler0, uv);
	}
	else*/
	{
		color = vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
	}

	
	cluster_index(gl_FragCoord.x, gl_FragCoord.y, eye_space_position.z, 1.0, 500.0, r_width, r_height, cluster);
	bm = texelFetch(cluster_texture, cluster, 0).r;
/*	contents = ivec4(texelFetch(cluster_texture, cluster, 0));
	
	bm = uint(contents.r);
	if(contents.g != r_frame)
		discard;*/

	//eye_vec = normalize(-position);
	eye_vec = normalize(active_camera_position.xyz - world_space_position);
	for(i = 0; i < LIGHT_CACHE_SIZE; i++)
	{
		
		if((bm & uint(1)) == uint(1))
		//if(bm != uint(0))
		{
			
			
			light_vec = light_params[i].position_radius.xyz - world_space_position;
			light_color = light_params[i].color_energy.rgb;
			
			distance = length(light_vec);
			///distance = dot(light_vec, light_vec);
			energy = light_params[i].color_energy.a;
			radius = light_params[i].position_radius.w;
			
			shadow = 1.0;
			
			if((light_params[i].bm_flags & LIGHT_GENERATE_SHADOWS) == LIGHT_GENERATE_SHADOWS)
			{
				shadow = sample_cube_map(world_space_position, i, debug);
			}
		
			/*shadow = 1.0;*/
			
			
			
			light_vec /= distance;
			//half_vec = normalize(eye_vec + light_vec);
			half_vec = normalize(eye_vec + light_vec);

			attenuation = (1.0 / (distance)) * max(1.0 - (distance / radius), 0.0);
			diffuse = ((color.rgb / 3.14159265) * max(dot(light_vec, world_space_normal), 0.0)) * attenuation * energy;
			specular = light_color * pow(max(dot(half_vec, world_space_normal), 0.0), 32.0) * attenuation * energy * 0.1;
			accum += (vec4(diffuse * light_color, 1.0) + vec4(specular, 1.0)) * shadow;
			//accum = vec4(color.rgb, 0.0);
			//accum = vec4(1.0);
			
		}
		
		bm >>= 1;
	}

	
	
	gl_FragColor = accum;
	//gl_FragColor = vec4(1.0);
	


}






