uniform sampler2D UNIFORM_texture_sampler0;
uniform int UNIFORM_r_width;
uniform int UNIFORM_r_height;
uniform int UNIFORM_r_bloom_radius;

varying vec2 tex_coord;

//#define BLOOM_RADIUS 16

void main()
{
	vec4 color = vec4(0);
	float delta_x; // = 1.0 / float(UNIFORM_r_width);
	float delta_y; // = 1.0 / float(UNIFORM_r_height);
	float weight;
	int i;
	color = texture2D(UNIFORM_texture_sampler0, tex_coord);
	
	int radius = int(UNIFORM_r_bloom_radius * 2);
	
	//int radius = 32;
	
	if(UNIFORM_r_width > 0)
	{
		delta_x = 1.0 / float(UNIFORM_r_width);
		for(i = 0; i < radius; i++)
		{
			weight = 1.0 - (float(i)/float(radius));
			color += texture2D(UNIFORM_texture_sampler0, tex_coord + vec2(delta_x * (float(i) + 0.5), 0.0)) * weight;
			color += texture2D(UNIFORM_texture_sampler0, tex_coord + vec2(-delta_x * (float(i) + 0.5), 0.0)) * weight;
		}
	}
	else
	{
		delta_y = 1.0 / float(UNIFORM_r_height);
		for(i = 0; i < radius; i++)
		{
			weight = 1.0 - (float(i)/float(radius));
			color += texture2D(UNIFORM_texture_sampler0, tex_coord + vec2(0.0, delta_y * (float(i) + 0.5))) * weight;
			color += texture2D(UNIFORM_texture_sampler0, tex_coord + vec2(0.0, -delta_y * (float(i) + 0.5))) * weight;
		}
	}
	
	gl_FragColor = (color / float(1 + radius * 2)) * 1.0;
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 0.5);
}
