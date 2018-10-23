#include "r_clustered_forward.h"
#include "r_common.h"
#include "r_shader.h"
#include "shd_common.h"

extern int r_width;
extern int r_height;
//extern int r_clusters_per_row;
//extern int r_cluster_rows;
//extern int r_cluster_layers;



int r_clusters_per_row = 0;
int r_cluster_rows = 0;
int r_cluster_layers = 0;
cluster_t *r_clusters = NULL;
unsigned int r_cluster_texture;

int r_clustered_forward_pass_shader = -1;


void renderer_ClusteredForwardSetup()
{
    static int initalized = 0;

    int clusters_per_row;
    int cluster_rows;

    if(!initialized)
    {
        initialized = 1;

        clusters_per_row = (int)(ceil((float)r_width / (float)R_CLUSTER_WIDTH));
        cluster_rows = (int)(ceil((float)r_height / (float)R_CLUSTER_HEIGHT));

        if(clusters_per_row != r_clusters_per_row || cluster_rows != r_clusters_per_row)
        {
            r_clusters_per_row = clusters_per_row;
            r_cluster_rows = cluster_rows;
            r_cluster_layers = CLUSTER_LAYERS;

            if(r_clusters)
            {
                memory_Free(r_clusters);
            }

            r_clusters = memory_Calloc(r_clusters_per_row * r_cluster_rows * r_cluster_layers, sizeof(cluster_t));

            if(r_cluster_texture)
            {
                glDeleteTextures(1, &r_cluster_texture);
            }

            glGenTextures(1, &r_cluster_texture);
            glBindTexture(GL_TEXTURE_3D, r_cluster_texture);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, r_clusters_per_row, r_cluster_rows, r_cluster_layers, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
            glBindTexture(GL_TEXTURE_3D, 0);
        }
    }

    return;
}



void renderer_DrawWorldClustererdForward()
{
    #if 0
    //camera_t *active_camera = camera_GetActiveCamera();
	view_def_t *active_view = renderer_GetActiveView();
	//triangle_group_t *triangle_group;
	struct batch_t *batch;
	int i;
	int c;

	float s;
	float e;

	glDepthFunc(GL_EQUAL);


	glViewport(0, 0, r_width, r_height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glDepthMask(GL_FALSE);

	c = w_world_batch_count;

	if(r_flat)
	{
		renderer_SetShader(r_flat_pass_shader);
	}
	else
	{
		//if(r_draw_shadow_maps)
		//{
			renderer_SetShader(r_clustered_forward_pass_shader);
		//	renderer_SetShadowTexture();
		//}
		//else
		//{
		//	renderer_SetShader(r_forward_pass_no_shadow_shader);
		//}

		renderer_SetClusterTexture();


		renderer_SetDefaultUniform1i(UNIFORM_r_width, r_width);
		renderer_SetDefaultUniform1i(UNIFORM_r_height, r_height);
		renderer_SetDefaultUniform1i(UNIFORM_r_frame, r_frame);
	}

	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->position);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->normal);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->tangent);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, sizeof(struct compact_vertex_t), &((struct compact_vertex_t *)0)->tex_coord);

	renderer_SetProjectionMatrix(&active_view->view_data.projection_matrix);
	renderer_SetViewMatrix(&active_view->view_data.view_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();

	for(i = 0; i < c; i++)
	{
		batch = &w_world_batches[i];
		renderer_SetMaterial(batch->material_index);
		renderer_DrawElements(GL_TRIANGLES, batch->next, GL_UNSIGNED_INT, (void *)((batch->start + w_world_index_start) * sizeof(int)));
	}

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	#endif
}

void renderer_DrawOpaqueClusteredForward()
{

}

void renderer_SetClusterTexture()
{
    glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, r_cluster_texture);
	renderer_SetDefaultUniform1i(UNIFORM_cluster_texture, 3);
	renderer_SetDefaultUniform1i(UNIFORM_r_clusters_per_row, r_clusters_per_row);
	renderer_SetDefaultUniform1i(UNIFORM_r_cluster_rows, r_cluster_rows);
	renderer_SetDefaultUniform1i(UNIFORM_r_cluster_layers, r_cluster_layers);
}

void renderer_VisibleLightsOnClusters()
{

}



