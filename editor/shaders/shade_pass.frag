uniform int r_width;
uniform int r_height;
uniform int r_frame;

uniform usampler3D cluster_texture;

uniform sampler2D texture_sampler0;
uniform sampler2D texture_sampler1;
uniform sampler2D texture_sampler2;
uniform samplerCube texture_cube_sampler0;
uniform mat4 camera_to_light_matrix;
uniform int light_index;


#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_uniform_buffer_object : enable
#extension GL_ARB_arrays_of_arrays : enable


#define LIGHT_CACHE_SIZE 32

#define CLUSTERS_PER_ROW 16
#define CLUSTER_ROWS 8
#define CLUSTER_LAYERS 16

#define SHADOW_MAP_RESOLUTION 512

#define LIGHT_CACHED 1
#define LIGHT_MOVED (1 << 1)
#define LIGHT_NEEDS_REUPLOAD (1 << 2)
#define LIGHT_GENERATE_SHADOWS (1 << 3)
#define LIGHT_INVALID (1 << 4)

varying vec2 tex_coords;

struct light_params_fields
{
	vec4 forward_axis;
	vec4 position;
	vec4 color_radius;
	float energy;
	int bm_flags;
	int x_y;
	int align0;
};

/*struct cluster_t
{
	unsigned int light_indexes_bm;
	unsigned int time_stamp;
	int align1;
	int align2;
};*/



layout(std140) uniform light_params_uniform_block
{
	light_params_fields light_params[LIGHT_CACHE_SIZE];
};

/*layout(std140) uniform cluster_uniform_block
{
	cluster_t clusters[CLUSTERS_PER_ROW * CLUSTER_ROWS * CLUSTER_LAYERS];
};*/

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
	
	vec4 albedo;
	vec3 normal;
	vec4 position;
	ivec2 dims;
	ivec3 debug;
	
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
	vec3 diffuse;
	vec3 specular;
	vec3 v;
	vec3 light_color;
	vec4 accum = vec4(0.1);
	
	int cluster;
	
	int i;
	unsigned int bm;
		
	dims = textureSize(texture_sampler0, 0);
	

	
	albedo = texture2D(texture_sampler0, tex_coords);
	normal = normalize(texture2D(texture_sampler1, tex_coords).xyz);
	depth = texture2D(texture_sampler2, tex_coords).r;
	
	position.x = (gl_FragCoord.x / float(dims.x)) * 2.0 - 1.0;
	position.y = (gl_FragCoord.y / float(dims.y)) * 2.0 - 1.0;
	position.z = depth * 2.0 - 1.0;
	position.w = 1.0;
	position = gl_ProjectionMatrixInverse * position;
	position /= position.w;
	
	eye_vec = normalize(-position.xyz);

		
		
	cluster = cluster_index(gl_FragCoord.x, gl_FragCoord.y, position.z, 4.0, 500.0, float(r_width), float(r_height), debug);
	
	bm = texelFetch(cluster_texture, debug, 0).r;
	
	/*if(debug.x % 2 == 0)
	{
		accum.x = 0.5;
	}*/
	
	
	//bm = clusters[cluster].light_indexes_bm;	
	
	for(i = 0; i < LIGHT_CACHE_SIZE; i++)
	{
		
		if((bm & 1) == 1)
		{
			light_vec = light_params[i].position.xyz - position.xyz;
			light_color = light_params[i].color_radius.rgb;
					
			shadow = 1.0;
				
			distance = length(light_vec);
			energy = light_params[i].energy;
			radius = light_params[i].color_radius.a;			
			
			light_vec /= distance;
			half_vec = normalize(eye_vec + light_vec);
			
			attenuation = (1.0 / (distance)) * max(1.0 - (distance / radius), 0.0);
			diffuse = ((albedo.rgb / 3.14159265) * max(dot(light_vec, normal), 0.0)) * attenuation * energy;
			specular = light_color * pow(max(dot(half_vec, normal), 0.0), 32.0) * attenuation * energy * 0.1;
			accum += (vec4(diffuse * light_color, 1.0) + vec4(specular, 1.0));
		}
		
		bm = bm >> 1;
		
	}


	gl_FragColor = accum;
	
	
	

}








