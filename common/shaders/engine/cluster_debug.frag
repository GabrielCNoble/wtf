#version 400 compatibility

#include "engine/default_clustered_forward.h"

vec3 cluster_colors[] =
{
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.2, 0.9),
    vec3(0.0, 0.4, 0.6),
    vec3(0.0, 0.8, 0.2),
    vec3(0.0, 1.0, 0.0),
    vec3(0.2, 0.8, 0.0),
    vec3(0.0, 0.2, 0.8),

};

void main()
{
	float depth_sample;
	vec4 cluster_color = vec4(0.0, 0.0, 0.0, 0.0);
	ivec3 cluster_index;
	unsigned int light_bitmask;
	int i;
	int light_count = 0;
	vec2 frag_coord = vec2(gl_FragCoord.x / float(UNIFORM_r_width), gl_FragCoord.y / float(UNIFORM_r_height));

	depth_sample = texture2D(UNIFORM_texture_sampler0, frag_coord).r * 2.0 - 1.0;

	//zlin=(2.0*zNear*zFar)/(zFar+zNear-depthSample*(zFar-zNear));

	//depth_sample = depth_sample * (UNIFORM_r_far - UNIFORM_r_near) + UNIFORM_r_near;

	depth_sample = (2.0 * UNIFORM_r_near * UNIFORM_r_far) / (UNIFORM_r_far + UNIFORM_r_near - depth_sample * (UNIFORM_r_far - UNIFORM_r_near));

	light_bitmask = cluster_lights(gl_FragCoord.x, gl_FragCoord.y, depth_sample);

    for(i = 0; i < LIGHT_UNIFORM_BUFFER_SIZE; i++)
    {
        if((light_bitmask & 1) == 1)
        {
            light_count++;
        }

        light_bitmask >>= 1;
    }


    if(light_count > 0 && light_count < 4)
    {
        cluster_color = vec4(0.0, 0.0, 1.0, 0.8);
    }
    else if(light_count >= 4 && light_count < 8)
    {
        cluster_color = vec4(0.0, 0.8, 0.6, 0.8);
    }
    else if(light_count >= 8 && light_count < 16)
    {
        cluster_color = vec4(0.0, 1.0, 0.0, 0.8);
    }
    else if(light_count >= 16 && light_count < 24)
    {
        cluster_color = vec4(0.8, 0.6, 0.0, 0.8);
    }
    else if(light_count >= 24)
    {
        cluster_color = vec4(1.0, 0.0, 0.0, 0.8);
    }

    /*switch(light_count)
    {
        case 0:

        break;

        case 1:
            cluster_color.b = 1.0;
        break;

        default:
        case 2:
            cluster_color.b = 0.8;
            cluster_color.g = 0.2;
        break;

        case 3:

        break;
    }*/

	/*if(light_bitmask != 0)
    {
        cluster_color.r = 1.0;
    }
    else
    {
        cluster_color.g = 1.0;
    }*/

	//cluster_index = cluster(gl_FragCoord.x, gl_FragCoord.y, depth_sample, 1.0, 500.0, float(UNIFORM_r_width), float(UNIFORM_r_height));

	/*if((cluster_index.x % 2) == 0)
    {
        cluster_color.g = 1.0;
    }
    else
    {
        cluster_color.r = 1.0;
    }*/

	gl_FragColor = cluster_color;
}
