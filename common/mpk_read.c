#include "mpk_read.h"
#include "material.h"
#include "memory.h"
#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//int mpk_read(char *file_name, char *model_name)
int mpk_read(char *file_name, mpk_vertex_record_t **vertex_records, int *vertex_record_count, vertex_t **vertices, int *vertice_count)
{
	FILE *file;
	//int model;
	//batch_t *batches;
	//mesh_t *mesh;
	
	int i;
	
	mpk_header_t *header;
	mpk_vertex_record_t *out_records;
	mpk_vertex_record_t *in_records;
	vertex_t *in_vertices;
	vertex_t *out_vertices;
	char *material_name;
	
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
	
	
	file_buffer = malloc(file_size);
	fread(file_buffer, file_size, 1, file);
	fclose(file);
	
	in = file_buffer;
	
	header = (mpk_header_t *)in;
	in += sizeof(mpk_header_t);
	
	if(!(header->vertice_count && header->vertex_record_count))
	{
		free(file_buffer);
		return 0;
	}
	
	in_records = (mpk_vertex_record_t *)in;
	in += sizeof(mpk_vertex_record_t) * header->vertex_record_count;
	
	out_records = memory_Malloc(sizeof(mpk_vertex_record_t) * header->vertex_record_count, "mpk_read");
	
	for(i = 0; i < header->vertex_record_count; i++)
	{
		out_records[i] = in_records[i];
	}
	
	out_vertices = memory_Malloc(sizeof(vertex_t) * header->vertice_count, "mpk_read");
	
	in_vertices = (vertex_t *)in;
	
	for(i = 0; i < header->vertice_count; i++)
	{
		out_vertices[i] = in_vertices[i];
	}
	
	*vertex_records = out_records;
	*vertex_record_count = header->vertex_record_count;
	
	*vertices = out_vertices;
	*vertice_count = header->vertice_count;
	
	free(file_buffer);
	
	return 1;

}








