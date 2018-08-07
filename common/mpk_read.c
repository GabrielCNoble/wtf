#include "mpk_read.h"
#include "material.h"
#include "c_memory.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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


	file_buffer = memory_Malloc(file_size, "read_mpk");
	fread(file_buffer, file_size, 1, file);
	fclose(file);

	in = file_buffer;

	/* firstly, we read the header... */
	header = (struct mpk_header_t *)in;
	in += sizeof(struct mpk_header_t);

	if(!(header->vertice_count && header->batch_count && header->lod_count))
	{
		free(file_buffer);
		return 0;
	}

	out_params->out_vertices_count = header->vertice_count;
	out_params->out_batches_count = header->batch_count;
	out_params->out_lods_count = header->lod_count;
	out_params->out_indices_count = header->indice_count;

	/* then we read all the vertices accessed from all batches/lods... */
	out_params->out_vertices = memory_Malloc(sizeof(vertex_t) * header->vertice_count, "mpk_read");
	
	for(i = 0; i < header->vertice_count; i++)
	{
		out_params->out_vertices[i] = *(vertex_t *)in;
		in += sizeof(vertex_t);
	}
	
	//memcpy(out_params->out_vertices, in, sizeof(vertex_t) * header->vertice_count);
	//in += sizeof(vertex_t) * header->vertice_count;


	out_params->out_lods = memory_Malloc(sizeof(struct mpk_lod_t) * header->lod_count, "mpk_read");
    out_params->out_lods_batches = memory_Malloc(sizeof(struct mpk_batch_t) * header->lod_count * header->batch_count, "mpk_read");
    out_params->out_lods_indices = memory_Malloc(sizeof(int) * header->lod_count, "mpk_read");

    for(i = 0; i < header->lod_count; i++)
	{
		/* we read a lod... */
        lod = (struct mpk_lod_t *)in;
        in += sizeof(struct mpk_lod_t);

        out_params->out_lods[i].indice_count = lod->indice_count;

        /* and the indices it has... */
        out_params->out_lods_indices[i] = memory_Malloc(sizeof(int) * lod->indice_count, "mpk_read");
        out_params->out_lods_batches[i] = memory_Malloc(sizeof(struct mpk_batch_t) * header->batch_count, "mpk_read");
        
        for(j = 0; j < lod->indice_count; j++)
		{
			*(out_params->out_lods_indices[i] + j) = *(int *)in;
			in += sizeof(int);
		}
        
        //memcpy(out_params->out_lods_indices[i], in, sizeof(int) * lod->indice_count);
        //in += sizeof(int) * lod->indice_count;

		for(j = 0; j < out_params->out_batches_count; j++)
		{
			/* then the batches for this lod... */
            batch = (struct mpk_batch_t *)in;
			in += sizeof(struct mpk_batch_t);
			memcpy(out_params->out_lods_batches[i * out_params->out_batches_count + j], batch, sizeof(struct mpk_batch_t));
		}
	}
	



	memory_Free(file_buffer);

	return 1;
}








