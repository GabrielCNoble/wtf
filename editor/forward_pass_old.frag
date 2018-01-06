uniform int r_width;
uniform int r_height;
uniform usampler3D cluster_texture;

#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_uniform_buffer_object : enable


#define LIGHT_CACHE_SIZE 32


#define LIGHT_CACHED 1
#define LIGHT_MOVED (1 << 1)
#define LIGHT_NEEDS_REUPLOAD (1 << 2)
#define LIGHT_GENERATE_SHADOWS (1 << 3)
#define LIGHT_INVALID (1 << 4)


struct light_params_fields
{
	//mat4 projection_matrix;
	//mat4 world_to_light_matrix;
	vec4 forward_axis;
	vec4 position;
	vec4 color_radius;
	float energy;
	int bm_flags;
	int x_y;
	int align0;
};



layout(std140) uniform light_params_uniform_block
{
	light_params_fields light_params[LIGHT_CACHE_SIZE];
};

uniform sampler2D texture_sampler0;
uniform sampler2D texture_sampler1;
uniform samplerCube texture_cube_sampler0;

//uniform samplerCube shadow_sampler;
uniform mat4 camera_to_light_matrix;
uniform int texture_flags;
//uniform int light_count;
uniform int r_frame;




#define USE_DIFFUSE_TEXTURE 1
#define USE_NORMAL_TEXTURE 2


#define CLUSTERS_PER_ROW 16
#define CLUSTER_ROWS 8
#define CLUSTER_LAYERS 16



varying vec2 uv;
varying vec3 normal;
varying vec3 position;

int cluster_index(float x, float y, float view_z, float znear, float zfar, float width, float height, out ivec3 debug)
{
	int cluster;
	int row;
	int layer;
	
	cluster = min(int((x / width) * float(CLUSTERS_PER_ROW)), CLUSTERS_PER_ROW);
	row = min(int((y / height) * float(CLUSTER_ROWS)), CLUSTER_ROWS);
	layer = int((log(-view_z / znear) / log(zfar / znear)) * float(CLUSTER_LAYERS));
	
	layer = max(min(layer, CLUSTER_LAYERS), 0);
	
	debug.x = cluster;
	debug.y = row;
	debug.z = layer;
	
	return cluster + layer * CLUSTERS_PER_ROW * CLUSTER_ROWS + row * CLUSTERS_PER_ROW;	
}


void main()
{
	
	vec4 color;
	vec4 accum = vec4(0.1);
	vec3 light_vec;
	vec3 light_color;
	vec3 v;
	
	int i;
	unsigned int bm;
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
	
/*	ivec3 cluster = get_cluster(gl_FragCoord.x, gl_FragCoord.y, position.z, 0.01, 256.0);*/
	
	if(bool(texture_flags & USE_DIFFUSE_TEXTURE) == true)
	{
		color = texture2D(texture_sampler0, uv);
	}
	else
	{
		color = vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
	}

	
	bm = cluster_index(gl_FragCoord.x, gl_FragCoord.y, position.z, 4.0, 500.0, float(r_width), float(r_height), cluster);
	
	bm = texelFetch(cluster_texture, ivec3(0, 0, 0), 0).r;
	//bm = 0;
	
	/*if(cluster.x % 2 == 0)
	{
		accum.x = 0.5;
	}*/
	
	if(bm == 0)
	{
		accum.x += 0.1;
	}
	
	
	for(i = 0; i < 1; i++)
	{
		
		//if((bm & 1) == 1)
		{
			light_index = i;
			
			light_vec = light_params[light_index].position.xyz - position;
			light_color = light_params[light_index].color_radius.rgb;
			
			distance = length(light_vec);
			energy = light_params[light_index].energy;
			radius = light_params[light_index].color_radius.a;
			
			eye_vec = normalize(-position);
			light_vec /= distance;
			half_vec = normalize(eye_vec + light_vec);
			
			//shadow = 1.0;
			
		
			
			/*if(light_params[light_index].bm_flags == 1)
			{
				v = vec3(camera_to_light_matrix * vec4(light_vec, 0.0));
		
				s = textureCube(texture_cube_sampler0, -v).r;
				
				if(s + 0.05 < distance)
				{
					shadow = 0.0;
				}
			}*/
			
			/* TODO: pbr */
			attenuation = (1.0 / (distance)) * max(1.0 - (distance / radius), 0.0);
			diffuse = ((color.rgb / 3.14159265) * max(dot(light_vec, normal), 0.0)) * attenuation * energy;
			specular = light_color * pow(max(dot(half_vec, normal), 0.0), 32.0) * attenuation * energy * 0.1;
			accum += (vec4(diffuse * light_color, 1.0) + vec4(specular, 1.0)) ;
		}
		
		bm >>= 1;
		
	}
	
	
	gl_FragColor = accum;
	


}





