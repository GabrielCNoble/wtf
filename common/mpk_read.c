#include "mpk_read.h"
#include "material.h"
#include "c_memory.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int mpk_deserialize(void *file_buffer, struct output_params_t *out_params)
{
    //FILE *file;
	//int model;
	//batch_t *batches;
	//mesh_t *mesh;

	int i;
	int j;

	struct mpk_header_t *header;
	struct mpk_batch_t *batch;
	struct mpk_lod_t *lod;

	char *in;


	in = file_buffer;

	/* firstly, we read the header... */
	header = (struct mpk_header_t *)in;
	in += sizeof(struct mpk_header_t);

	if(!(header->vertice_count && header->batch_count && header->lod_count))
	{
		return 0;
	}

	out_params->vertices_count = header->vertice_count;
	out_params->batches_count = header->batch_count;
	out_params->lods_count = header->lod_count;
	out_params->indices_count = header->indice_count;

	/* then we read all the vertices accessed from all batches/lods... */
	out_params->vertices = memory_Malloc(sizeof(mpk_vertex_t) * header->vertice_count);
	out_params->indices = memory_Malloc(sizeof(int) * header->indice_count);
	out_params->batches = memory_Malloc(sizeof(struct mpk_batch_t) * header->batch_count * header->lod_count);
	out_params->lods = memory_Malloc(sizeof(struct mpk_lod_t) * header->lod_count);

	/*for(i = 0; i < header->vertice_count; i++)
	{
		out_params->out_vertices[i] = *(vertex_t *)in;
		in += sizeof(vertex_t);
	}*/

    /* read vertices... */
	memcpy(out_params->vertices, in, sizeof(mpk_vertex_t) * header->vertice_count);
	in += sizeof(mpk_vertex_t) * header->vertice_count;

    /* read indices... */
	memcpy(out_params->indices, in, sizeof(int) * header->indice_count);
    in += sizeof(int) * header->indice_count;

    /* read lods... */
    memcpy(out_params->lods, in, sizeof(struct mpk_lod_t) * header->lod_count);
    in += sizeof(struct mpk_lod_t) * header->lod_count;

    /* read batches... */
    memcpy(out_params->batches, in, sizeof(struct mpk_batch_t) * header->batch_count * header->lod_count);
    in += sizeof(struct mpk_batch_t) * header->batch_count * header->lod_count;

    /*for(j = 0; j < header->lod_count; j++)
    {
        out_params->lods[j].batches = out_params->batches + j * header->batch_count;
    }*/



    //out_params->out_lods_batches = memory_Malloc(sizeof(struct mpk_batch_t) * header->lod_count * header->batch_count);
    //out_params->out_lods_indices = memory_Malloc(sizeof(int) * header->lod_count);

    //for(i = 0; i < header->lod_count; i++)
	//{
		/* we read a lod... */
    //    lod = (struct mpk_lod_t *)in;
    //    in += sizeof(struct mpk_lod_t);

     //   out_params->lods[i].indice_count = lod->indice_count;
     //   out_params->lods[i].indice_start = lod->indice_start;

        /* and the indices it has... */
        //out_params->out_lods_indices[i] = memory_Malloc(sizeof(int) * lod->indice_count);
        //out_params->out_lods_batches[i] = memory_Malloc(sizeof(struct mpk_batch_t) * header->batch_count);

        /*for(j = 0; j < lod->indice_count; j++)
		{
			*(out_params->out_lods_indices[i] + j) = *(int *)in;
			in += sizeof(int);
		}*/

        //memcpy(out_params->indices + lod->indice_start, in, sizeof(int) * lod->indice_count);
        //in += sizeof(int) * lod->indice_count;

     //   out_params->lods[i].batches = out_params->batches + out_params->batches_count * i;

     //   memcpy(out_params->lods[i].batches, in, sizeof(struct mpk_batch_t) * out_params->batches_count);
     //   in += sizeof(struct mpk_batch_t) * out_params->batches_count;

		//for(j = 0; j < out_params->out_batches_count; j++)
		//{
			/* then the batches for this lod... */
        //    batch = (struct mpk_batch_t *)in;
		//	in += sizeof(struct mpk_batch_t);
		//	memcpy(out_params->out_lods_batches[i * out_params->out_batches_count + j], batch, sizeof(struct mpk_batch_t));
		//}
	//}


	return 1;
}

//int mpk_read(char *file_name, char *model_name)
int mpk_read(char *file_name, struct output_params_t *out_params)
{
	FILE *file;
	//int model;
	//batch_t *batches;
	//mesh_t *mesh;

	int i;
	int j;

	struct mpk_header_t *header;
	struct mpk_batch_t *batch;
	struct mpk_lod_t *lod;
	//struct mpk_vertex_record_t *out_records;
	//struct mpk_vertex_record_t *in_records;

//	vertex_t *in_vertices;
	//vertex_t *out_vertices;
//	char *material_name;

	unsigned int file_size;
	char *file_buffer;
	char *in;

	file = fopen(file_name, "rb");

	if(!file)
	{
		printf("couldn't open file %s!\n", file_name);
		return 0;
	}


	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);


	file_buffer = memory_Malloc(file_size);
	fread(file_buffer, file_size, 1, file);
	fclose(file);

	mpk_deserialize(file_buffer, out_params);

	#if 0

	in = file_buffer;

	/* firstly, we read the header... */
	header = (struct mpk_header_t *)in;
	in += sizeof(struct mpk_header_t);

	if(!(header->vertice_count && header->batch_count && header->lod_count))
	{
		free(file_buffer);
		return 0;
	}

	out_params->vertices_count = header->vertice_count;
	out_params->batches_count = header->batch_count;
	out_params->lods_count = header->lod_count;
	out_params->indices_count = header->indice_count;

	/* then we read all the vertices accessed from all batches/lods... */
	out_params->vertices = memory_Malloc(sizeof(vertex_t) * header->vertice_count);
	out_params->indices = memory_Malloc(sizeof(int) * header->indice_count);
	out_params->batches = memory_Malloc(sizeof(struct batch_t) * header->batch_count * header->lod_count);

	/*for(i = 0; i < header->vertice_count; i++)
	{
		out_params->out_vertices[i] = *(vertex_t *)in;
		in += sizeof(vertex_t);
	}*/

	memcpy(out_params->vertices, in, sizeof(vertex_t) * header->vertice_count);
	in += sizeof(vertex_t) * header->vertice_count;

	out_params->lods = memory_Malloc(sizeof(struct mpk_lod_t) * header->lod_count);


    //out_params->out_lods_batches = memory_Malloc(sizeof(struct mpk_batch_t) * header->lod_count * header->batch_count);
    //out_params->out_lods_indices = memory_Malloc(sizeof(int) * header->lod_count);

    for(i = 0; i < header->lod_count; i++)
	{
		/* we read a lod... */
        lod = (struct mpk_lod_t *)in;
        in += sizeof(struct mpk_lod_t);

        out_params->lods[i].indice_count = lod->indice_count;
        out_params->lods[i].indice_start = lod->indice_start;

        /* and the indices it has... */
        //out_params->out_lods_indices[i] = memory_Malloc(sizeof(int) * lod->indice_count);
        //out_params->out_lods_batches[i] = memory_Malloc(sizeof(struct mpk_batch_t) * header->batch_count);

        /*for(j = 0; j < lod->indice_count; j++)
		{
			*(out_params->out_lods_indices[i] + j) = *(int *)in;
			in += sizeof(int);
		}*/

        memcpy(out_params->indices + lod->indice_start, in, sizeof(int) * lod->indice_count);
        in += sizeof(int) * lod->indice_count;

        out_params->lods[i].batches = out_params->batches + out_params->batches_count * i;

        memcpy(out_params->lods[i].batches, in, sizeof(struct mpk_batch_t) * out_params->batches_count);
        in += sizeof(struct mpk_batch_t) * out_params->batches_count;

		//for(j = 0; j < out_params->out_batches_count; j++)
		//{
			/* then the batches for this lod... */
        //    batch = (struct mpk_batch_t *)in;
		//	in += sizeof(struct mpk_batch_t);
		//	memcpy(out_params->out_lods_batches[i * out_params->out_batches_count + j], batch, sizeof(struct mpk_batch_t));
		//}
	}

    #endif



	memory_Free(file_buffer);

	return 1;
}








