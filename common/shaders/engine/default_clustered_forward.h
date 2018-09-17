#ifndef MASSACRE_DEFAULT_CLUSTERED_FORWARD
#define MASSACRE_DEFAULT_CLUSTERED_FORWARD

	#include "engine/default_uniforms.h"


	#ifdef MASSACRE_VERTEX_SHADER

		#include "engine/default_attribs.h"

		out vec2 tex_coords;
		out vec3 eye_space_normal;
		out vec3 eye_space_position;
		out vec3 eye_space_tangent;

		void transform_attribs()
		{
			tex_coords = vertex_tex_coords;
			eye_space_normal = vec3(UNIFORM_model_view_matrix * vec4(vertex_normal.xyz, 0.0));
			eye_space_tangent = vec3(UNIFORM_model_view_matrix * vec4(vertex_tangent.xyz, 0.0));
			eye_space_position = vec3(UNIFORM_model_view_matrix * vec4(vertex_position.xyz, 1.0));
		}

		vec4 evaluate_vertex()
		{
			transform_attribs();
			return UNIFORM_model_view_projection_matrix * vec4(vertex_position.xyz, 1.0);
		}

	#else

		in vec2 tex_coords;
		in vec3 eye_space_normal;
		in vec3 eye_space_position;
		in vec3 eye_space_tangent;

		/*
		===============================================================
		===============================================================
		===============================================================
		*/

		ivec3 cluster(float x, float y, float view_z, float znear, float zfar, float width, float height)
		{
			int cluster;
			int row;
			int layer;

			cluster = min(int((x / float(UNIFORM_r_width)) * float(UNIFORM_r_clusters_per_row)), UNIFORM_r_clusters_per_row);
			row = min(int((y / float(UNIFORM_r_height)) * float(UNIFORM_r_cluster_rows)), UNIFORM_r_cluster_rows);
			layer = int((log(view_z / znear) / log(zfar / znear)) * float(UNIFORM_r_cluster_layers));
			layer = max(min(layer, UNIFORM_r_cluster_layers), 0);
			return ivec3(cluster, row, layer);
		}

		/*
		===============================================================
		===============================================================
		===============================================================
		*/

		#if 0
		float sample_cube_map(vec3 frag_pos, vec3 frag_normal, int light_index, out vec3 debug_color)
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

			#if 1

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

			#endif

			u0 = 0.5 * (u0 / largest) + 0.5;
			v0 = 0.5 * (v0 / largest) + 0.5;

			v0 = x + ox0 + w * v0;
			u0 = y + oy0 + h * u0;


			shadow0 = texelFetch(UNIFORM_texture_sampler2, ivec2(SHARED_SHADOW_MAP_WIDTH * v0, SHARED_SHADOW_MAP_HEIGHT * u0), 0).r;
			//shadow0 = texture2D(UNIFORM_texture_sampler2, vec2(v0, u0), 0).r;

			//if(dist > shadow0 + 0.3)
			//if(fz > shadow0 + 0.00001)
			//{
			//	return 0.0;
			//}
			//return 1.0;

			return float(dist < shadow0 + 0.3);
		}

		#endif


		float pixel_shadow(int light_index)
		{
			vec3 eye_space_light_position;
			vec3 eye_space_light_vec;
			vec3 light_space_vec;
			vec3 abs_light_space_vec;
			vec3 light_space_normal;

			float shadow_sample;
			float largest;

			float shadow = 1.0;

			float light_z;
			float pixel_z;

            eye_space_light_position = light_params[light_index].position_radius.xyz;
            eye_space_light_vec = eye_space_position - eye_space_light_position;

			light_space_vec = vec3(vec4(eye_space_light_vec, 0.0) * UNIFORM_view_matrix);
			light_space_normal = vec3(vec4(eye_space_normal, 0.0) * UNIFORM_view_matrix);

			//light_space_vec.z = -light_space_vec.z;

			shadow_sample = -(texture(UNIFORM_texture_cube_array_sampler0, vec4(light_space_vec, float(light_params[light_index].shadow_map))).r * 2.0 - 1.0);

			//shadow_sample = texture(UNIFORM_texture_cube_array_sampler0, vec4(light_space_vec, float(light_params[light_index].shadow_map))).r * 2.0 - 1.0;

			//shadow_sample = texture(UNIFORM_texture_cube_array_sampler0, vec4(eye_space_light_vec, 0.0)).r;

            abs_light_space_vec = abs(light_space_vec);

			largest = max(abs_light_space_vec.x, max(abs_light_space_vec.y, abs_light_space_vec.z));

			//pixel_z = (light_params[light_index].proj_param_a + light_params[light_index].proj_param_b / -largest);
			//light_z = -shadow_sample;

			pixel_z = -largest;
			light_z = light_params[light_index].proj_param_b / (shadow_sample - light_params[light_index].proj_param_a);

			//light_space_vec = normalize(light_space_vec);

			return float(pixel_z >= light_z - 0.1);

			/*if(pixel_z < light_z - 0.1)
			{
				shadow = 0.0;
			}*/

			//if(pixel_z + 0.00000 < light_z)
			//{
			//	shadow = 0.25;
			//}

			//return shadow;
		}

		/*
		===============================================================
		===============================================================
		===============================================================
		*/

		vec4 pixel_albedo()
		{
			return mix(gl_FrontMaterial.diffuse, texture2D(UNIFORM_texture_sampler0, tex_coords), float(bool(UNIFORM_material_flags & MATERIAL_USE_DIFFUSE_TEXTURE)));
		}

		vec3 pixel_normal()
		{
			vec3 normal;
			mat3 tbn;

			if(bool(UNIFORM_material_flags & MATERIAL_USE_NORMAL_TEXTURE))
			{
				tbn[0] = (eye_space_tangent);
				tbn[1] = (cross(eye_space_tangent, eye_space_normal));
				tbn[2] = (eye_space_normal);

				normal = (texture2D(UNIFORM_texture_sampler1, tex_coords).rgb * 2.0 - 1.0);

				normal.x = mix(normal.x, -normal.x, float(bool(UNIFORM_material_flags & MATERIAL_INVERT_NORMAL_X)));
				normal.y = mix(normal.y, -normal.y, float(bool(UNIFORM_material_flags & MATERIAL_INVERT_NORMAL_Y)));

				return normalize(tbn * normal);
			}
			else
			{
				return normalize(eye_space_normal);
			}
		}

		vec4 accumulate_lights(vec3 albedo, vec3 normal)
		{

			float shininess;
			float distance;
			float radius;
			float energy;
			float shadow;
			float attenuation;

			ivec3 current_cluster;
			int light_index;
			unsigned int light_bitmask;

			vec3 eye_vec;
			vec3 light_vec;
			vec3 half_vec;
			vec3 light_color;
			vec3 specular;
			vec3 diffuse;

			vec4 accum = vec4(albedo * 0.5, 0.0);


			shininess = 512.0;

			/* fucking stupid GL drivers... */
			current_cluster = cluster(gl_FragCoord.x, gl_FragCoord.y, -eye_space_position.z, 1.0, 500.0, float(UNIFORM_r_width), float(UNIFORM_r_height));
			light_bitmask = texelFetch(UNIFORM_cluster_texture, current_cluster, 0).r;

			eye_vec = normalize(-eye_space_position);

			for(light_index = 0; light_index < LIGHT_UNIFORM_BUFFER_SIZE; light_index++)
			{

				if((light_bitmask & uint(1)) == uint(1))
				{
					light_vec = light_params[light_index].position_radius.xyz - eye_space_position;
					light_color = light_params[light_index].color_energy.rgb;

					distance = length(light_vec);
					//distance = dot(light_vec, light_vec);
					energy = light_params[light_index].color_energy.a;
					radius = light_params[light_index].position_radius.w;

					//shadow = 1.0;

					shadow = pixel_shadow(light_index);

					light_vec /= distance;
					half_vec = normalize(eye_vec + light_vec);

					attenuation = (1.0 / (distance)) * max(1.0 - (distance / radius), 0.0) * energy;
					diffuse = ((albedo / 3.14159265) * max(dot(light_vec, normal), 0.0)) * attenuation;
					specular = light_color * pow(max(dot(half_vec, normal), 0.0), shininess) * attenuation * 1.5;

					accum += ((vec4(diffuse * light_color, 1.0)) + vec4(specular, 1.0)) * shadow;
				}

				light_bitmask >>= 1;
			}

			return accum;
		}

		vec4 evaluate_pixel()
		{
			vec3 albedo;
			vec3 normal;

			albedo = pixel_albedo().xyz;
			normal = pixel_normal();

			return accumulate_lights(albedo, normal);
		}

	#endif

#endif

