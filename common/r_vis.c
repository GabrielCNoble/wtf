#include "r_vis.h"
#include "r_main.h"
#include "r_common.h"
#include "r_debug.h"
#include "l_common.h"
#include "entity.h"
#include "containers/stack_list.h"
#include "bsp.h"
#include "gpu.h"

#include "GL/glew.h"

#include <float.h>

extern unsigned int r_visible_lights_count;
extern unsigned int r_visible_lights[];
extern struct list_t r_visible_entities;
extern unsigned int r_frame;
extern unsigned int r_clusters_per_row;
extern unsigned int r_cluster_rows;
extern unsigned int r_cluster_layers;
extern struct cluster_t *r_clusters;
extern unsigned int r_cluster_texture;

extern struct gpu_light_t *r_light_buffer;
extern unsigned int r_light_uniform_buffer;

extern struct gpu_bsp_node_t *r_bsp_buffer;
extern unsigned int r_bsp_uniform_buffer;
extern int r_bsp_node_count;

extern unsigned int r_visible_leaves_count;
extern struct bsp_dleaf_t **r_visible_leaves;


extern struct gpu_alloc_handle_t w_world_index_handle;
extern int w_world_leaves_count;
extern struct bsp_dleaf_t *w_world_leaves;
extern struct bsp_pnode_t *w_world_nodes;
extern struct batch_t *w_world_batches;
extern unsigned int *w_index_buffer;
extern int w_world_batch_count;
extern int w_world_start;
extern int w_world_vertices_count;
extern int w_world_nodes_count;


//extern int l_light_list_cursor;
//extern light_params_t *l_light_params;
//extern light_position_t *l_light_positions;

extern struct stack_list_t l_light_positions;
extern struct stack_list_t l_light_params;
extern struct stack_list_t l_light_clusters;


extern struct stack_list_t ent_entities[];
extern struct stack_list_t ent_world_transforms;
extern struct stack_list_t ent_entity_aabbs;


void renderer_VisibleWorld()
{
	int i;
	int c;
	int j;

	int start;
	int next;
	int leaf_index;
	int positive_z;

	//camera_t *active_camera = camera_GetActiveCamera();

    view_def_t *active_view = renderer_GetActiveView();

	float nzfar = -active_view->frustum.zfar;
	float nznear = -active_view->frustum.znear;
	float ntop = active_view->frustum.top;
	float nright = active_view->frustum.right;
	float qt = nznear / ntop;
	float qr = nznear / nright;
	float x_max;
	float x_min;
	float y_max;
	float y_min;

	float s;
	float e;

	struct bsp_dleaf_t *leaf;
	//triangle_group_t *group;
	struct batch_t *batch;
	struct bsp_striangle_t *triangle;
	//bsp_dleaf_t **visible;
	//int visible_count;
	unsigned int *indexes;

	vec4_t corners[8];

	if(!w_world_nodes)
		return;

	//s = engine_GetDeltaTime();

	//visible_leaves_count = 0;
	//leaf = bsp_GetCurrentLeaf(world_nodes, active_camera->world_position);
	r_visible_leaves = bsp_PotentiallyVisibleLeaves(&r_visible_leaves_count, active_view->world_position);

	/*e = engine_GetDeltaTime();

	printf("%f\n", e - s);*/

	if(r_visible_leaves)
	{
		/* zero out the next index of every world batch... */
		for(i = 0; i < w_world_batch_count; i++)
		{
			w_world_batches[i].next = 0;
		}

		for(j = 0; j < r_visible_leaves_count; j++)
		{
			leaf = r_visible_leaves[j];

			corners[0].x = leaf->center.x - leaf->extents.x;
			corners[0].y = leaf->center.y + leaf->extents.y;
			corners[0].z = leaf->center.z + leaf->extents.z;
			corners[0].w = 1.0;

			corners[1].x = leaf->center.x - leaf->extents.x;
			corners[1].y = leaf->center.y - leaf->extents.y;
			corners[1].z = leaf->center.z + leaf->extents.z;
			corners[1].w = 1.0;

			corners[2].x = leaf->center.x + leaf->extents.x;
			corners[2].y = leaf->center.y - leaf->extents.y;
			corners[2].z = leaf->center.z + leaf->extents.z;
			corners[2].w = 1.0;

			corners[3].x = leaf->center.x + leaf->extents.x;
			corners[3].y = leaf->center.y + leaf->extents.y;
			corners[3].z = leaf->center.z + leaf->extents.z;
			corners[3].w = 1.0;



			corners[4].x = leaf->center.x - leaf->extents.x;
			corners[4].y = leaf->center.y + leaf->extents.y;
			corners[4].z = leaf->center.z - leaf->extents.z;
			corners[4].w = 1.0;

			corners[5].x = leaf->center.x - leaf->extents.x;
			corners[5].y = leaf->center.y - leaf->extents.y;
			corners[5].z = leaf->center.z - leaf->extents.z;
			corners[5].w = 1.0;

			corners[6].x = leaf->center.x + leaf->extents.x;
			corners[6].y = leaf->center.y - leaf->extents.y;
			corners[6].z = leaf->center.z - leaf->extents.z;
			corners[6].w = 1.0;

			corners[7].x = leaf->center.x + leaf->extents.x;
			corners[7].y = leaf->center.y + leaf->extents.y;
			corners[7].z = leaf->center.z - leaf->extents.z;
			corners[7].w = 1.0;

			x_max = -10.9;
			x_min = 10.9;

			y_max = -10.9;
			y_min = 10.9;

			positive_z = 0;
			for(i = 0; i < 8; i++)
			{
				mat4_t_vec4_t_mult(&active_view->view_data.view_matrix, &corners[i]);
				if(corners[i].z > nznear)
				{
					corners[i].z = nznear;
					positive_z++;
				}

				corners[i].x = (corners[i].x * qr) / corners[i].z;
				corners[i].y = (corners[i].y * qt) / corners[i].z;

				if(corners[i].x > x_max) x_max = corners[i].x;
				if(corners[i].x < x_min) x_min = corners[i].x;

				if(corners[i].y > y_max) y_max = corners[i].y;
				if(corners[i].y < y_min) y_min = corners[i].y;

			}


			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;

			if(x_min > 1.0) x_min = 1.0;
			else if(x_min < -1.0) x_min = -1.0;

			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;

			if(y_min > 1.0) y_min = 1.0;
			else if(y_min < -1.0) y_min = -1.0;


			if((x_max - x_min) * (y_max - y_min) <= 0.0 || positive_z == 8)
			{
				if(j < r_visible_leaves_count - 1)
				{
					r_visible_leaves[j] = r_visible_leaves[r_visible_leaves_count - 1];
					j--;
				}
				r_visible_leaves_count--;
			}

		}

		/* for each leaf on the list... */
		for(j = 0; j < r_visible_leaves_count; j++)
		{
			leaf = r_visible_leaves[j];

			leaf->visible_frame = r_frame;

			c = leaf->tris_count;

			/* add it's triangles for rendering... */
			for(i = 0; i < c; i++)
			{
				triangle = &leaf->tris[i];
				batch = &w_world_batches[triangle->batch];

				start = batch->start;
				next = batch->next;

				w_index_buffer[start + next	] = w_world_start + triangle->first_vertex;
				w_index_buffer[start + next + 1] = w_world_start + triangle->first_vertex + 1;
				w_index_buffer[start + next + 2] = w_world_start + triangle->first_vertex + 2;

				batch->next += 3;
			}
		}

		renderer_WriteNonMapped(w_world_index_handle, 0, w_index_buffer, sizeof(int) * w_world_vertices_count);
	}

    glBindBuffer(GL_UNIFORM_BUFFER, r_bsp_uniform_buffer);

    for(i = 0; i < w_world_nodes_count; i++)
	{
		r_bsp_buffer[i].normal_dist.x = w_world_nodes[i].normal.x;
		r_bsp_buffer[i].normal_dist.y = w_world_nodes[i].normal.y;
		r_bsp_buffer[i].normal_dist.z = w_world_nodes[i].normal.z;

		r_bsp_buffer[i].normal_dist.w = w_world_nodes[i].dist;

		r_bsp_buffer[i].children[0] = w_world_nodes[i].child[0];
		r_bsp_buffer[i].children[1] = w_world_nodes[i].child[1];
	}

    r_bsp_buffer[0].node_count = i;

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct gpu_bsp_node_t) * R_MAX_BSP_NODES, r_bsp_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void renderer_VisibleLightsBounds()
{

	int i;
	int c;

	int x;
	int y;
	int z;

//	camera_t *active_camera = camera_GetActiveCamera();
    view_def_t *active_view = renderer_GetActiveView();
	//camera_t *active_camera = view;
	vec4_t light_origin;

//	view_clusters_t *view_clusters;
//	view_light_t *view_light;
	mat4_t mt;

	vec2_t ac;
	vec2_t lb;
	vec2_t rb;

	//float nzfar = -view->frustum.zfar;
	//float nznear = -view->frustum.znear;
	//float ntop = view->frustum.top;
	//float nright = view->frustum.right;

	float x_max;
	float x_min;
	float y_max;
	float y_min;
	float d;
	float t;
	float denom;
	float si;
	float co;
	float k;
	float l;
	float light_radius;
	float light_z;
	int positive_z;
	int light_index;
	int view_clusters_index;
	int offset;

//	light_position_t *position;
//	light_params_t *params;

    struct light_position_data_t *position;
    struct light_params_data_t *params;
    struct light_cluster_data_t *cluster;

	//float qt = -view_data->projection_matrix.floats[1][1];
	//float qr = -view_data->projection_matrix.floats[0][0];
	/* for z' == 1 -> z == znear */
	//float znear = view_data->projection_matrix.floats[3][2] / 2.0;

	float znear = -active_view->frustum.znear;
	float qr = znear / active_view->frustum.right;
	float qt = znear / active_view->frustum.top;

	/* FUCK!!! */
	float zfar = -active_view->frustum.zfar;
	/* for z' == -1 ->z == zfar */
	//float zfar = view_data->projection_matrix.floats[3][3] - view_data->projection_matrix.floats[3][2];

	short cluster_x_start;
	short cluster_y_start;
	short cluster_z_start;
	short cluster_x_end;
	short cluster_y_end;
	short cluster_z_end;

	#define CLUSTER_NEAR 1.0

	denom = log(-zfar / CLUSTER_NEAR);

	//c = view_data->view_lights_list_cursor;
	c = r_visible_lights_count;

	for(i = 0; i < c; i++)
	{

		//light_index = view_data->view_lights[i].light_index;
		light_index = r_visible_lights[i];


		//position = &l_light_positions[light_index];
		//params = &l_light_params[light_index];


		position = (struct light_position_data_t *)l_light_positions.elements + light_index;
		cluster = (struct light_cluster_data_t *)l_light_clusters.elements + light_index;
		//params = (struct light_params_data_t *)l_light_params.elemebts + light_index;

		light_origin.x = position->position.x;
		light_origin.y = position->position.y;
		light_origin.z = position->position.z;
		light_origin.w = 1.0;

		//mat4_t_vec4_t_mult(&view_data->view_matrix, &light_origin);
		mat4_t_vec4_t_mult(&active_view->view_data.view_matrix, &light_origin);

		light_radius = UNPACK_LIGHT_RADIUS(position->radius);

		d = light_origin.x * light_origin.x + light_origin.y * light_origin.y + light_origin.z * light_origin.z;

		if(light_radius * light_radius > d)
		{
			x_max = 1.0;
			x_min = -1.0;
			y_max = 1.0;
			y_min = -1.0;
			positive_z = 0;
			goto _skip_stuff;
		}


		/*
			from   '2D Polyhedral Bounds of a Clipped,
			   		Perspective-Projected 3D Sphere
			   		by Michael Mara and Morgan McGuire'
		*/

		x_max = -10.0;
		x_min = 10.0;

		y_max = -10.0;
		y_min = 10.0;

		positive_z = 0;

		ac.x = light_origin.x;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
		l = light_radius * light_radius;
		k = znear - ac.y;
		k = sqrt(light_radius * light_radius - k * k);

		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);


			si = light_radius / d;
			co = t / d;

			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;

			if(rb.y > znear)
			{
				rb.x = ac.x + k;
				rb.y = znear;
				positive_z++;
			}

			if(lb.y > znear)
			{
				lb.x = ac.x - k;
				lb.y = znear;
				positive_z++;
			}

			x_min = (qr * lb.x) / lb.y;
			x_max = (qr * rb.x) / rb.y;

			if(x_min < -1.0) x_min = -1.0;
			else if(x_min > 1.0) x_min = 1.0;
			if(x_max > 1.0) x_max = 1.0;
			else if(x_max < -1.0) x_max = -1.0;
		}
		else
		{
			x_min = -1.0;
			x_max = 1.0;
		}

		ac.x = light_origin.y;
		ac.y = light_origin.z;
		d = ac.x * ac.x + ac.y * ac.y;
				//l = light_radius * light_radius;

		if(d > l)
		{
			t = sqrt(d - light_radius * light_radius);
			d = sqrt(d);
					//k = nznear - ac.y;
					//k = sqrt(light_radius * light_radius - k * k);

			si = light_radius / d;
			co = t / d;

			rb.x = ac.x * co - ac.y * si;
			rb.y = ac.x * si + ac.y * co;
			lb.x = ac.x * co + ac.y * si;
			lb.y = -ac.x * si + ac.y * co;


			if(rb.y > znear)
			{
				rb.x = ac.x + k;
				rb.y = znear;
				positive_z++;
			}

			if(lb.y > znear)
			{
				lb.x = ac.x - k;
				lb.y = znear;
				positive_z++;
			}

			y_min = (qt * lb.x) / lb.y;
			y_max = (qt * rb.x) / rb.y;

			if(y_min < -1.0) y_min = -1.0;
			else if(y_min > 1.0) y_min = 1.0;
			if(y_max > 1.0) y_max = 1.0;
			else if(y_max < -1.0) y_max = -1.0;
		}
		else
		{
			y_min = -1.0;
			y_max = 1.0;
		}

		if((x_max - x_min) * (y_max - y_min) == 0.0 || positive_z == 4)
		{
			_remove_light:

			if(i < c)
			{
				r_visible_lights[i] = r_visible_lights[c - 1];
				//view_data->view_lights[i] = view_data->view_lights[c - 1];
			}
			c--;
			i--;
			continue;
		}
		else
		{
			_skip_stuff:

            //params->screen_min_max.x = x_min;
            //params->screen_min_max.y = y_min;

            //params->screen_min_max.z = x_max;
            //params->screen_min_max.w = y_max;

			cluster_x_start = (int)((x_min * 0.5 + 0.5) * r_clusters_per_row);
			cluster_y_start = (int)((y_min * 0.5 + 0.5) * r_cluster_rows);
			light_z = light_origin.z + light_radius;

			if(cluster_x_start >= r_clusters_per_row) cluster_x_start = r_clusters_per_row - 1;
			if(cluster_y_start >= r_cluster_rows) cluster_y_start = r_cluster_rows - 1;

			if(light_z > -CLUSTER_NEAR) cluster_z_start = 0;
			else cluster_z_start = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(r_cluster_layers));

			if(cluster_z_start > r_cluster_layers) cluster_z_start = r_cluster_layers - 1;

			cluster_x_end = (int)((x_max * 0.5 + 0.5) * r_clusters_per_row);
			cluster_y_end = (int)((y_max * 0.5 + 0.5) * r_cluster_rows);
			light_z = light_origin.z - light_radius;

			if(cluster_x_end >= r_clusters_per_row) cluster_x_end = r_clusters_per_row - 1;
			if(cluster_y_end >= r_cluster_rows) cluster_y_end = r_cluster_rows - 1;

			if(light_z > -CLUSTER_NEAR) cluster_z_end = 0;
			else cluster_z_end = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(r_cluster_layers));

			if(cluster_z_end > r_cluster_layers) cluster_z_end = r_cluster_layers - 1;


            cluster->first_cluster.x = cluster_x_start;
            cluster->first_cluster.y = cluster_y_start;
            cluster->first_cluster.z = cluster_z_start;


            cluster->last_cluster.x = cluster_x_end;
            cluster->last_cluster.y = cluster_y_end;
            cluster->last_cluster.z = cluster_z_end;

			//params->first_cluster.x = cluster_x_start;
			//params->first_cluster.y = cluster_y_start;
			//params->first_cluster.z = cluster_z_start;

			//params->last_cluster.x = cluster_x_end;
			//params->last_cluster.y = cluster_y_end;
			//params->last_cluster.z = cluster_z_end;
		}
	}

	r_visible_lights_count = c;
}




void renderer_VisibleLights()
{
    //printf("light_VisibleLights\n");
	int i;
	int c;

	int j;
	int k;
	//int leaf_index;
	//int int_index;
	//int bit_index;

	//struct bsp_dleaf_t *leaf;
	//struct bsp_dleaf_t *cur_leaf;
	//camera_t *active_camera = camera_GetActiveCamera();
	view_def_t *active_view = renderer_GetActiveView();
	vec4_t light_origin;
	vec3_t v;
	//vec3_t farthest;
	float farthest;
	int farthest_index;
	//light_position_t *pos;
	//light_params_t *parms;

	struct light_position_data_t *position;
	struct light_params_data_t *params;
	struct light_cluster_data_t *cluster;


	bsp_lights_t lights;

	r_visible_lights_count = 0;

	float d;
	float radius;
	float s;
	float e;

	//if(!w_world_leaves)
	//{
	for(i = 0; i < l_light_params.element_count; i++)
	{
        //params = (struct light_params_data_t *)l_light_params.elements + i;

        position = (struct light_position_data_t *)l_light_positions.elements + i;

        if(!(position->flags & LIGHT_INVALID))
        {
            r_visible_lights[r_visible_lights_count++] = i;
        }


		//if(!(l_light_params[i].bm_flags & LIGHT_INVALID))
		//{
		//	r_visible_lights[r_visible_lights_count++] = i;
		//}


	}
	//}
	//else
	//{
	//	s = engine_GetDeltaTime();

	//	for(i = 0; i < MAX_WORLD_LIGHTS >> 5; i++)
	//	{
	//		lights.lights[i] = 0;
	//	}

	//	for(i = 0; i < c; i++)
	//	{
	//		leaf = w_visible_leaves[i];

	//		leaf_index = leaf - w_world_leaves;

	//		for(j = 0; j < MAX_WORLD_LIGHTS >> 5; j++)
	//		{
	//			lights.lights[j] |= w_leaf_lights[leaf_index].lights[j];
	//		}
	//	}

	//	for(i = 0; i < MAX_WORLD_LIGHTS; i++)
	//	{
	//		if(lights.lights[i >> 5] & (1 << (i % 32)))
	//		{
	//			w_visible_lights[w_visible_lights_count++] = i;
	//		}
	//	}
	//}

	renderer_VisibleLightsBounds();


	/* drop far away lights... */
	/* NOTE: quicksorting the lights might scale better... */
	if(r_visible_lights_count > R_MAX_VISIBLE_LIGHTS)
	{
		//printf("maximum visible lights exceeded! Eliminating...\n");
		c = r_visible_lights_count - R_MAX_VISIBLE_LIGHTS;

		for(i = 0; i < c; i++)
		{
			farthest = FLT_MIN;

			for(j = 0; j < r_visible_lights_count; j++)
			{
				k = r_visible_lights[j];

				position = (struct light_position_data_t *)l_light_positions.elements + k;

				v.x = position->position.x - active_view->world_position.x;
				v.y = position->position.y - active_view->world_position.y;
				v.z = position->position.z - active_view->world_position.z;

				d = dot3(v, v);

				if(d > farthest)
				{
					farthest = d;
					farthest_index = j;
				}
			}

			if(farthest_index < r_visible_lights_count - 1)
			{
				r_visible_lights[farthest_index] = r_visible_lights[r_visible_lights_count - 1];
			}

			r_visible_lights_count--;
			c--;
		}
	}

    //renderer_VisibleLightsOnClusters();
	renderer_UploadVisibleLights();
}
/*
void renderer_VisibleLightsOnClusters()
{
    int i;

    int cluster_x_start;
    int cluster_y_start;
    int cluster_z_start;

    view_def_t *active_view;


    int cluster_x_end;
    int cluster_y_end;
    int cluster_z_end;

    float denom;

    light_params_t *parms;

    #define CLUSTER_NEAR 1.0

    active_view = renderer_GetActiveView();

	denom = log(active_view->frustum.zfar / CLUSTER_NEAR);

    for(i = 0; i < r_visible_lights_count; i++)
    {
        parms = &l_light_params[r_visible_lights[i]];

        cluster_x_start = (int)((parms->screen_min_max.x * 0.5 + 0.5) * r_clusters_per_row);
		cluster_y_start = (int)((parms->screen_min_max.y * 0.5 + 0.5) * r_cluster_rows);
		light_z = light_origin.z + light_radius;

		if(cluster_x_start >= r_clusters_per_row) cluster_x_start = r_clusters_per_row - 1;
		if(cluster_y_start >= r_cluster_rows) cluster_y_start = r_cluster_rows - 1;


		if(light_z > -CLUSTER_NEAR) cluster_z_start = 0;
		else cluster_z_start = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(r_cluster_layers));

		if(cluster_z_start > r_cluster_layers) cluster_z_start = r_cluster_layers - 1;


		cluster_x_end = (int)((parms->screen_min_max.z * 0.5 + 0.5) * r_clusters_per_row);
		cluster_y_end = (int)((parms->screen_min_max.w * 0.5 + 0.5) * r_cluster_rows);
		light_z = light_origin.z - light_radius;

		if(cluster_x_end >= r_clusters_per_row) cluster_x_end = r_clusters_per_row - 1;
		if(cluster_y_end >= r_cluster_rows) cluster_y_end = r_cluster_rows - 1;

		if(light_z > -CLUSTER_NEAR) cluster_z_end = 0;
		else cluster_z_end = (int)((log(-light_z / CLUSTER_NEAR) / denom) * (float)(r_cluster_layers));

		if(cluster_z_end > r_cluster_layers) cluster_z_end = r_cluster_layers - 1;


		params->first_cluster.x = cluster_x_start;
		params->first_cluster.y = cluster_y_start;
		params->first_cluster.z = cluster_z_start;

		params->last_cluster.x = cluster_x_end;
		params->last_cluster.y = cluster_y_end;
		params->last_cluster.z = cluster_z_end;
    }
}*/

void renderer_UploadVisibleLights()
{
    int i;
	int c;
	int light_index;
	int gpu_index = 0;

	float s;
	float e;

	int x;
	int y;
	int z;

	int cluster_x_start;
	int cluster_y_start;
	int cluster_z_start;

	int cluster_x_end;
	int cluster_y_end;
	int cluster_z_end;

	int cluster_index;
	int offset;


	vec4_t light_position;
	//light_position_t *pos;
	//light_params_t *parms;
	//camera_t *active_camera;
	//view_light_t *view_lights;

	struct light_position_data_t *position;
	struct light_params_data_t *params;
	struct light_cluster_data_t *cluster;

    view_def_t *active_view;

	struct gpu_light_t *lights;

	if(!r_light_uniform_buffer)
		return;


	R_DBG_PUSH_FUNCTION_NAME();


	float proj_param_a;
	float proj_param_b;



//	proj_param_a = l_shadow_map_projection_matrix.floats[2][2];
//	proj_param_b = l_shadow_map_projection_matrix.floats[3][2];

	active_view = renderer_GetActiveView();
	glBindBuffer(GL_UNIFORM_BUFFER, r_light_uniform_buffer);
	//lights = w_light_buffer;

	//view_lights = view_data->view_lights;

	for(i = 0; i < r_visible_lights_count && i < R_MAX_VISIBLE_LIGHTS; i++)
	{
		//light_index = view_lights[i].light_index;
		light_index = r_visible_lights[i];

		//light_AllocShadowMap(light_index);

		//pos = &l_light_positions[light_index];
		//parms = &l_light_params[light_index];

        position = (struct light_position_data_t *)l_light_positions.elements + light_index;
        params = (struct light_params_data_t *)l_light_params.elements + light_index;
        //cluster = (struct )

		light_position.x = position->position.x;
		light_position.y = position->position.y;
		light_position.z = position->position.z;
		light_position.w = 1.0;

		//mat4_t_vec4_t_mult(&view_data->view_matrix, &light_position);
		mat4_t_vec4_t_mult(&active_view->view_data.view_matrix, &light_position);

		r_light_buffer[i].position_radius.x = light_position.x;
		r_light_buffer[i].position_radius.y = light_position.y;
		r_light_buffer[i].position_radius.z = light_position.z;

		r_light_buffer[i].position_radius.w = UNPACK_LIGHT_RADIUS(position->radius);

		r_light_buffer[i].color_energy.r = (float)params->r / 255.0;
		r_light_buffer[i].color_energy.g = (float)params->g / 255.0;
		r_light_buffer[i].color_energy.b = (float)params->b / 255.0;
		r_light_buffer[i].color_energy.a = UNPACK_LIGHT_ENERGY(params->energy);

//		w_light_buffer[i].x_y = (parms->y << 16) | parms->x;
		r_light_buffer[i].bm_flags = position->flags & (~LIGHT_GENERATE_SHADOWS);

		//r_light_buffer[i].proj_param_a = proj_param_a;
		//r_light_buffer[i].proj_param_b = proj_param_b;
		//r_light_buffer[i].shadow_map = parms->shadow_map;

		/*if(parms->bm_flags & LIGHT_UPLOAD_INDICES)
		{
			if(parms->indices_handle.alloc_index == INVALID_GPU_ALLOC_INDEX)
			{
                parms->indices_handle = gpu_AllocIndexesAlign(sizeof(int) * 8192 * 3, sizeof(int));
				parms->indices_start = gpu_GetAllocStart(parms->indices_handle) / sizeof(int);
			}

            gpu_WriteNonMapped(parms->indices_handle, 0, parms->visible_triangles.elements, sizeof(int) * parms->visible_triangles.element_count);

            parms->bm_flags &= ~LIGHT_UPLOAD_INDICES;
		}*/
	}

	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(struct gpu_light_t) * R_MAX_VISIBLE_LIGHTS, r_light_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	/*************************************************************/
	/*************************************************************/
	/*************************************************************/

	#undef CLUSTER_OFFSET

	#define CLUSTER_OFFSET(x, y, z) (x+y*r_clusters_per_row+z*r_clusters_per_row*r_cluster_rows)



	for(z = 0; z < r_cluster_layers; z++)
	{
		for(y = 0; y < r_cluster_rows; y++)
		{
			for(x = 0; x < r_clusters_per_row; x++)
			{
				cluster_index = CLUSTER_OFFSET(x, y, z);
				r_clusters[cluster_index].light_indexes_bm = 0;
			}
		}
	}


	//view_lights = view_data->view_lights;
	//c = view_data->view_lights_list_cursor;

	c = r_visible_lights_count;

	for(i = 0; i < c && i < R_MAX_VISIBLE_LIGHTS; i++)
	{
		//light_index = view_lights[i].light_index;
		light_index = r_visible_lights[i];
		cluster = (struct light_cluster_data_t *)l_light_clusters.elements + light_index;
		//parms = &l_light_params[light_index];

		//UNPACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end, view_lights[i].view_clusters);
		//UNPACK_CLUSTER_INDEXES2(cluster_x_start, cluster_y_start, cluster_z_start, cluster_x_end, cluster_y_end, cluster_z_end, parms->first_cluster);

		cluster_x_start = cluster->first_cluster.x;
		cluster_y_start = cluster->first_cluster.y;
		cluster_z_start = cluster->first_cluster.z;

		cluster_x_end = cluster->last_cluster.x;
		cluster_y_end = cluster->last_cluster.y;
		cluster_z_end = cluster->last_cluster.z;

		for(z = cluster_z_start; z <= cluster_z_end; z++)
		{
			for(y = cluster_y_start; y <= cluster_y_end; y++)
			{
				for(x = cluster_x_start; x <= cluster_x_end; x++)
				{
					cluster_index = CLUSTER_OFFSET(x, y, z);
					r_clusters[cluster_index].light_indexes_bm |= 1 << i;
				}
			}
		}

	}


	/* this is a bottleneck... */
	glBindTexture(GL_TEXTURE_3D, r_cluster_texture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, r_clusters_per_row, r_cluster_rows, r_cluster_layers, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, r_clusters);
	glBindTexture(GL_TEXTURE_3D, 0);


	R_DBG_POP_FUNCTION_NAME();
}

void renderer_VisibleEntities()
{
	int i;
	int c;
	int j;

	struct entity_t *entity;

	struct entity_transform_t *world_transforms;
	struct entity_transform_t *world_transform;

	struct entity_aabb_t *aabbs;
	struct entity_aabb_t *aabb;

	struct model_component_t *model_component;
	struct model_t *model;
	struct batch_t *batch;
	struct lod_t *lod;

	//camera_t *active_camera = camera_GetActiveCamera();

	view_def_t *active_view = renderer_GetActiveView();

	vec3_t box[8];

	struct entity_handle_t *visible_entities;
	int visible_entity_count;

	world_transforms = (struct entity_transform_t *)ent_world_transforms.elements;
	aabbs = (struct entity_aabb_t *)ent_entity_aabbs.elements;


	r_visible_entities.element_count = 0;
	visible_entities = (struct entity_handle_t *)r_visible_entities.elements;
	visible_entity_count = 0;
	c = ent_entities[0].element_count;

	//if(!w_world_)
	for(i = 0; i < c; i++)
	{
		entity = entity_GetEntityPointerIndex(i);

		if(!entity)
		{
			continue;
		}

		if(entity->components[COMPONENT_TYPE_MODEL].type == COMPONENT_TYPE_NONE)
		{
			/* entities without models won't get rendered... */
			continue;
		}

		world_transform = world_transforms + entity->components[COMPONENT_TYPE_TRANSFORM].index;
		aabb = aabbs + entity->components[COMPONENT_TYPE_TRANSFORM].index;

		if(camera_BoxScreenArea((camera_t *)active_view, vec3_t_c(world_transform->transform.floats[3][0], world_transform->transform.floats[3][1], world_transform->transform.floats[3][2]), aabb->current_extents))
		{
			if(visible_entity_count >= r_visible_entities.max_elements)
			{
				list_resize(&r_visible_entities, r_visible_entities.max_elements + 128);
				visible_entities = (struct entity_handle_t *)r_visible_entities.elements;
			}

			visible_entities[visible_entity_count].def = 0;
			visible_entities[visible_entity_count].entity_index = i;

			visible_entity_count++;
		}
	}

	//printf("%d\n", visible_entity_count);

	for(i = 0; i < visible_entity_count; i++)
	{
		entity = entity_GetEntityPointerHandle(visible_entities[i]);

		model_component = entity_GetComponentPointer(entity->components[COMPONENT_TYPE_MODEL]);
		world_transform = world_transforms + entity->components[COMPONENT_TYPE_TRANSFORM].index;
		model = model_GetModelPointerIndex(model_component->model_index);

		lod = &model->lods[0];

		for(j = 0; j < model->batch_count; j++)
		{
			batch = lod->batches + j;
			//renderer_SubmitDrawCommand(&world_transform->transform, lod->draw_mode, model->vert_start + batch->start, batch->next, batch->material_index, 0);
			renderer_SubmitDrawCommand(&world_transform->transform, GL_TRIANGLES, lod->lod_indice_buffer_start + batch->start, batch->next, batch->material_index, 1);
		}
	}

	r_visible_entities.element_count = visible_entity_count;


}





