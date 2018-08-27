#include <stdio.h>

#include "GL\glew.h"


#include "indirect.h"
#include "bsp_common.h"

#include "l_main.h"


#include "camera.h"

/* from world.c */
//extern int world_nodes_count;
//extern struct bsp_pnode_t *world_nodes;
//extern bsp_polygon_t *node_polygons;			/* necessary to quickly build portals... */
//extern int world_leaves_count;
//extern bsp_dleaf_t *world_leaves;



/* from light.c */
extern light_position_t *visible_light_positions;
extern light_params_t *visible_light_params;
extern light_position_t *light_positions;
extern light_params_t *light_params;
extern int light_count;
extern int visible_light_count;



int x_vols[4] = {0, 0, 0, 0};
int y_vols[4] = {0, 0, 0, 0};
int z_vols[4] = {0, 0, 0, 0};
light_volume_t *lods[4] = {NULL, NULL, NULL, NULL};

int affected_count;
int *affected_indexes;

#define VOLUME_LOD0_SIZE 2.0
#define VOLUME_LOD1_SIZE 4.0
#define VOLUME_LOD2_SIZE 8.0
#define VOLUME_LOD3_SIZE 16.0



void indirect_BuildVolumes()
{
	#if 0
	int i;
	int c;
	int j;
	int k;

	int x;
	int y;
	int z;

	bsp_dleaf_t *leaf;
	vec3_t max;
	vec3_t min;

	vec3_t center;

	float x_max = -9999999999.9;
	float y_max = -9999999999.9;
	float z_max = -9999999999.9;

	float x_min = 9999999999.9;
	float y_min = 9999999999.9;
	float z_min = 9999999999.9;

	int row_size;
	int layer_size;


	if(world_leaves)
	{
		c = world_leaves_count;

		for(i = 0; i < c; i++)
		{
			leaf = &world_leaves[i];

			max.x = leaf->center.x + leaf->extents.x;
			max.y = leaf->center.y + leaf->extents.y;
			max.z = leaf->center.z + leaf->extents.z;

			min.x = leaf->center.x - leaf->extents.x;
			min.y = leaf->center.y - leaf->extents.y;
			min.z = leaf->center.z - leaf->extents.z;

			if(max.x > x_max) x_max = max.x;
			if(max.y > y_max) y_max = max.y;
			if(max.z > z_max) z_max = max.z;

			if(min.x < x_min) x_min = min.x;
			if(min.y < y_min) y_min = min.y;
			if(min.z < z_min) z_min = min.z;

		}

		/*x_vols[VOLUME_LOD0] = (int)((x_max - x_min) / VOLUME_LOD0_SIZE) + 2;
		y_vols[VOLUME_LOD0] = (int)((y_max - y_min) / VOLUME_LOD0_SIZE) + 2;
		z_vols[VOLUME_LOD0] = (int)((z_max - z_min) / VOLUME_LOD0_SIZE) + 2;

		x_vols[VOLUME_LOD1] = (int)((x_max - x_min) / VOLUME_LOD1_SIZE) + 2;
		y_vols[VOLUME_LOD1] = (int)((y_max - y_min) / VOLUME_LOD1_SIZE) + 2;
		z_vols[VOLUME_LOD1] = (int)((z_max - z_min) / VOLUME_LOD1_SIZE) + 2;

		x_vols[VOLUME_LOD2] = (int)((x_max - x_min) / VOLUME_LOD2_SIZE) + 2;
		y_vols[VOLUME_LOD2] = (int)((y_max - y_min) / VOLUME_LOD2_SIZE) + 2;
		z_vols[VOLUME_LOD2] = (int)((z_max - z_min) / VOLUME_LOD2_SIZE) + 2;

		x_vols[VOLUME_LOD3] = (int)((x_max - x_min) / VOLUME_LOD3_SIZE) + 2;
		y_vols[VOLUME_LOD3] = (int)((y_max - y_min) / VOLUME_LOD3_SIZE) + 2;
		z_vols[VOLUME_LOD3] = (int)((z_max - z_min) / VOLUME_LOD3_SIZE) + 2;*/

		x_vols[VOLUME_LOD3] = (int)((x_max - x_min) / VOLUME_LOD3_SIZE) + 2;
		y_vols[VOLUME_LOD3] = (int)((y_max - y_min) / VOLUME_LOD3_SIZE) + 2;
		z_vols[VOLUME_LOD3] = (int)((z_max - z_min) / VOLUME_LOD3_SIZE) + 2;

		x_vols[VOLUME_LOD2] = x_vols[VOLUME_LOD3] * 2.0;
		y_vols[VOLUME_LOD2] = y_vols[VOLUME_LOD3] * 2.0;
		z_vols[VOLUME_LOD2] = z_vols[VOLUME_LOD3] * 2.0;

		x_vols[VOLUME_LOD1] = x_vols[VOLUME_LOD2] * 2.0;
		y_vols[VOLUME_LOD1] = y_vols[VOLUME_LOD2] * 2.0;
		z_vols[VOLUME_LOD1] = z_vols[VOLUME_LOD2] * 2.0;

		x_vols[VOLUME_LOD0] = x_vols[VOLUME_LOD1] * 2.0;
		y_vols[VOLUME_LOD0] = y_vols[VOLUME_LOD1] * 2.0;
		z_vols[VOLUME_LOD0] = z_vols[VOLUME_LOD1] * 2.0;


		//printf("%d %d %d\n", x_vols[VOLUME_LOD1], y_vols[VOLUME_LOD1], z_vols[VOLUME_LOD1]);

		center.x = (x_max + x_min) / 2.0;
		center.y = (y_max + y_min) / 2.0;
		center.z = (z_max + z_min) / 2.0;


		row_size = x_vols[VOLUME_LOD3];
		layer_size = row_size * y_vols[VOLUME_LOD3];

		lods[VOLUME_LOD3] = malloc(sizeof(light_volume_t) * x_vols[VOLUME_LOD3] * y_vols[VOLUME_LOD3] * z_vols[VOLUME_LOD3]);

		for(z = 0; z < z_vols[VOLUME_LOD3]; z++)
		{
			for(y = 0; y < y_vols[VOLUME_LOD3]; y++)
			{
				for(x = 0; x < x_vols[VOLUME_LOD3]; x++)
				{
					i = x + y * row_size + z * layer_size;
					lods[VOLUME_LOD3][i].lod = VOLUME_LOD3;
					lods[VOLUME_LOD3][i].accum = 0.0;
					lods[VOLUME_LOD3][i].frame = 0;
					lods[VOLUME_LOD3][i].next_lod = 0;
					lods[VOLUME_LOD3][i].origin.x = x_min + VOLUME_LOD3_SIZE * x;
					lods[VOLUME_LOD3][i].origin.y = y_min + VOLUME_LOD3_SIZE * y;
					lods[VOLUME_LOD3][i].origin.z = z_min + VOLUME_LOD3_SIZE * z;
				}
			}

		}





		row_size = x_vols[VOLUME_LOD2];
		layer_size = row_size * y_vols[VOLUME_LOD2];

		lods[VOLUME_LOD2] = malloc(sizeof(light_volume_t) * x_vols[VOLUME_LOD2] * y_vols[VOLUME_LOD2] * z_vols[VOLUME_LOD2]);

		affected_indexes = malloc(sizeof(int) * x_vols[VOLUME_LOD2] * y_vols[VOLUME_LOD2] * z_vols[VOLUME_LOD2] * 25);

		i = 0;

		for(z = 0; z < z_vols[VOLUME_LOD2]; z++)
		{
			for(y = 0; y < y_vols[VOLUME_LOD2]; y++)
			{
				for(x = 0; x < x_vols[VOLUME_LOD2]; x++)
				{

					i = x + y * row_size + z * layer_size;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].next_lod = 0;
					lods[VOLUME_LOD2][i].origin.x = x_min + VOLUME_LOD2_SIZE * x;
					lods[VOLUME_LOD2][i].origin.y = y_min + VOLUME_LOD2_SIZE * y;
					lods[VOLUME_LOD2][i].origin.z = z_min + VOLUME_LOD2_SIZE * z;

					/*center.x = x_min + VOLUME_LOD3_SIZE * x;
					center.y = y_min + VOLUME_LOD3_SIZE * y;
					center.z = z_min + VOLUME_LOD3_SIZE * z;

					c = x + y * row_size + z * layer_size;
					lods[VOLUME_LOD3][c].next_lod = i;


					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z - VOLUME_LOD2_SIZE * 0.5;

					i++;

					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z - VOLUME_LOD2_SIZE * 0.5;

					i++;

					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z - VOLUME_LOD2_SIZE * 0.5;

					i++;

					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z - VOLUME_LOD2_SIZE * 0.5;

					i++;




					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z + VOLUME_LOD2_SIZE * 0.5;

					i++;

					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z + VOLUME_LOD2_SIZE * 0.5;

					i++;

					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x - VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z + VOLUME_LOD2_SIZE * 0.5;

					i++;

					lods[VOLUME_LOD2][i].accum = 0.0;
					lods[VOLUME_LOD2][i].frame = 0;
					lods[VOLUME_LOD2][i].lod = VOLUME_LOD2;
					lods[VOLUME_LOD2][i].origin.x = center.x + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.y = center.y + VOLUME_LOD2_SIZE * 0.5;
					lods[VOLUME_LOD2][i].origin.z = center.z + VOLUME_LOD2_SIZE * 0.5;

					i++;*/

				}
			}

		}

	}

	#endif
}

void indirect_GetAffectedVolumes()
{
	#if 0
	int i;
	int c = light_count;

	int x;
	int y;
	int z;

	int j;

	float d;
	float radius;

	int row_size = x_vols[VOLUME_LOD2];
	int layer_size = row_size * y_vols[VOLUME_LOD2];

	affected_count = 0;

	vec3_t v;
	light_position_t *pos;

	float min_x;
	float min_y;
	float min_z;

	float max_x;
	float max_y;
	float max_z;

	float dx;
	float dy;
	float dz;


	min_x = lods[VOLUME_LOD2][0].origin.x;
	min_y = lods[VOLUME_LOD2][0].origin.y;
	min_z = lods[VOLUME_LOD2][0].origin.z;

	max_x = min_x + VOLUME_LOD2_SIZE * x_vols[VOLUME_LOD2];
	max_y = min_y + VOLUME_LOD2_SIZE * y_vols[VOLUME_LOD2];
	max_z = min_z + VOLUME_LOD2_SIZE * z_vols[VOLUME_LOD2];


	dx = (max_x - min_x);
	dy = (max_y - min_y);
	dz = (max_z - min_z);

	for(i = 0; i < c; i++)
	{
		pos = &light_positions[i];

		dx = (max_x - pos->position.x - VOLUME_LOD2_SIZE * 0.5) / (max_x - min_x);
		dy = (max_y - pos->position.y - VOLUME_LOD2_SIZE * 0.5) / (max_y - min_y);
		dz = (max_z - pos->position.z - VOLUME_LOD2_SIZE * 0.5) / (max_z - min_z);

		if(dx >= 0.0 && dx <= 1.0)
		{
			if(dy >= 0.0 && dy <= 1.0)
			{
				if(dz >= 0.0 && dz <= 1.0)
				{
					x = x_vols[VOLUME_LOD2] * (1.0 - dx);
					y = y_vols[VOLUME_LOD2] * (1.0 - dy);
					z = z_vols[VOLUME_LOD2] * (1.0 - dz);

					affected_indexes[affected_count++] = x + y * row_size + z * layer_size;
				}
			}
		}
	}
	#endif
}

int call_count = 0;

void indirect_Propagate()
{
	#if 0
	int i;
	int j;
	int l;
	int ni;


	int x;
	int y;
	int z;

	int nx;
	int ny;
	int nz;

	int k;

	vec3_t v;

	light_position_t *pos;
	float d;

	int row_size = x_vols[VOLUME_LOD2];
	int layer_size = row_size * y_vols[VOLUME_LOD2];




	int q[][3] =
				{
					1, 0, 0,
				   -1, 0, 0,

				    0, 1, 0,
				    0,-1, 0,

				    0, 0, 1,
				    0, 0,-1,

				    1, 1, 1,
				   -1,-1,-1,

				    1,-1, 1,
				   -1, 1,-1,




				};


	for(j = 0; j < affected_count; j++)
	{
		for(i = 0; i < light_count; i++)
		{
			pos = &light_positions[i];

			v.x = pos->position.x - lods[VOLUME_LOD2][affected_indexes[j]].origin.x;
			v.y = pos->position.y - lods[VOLUME_LOD2][affected_indexes[j]].origin.y;
			v.z = pos->position.z - lods[VOLUME_LOD2][affected_indexes[j]].origin.z;

			d = sqrt(dot3(v, v));

			lods[VOLUME_LOD2][affected_indexes[j]].accum = LIGHT_MAX_ENERGY * ((float)light_params[i].energy / 0xffff);
			lods[VOLUME_LOD2][affected_indexes[j]].frame = call_count;
		}
	}

	_do_again:

	k = affected_count;

	for(j = 0; j < affected_count; j++)
	{
		i = affected_indexes[j];
		l = i;

		z = l / layer_size;
		l -= z * layer_size;
		y = l / row_size;
		x = l - y * row_size;


		for(l = 0; l < 6; l++)
		{
			nx = x + q[l][0];
			ny = y + q[l][1];
			nz = z + q[l][2];


			if(nx >= 0 && nx < x_vols[VOLUME_LOD2])
			{
				if(ny >= 0 && ny < y_vols[VOLUME_LOD2])
				{
					if(nz >= 0 && nz < z_vols[VOLUME_LOD2])
					{
						ni = nx + ny * row_size + nz * layer_size;

						d = lods[VOLUME_LOD2][i].accum * 0.25;

						if(d < 0.01)
						{
							continue;
						}

						if(lods[VOLUME_LOD2][ni].frame != call_count)
						{
							lods[VOLUME_LOD2][ni].accum = 0.0;
							lods[VOLUME_LOD2][ni].frame = call_count;
							affected_indexes[k++] = ni;
						}

						if(d > lods[VOLUME_LOD2][ni].accum) lods[VOLUME_LOD2][ni].accum += d;



					}
				}
			}

		}

	}

	if(k != affected_count)
	{
		affected_count = k;

		goto _do_again;
	}


	affected_count = k;

	call_count++;

	#endif

}

void indirect_DrawVolumes()
{

	#if 0
	int x;
	int y;
	int z;

	int i;

	vec3_t origin;

	int row_size;
	int layer_size;


	if(! lods[VOLUME_LOD2])
		return;


	camera_t *active_camera = camera_GetActiveCamera();

	row_size = x_vols[VOLUME_LOD2];
	layer_size = row_size * y_vols[VOLUME_LOD2];


	indirect_GetAffectedVolumes();
	indirect_Propagate();


	glUseProgram(0);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&active_camera->world_to_camera_matrix.floats[0][0]);

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	//glColor3f(0.0, 1.0, 0.0);


	/*for(z = 0; z < z_vols[VOLUME_LOD1]; z++)
	{
		for(y = 0; y < y_vols[VOLUME_LOD1]; y++)
		{
			for(x = 0; x < x_vols[VOLUME_LOD1]; x++)
			{
				i = x + y * row_size + z * layer_size;

				origin = lods[VOLUME_LOD1][i].origin;

				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);

				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);


				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);

				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y - VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z + VOLUME_LOD1_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD1_SIZE * 0.5, origin.y + VOLUME_LOD1_SIZE * 0.5, origin.z - VOLUME_LOD1_SIZE * 0.5);

			}
		}
	}*/


			for(x = 0; x < affected_count; x++)
			{
				//i = x + y * row_size + z * layer_size;

				glColor3f(lods[VOLUME_LOD2][affected_indexes[x]].accum, lods[VOLUME_LOD2][affected_indexes[x]].accum, lods[VOLUME_LOD2][affected_indexes[x]].accum);

				origin = lods[VOLUME_LOD2][affected_indexes[x]].origin;

				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);

				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);


				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);

				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y - VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z + VOLUME_LOD2_SIZE * 0.45);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.45, origin.y + VOLUME_LOD2_SIZE * 0.45, origin.z - VOLUME_LOD2_SIZE * 0.45);

			}



	/*for(z = 0; z < z_vols[VOLUME_LOD2]; z++)
	{
		for(y = 0; y < y_vols[VOLUME_LOD2]; y++)
		{
			for(x = 0; x < x_vols[VOLUME_LOD2]; x++)
			{
				i = x + y * row_size + z * layer_size;

				origin = lods[VOLUME_LOD2][i].origin;

				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);

				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);


				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x - VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);

				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y - VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z + VOLUME_LOD2_SIZE * 0.5);
				glVertex3f(origin.x + VOLUME_LOD2_SIZE * 0.5, origin.y + VOLUME_LOD2_SIZE * 0.5, origin.z - VOLUME_LOD2_SIZE * 0.5);

			}
		}
	}*/


	glEnd();
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	#endif

}























