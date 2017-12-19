#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "GL\glew.h"

#include "mesh.h"
#include "gpu.h"
//#include "physics.h"


static int mesh_data_list_size;
static int mesh_data_count;
static mesh_t *mesh_data;
static int *alloc_handles;


/********************************/

typedef struct
{
	ivec3_t *indexes;
	int max_verts;
	int vert_count;
}face_indexes_t;


/********************************/

/*static int max_world_mesh_vertexes;
static int world_mesh_vertex_count;

static vertex_t *world_mesh_vertices;
static world_mesh_triangle_t *world_mesh;

static int world_mesh_handle;
int world_mesh_start;
int world_mesh_count;

int triangle_group_list_size;
int triangle_group_count;
triangle_group_t *triangle_groups;
int *triangle_group_buffer;

unsigned int index_buffer;*/

void mesh_Init()
{
	mesh_data_list_size = 64;
	mesh_data_count = 0;
	mesh_data = malloc(sizeof(mesh_t) * mesh_data_list_size);
	alloc_handles = malloc(sizeof(int) * mesh_data_list_size);
	
	/*max_world_mesh_vertexes = 16000 * 3;
	world_mesh_vertex_count = 0;
	world_mesh_vertices = malloc(sizeof(vertex_t) * max_world_mesh_vertexes);
	world_mesh = malloc(sizeof(world_mesh_triangle_t) * (max_world_mesh_vertexes / 3));
	world_mesh_handle = gpu_Alloc(sizeof(vertex_t) * max_world_mesh_vertexes);
	world_mesh_start = gpu_GetAllocStart(world_mesh_handle) / sizeof(vertex_t);
	world_mesh_count = 0;
	
	triangle_group_list_size = 64;
	triangle_group_count = 0;
	triangle_groups = malloc(sizeof(triangle_group_t) * triangle_group_list_size);
	triangle_group_buffer = malloc(sizeof(int) * triangle_group_list_size);
	
	glGenBuffers(1, &index_buffer);*/
}

void mesh_Finish()
{
	int i;
	for(i = 0; i < mesh_data_count; i++)
	{
		free(mesh_data[i].name);
		free(mesh_data[i].vertices);
	}
	free(mesh_data);
	free(alloc_handles);
	//free(world_mesh);
	//free(world_mesh_vertices);
	//free(triangle_groups);
	//free(triangle_group_buffer);
	
	//glDeleteBuffers(1, &index_buffer);
}

void mesh_LoadModel(char *file_name, char *model_name)
{
	FILE *f;
	int i;
	int c;
	int k;
	int total_vert_count = 0;
	int name_len;
	int mesh_index;
	unsigned long file_size;
	char *file_str;
	
	ivec3_t *iv3temp;
	vec3_t *v3temp;
	int *itemp;

	vec3_t *array;
	int *count;
	
	int max_verts;
	int vert_count;
	vec3_t *verts;
	
	int max_norms;
	int norm_count;
	vec3_t *norms;
	
	int max_tex_coords;
	int tex_coord_count;
	vec3_t *tex_coords;
	
	int max_face_indexes_data;
	int face_indexes_data_count;
	face_indexes_t *face_indexes;
	face_indexes_t *current_face_indexes;
	
	mesh_t *mesh;
	
	int val_str_index;
	char val_str[128];
	
	
	if(!(f = fopen(file_name, "r")))
	{
		printf("Couldn't open file %s\n", file_name);
	}
	
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);
	
	file_str = calloc(file_size + 4, 1);
	fread(file_str, 1, file_size, f);
	fclose(f);
	
	//file_str[file_size - 1] = '\0';
	i = 0;
	
	
	
	/* TODO: reallocations inside this function is causing crashes... find and fix! */
	max_verts = 320;
	vert_count = 0;
	verts = malloc(sizeof(vec3_t) * max_verts);
	
	max_norms = 320;
	norm_count = 0;
	norms = malloc(sizeof(vec3_t) * max_norms);
	
	max_tex_coords = 320;
	tex_coord_count = 0;
	tex_coords = malloc(sizeof(vec2_t) * max_tex_coords);
	
	max_face_indexes_data = 320;
	face_indexes_data_count = 0;
	face_indexes = malloc(sizeof(face_indexes_t) * max_face_indexes_data);
	
	while(i < file_size)
	{
		
	
		if(file_str[i] == 'v')
		{
			i++;
			
			if(file_str[i] == 'n')
			{
				/* normal... */	
				i++;
					
				if(norm_count >= max_norms)
				{
					v3temp = malloc(sizeof(vec3_t) * (max_norms + 320));
					memcpy(v3temp, norms, sizeof(vec3_t) * max_norms);
					free(norms);
					norms = v3temp;
					max_norms += 320;
				}
					
				k = 3;
				
				array = norms;
				count = &norm_count;
			}
			else if(file_str[i] == 't')
			{
				/* tex coord... */
					
				i++;
					
				if(tex_coord_count >= max_tex_coords)
				{
					v3temp = malloc(sizeof(vec3_t) * (max_tex_coords + 320));
					memcpy(v3temp, tex_coords, sizeof(vec3_t) * max_tex_coords);
					free(tex_coords);
					tex_coords = v3temp;
					max_tex_coords += 320;
				}
					
				k = 2;
					
				array = tex_coords;
				count = &tex_coord_count;
					
			}
			else
			{
				/* vertex... */
				i++;
					
				if(vert_count >= max_verts)
				{
					v3temp = malloc(sizeof(vec3_t) * (max_verts + 320));
					memcpy(v3temp, verts, sizeof(vec3_t) * max_verts);
					free(verts);
					verts = v3temp;
					max_verts += 320;
				}
					
				k = 3;
					
				array = verts;
				count = &vert_count;
			}
				
			for(c = 0; c < k; c++)
			{
				while(file_str[i] == ' ' || file_str[i] == '\n') i++;
					
				val_str_index = 0;
				while(file_str[i] != ' ' && file_str[i] != '\n' && file_str[i] != '\0')
				{
					val_str[val_str_index] = file_str[i];
					val_str_index++;
					i++;
				}
				val_str[val_str_index] = '\0';	
				array[*count].floats[c] = atof(val_str);
			}
				
			(*count)++;
		}
		else if(file_str[i] == 'f')
		{
			/* face data... */
			i++;
			
			
			if(face_indexes_data_count >= max_face_indexes_data)
			{
				current_face_indexes = malloc(sizeof(face_indexes_t) * (max_face_indexes_data + 320));
				memcpy(current_face_indexes, face_indexes, sizeof(face_indexes_t) * max_face_indexes_data);
				free(face_indexes);
				face_indexes = current_face_indexes;
				max_face_indexes_data += 320;
			}
			
			current_face_indexes = &face_indexes[face_indexes_data_count];
			current_face_indexes->max_verts = 320;
			current_face_indexes->vert_count = 0;
			current_face_indexes->indexes = malloc(sizeof(ivec3_t) * current_face_indexes->max_verts);
			
			face_indexes_data_count++;
			
			do
			{	
				for(c = 0; c < 3; c++)
				{		
					if(current_face_indexes->vert_count >= current_face_indexes->max_verts)
					{
						iv3temp = malloc(sizeof(ivec3_t) * (current_face_indexes->max_verts + 320));
						memcpy(iv3temp, current_face_indexes->indexes, sizeof(ivec3_t) * current_face_indexes->max_verts);
						free(current_face_indexes->indexes);
						current_face_indexes->indexes = iv3temp;
						current_face_indexes->max_verts += 320;
					}
					
					while(file_str[i] == ' ' || file_str[i] == '\n') i++;
						
					val_str_index = 0;
					while(file_str[i] != ' ' && file_str[i] != '\n' && file_str[i] != '\0' && file_str[i] != '/')
					{
						val_str[val_str_index] = file_str[i];
						val_str_index++;
						i++;
					}
					i++;
					if(!val_str_index)
					{
						current_face_indexes->indexes[current_face_indexes->vert_count].ints[c] = -1;
						while(file_str[i] == ' ') i++;
						//i++;
					}
					else
					{
						val_str[val_str_index] = '\0';	
						current_face_indexes->indexes[current_face_indexes->vert_count].ints[c] = atoi(val_str) - 1;
					}
					
				}
				current_face_indexes->vert_count++;
				
				total_vert_count++;
				
				while(file_str[i] == ' ' || file_str[i] == '\n') i++;
				
			}while(file_str[i] >= '0' && file_str[i] <= '9');
			
		}
		else if(file_str[i] == '#')
		{
			while(file_str[i] != '\n') i++;
		}
		else if(file_str[i] 	== 'm' &&
				file_str[i + 1] == 't' &&
				file_str[i + 2] == 'l' &&
				file_str[i + 3] == 'l' &&
				file_str[i + 4] == 'i' &&
				file_str[i + 5] == 'b')
		{
			i += 6;
			while(file_str[i] != '\n') i++;
		}
		else if(file_str[i]		== 'u' &&
				file_str[i + 1] == 's' &&
				file_str[i + 2] == 'e' && 
				file_str[i + 3] == 'm' &&
				file_str[i + 4] == 't' &&
				file_str[i + 5] == 'l')
		{
			i += 6;	
			while(file_str[i] == ' ') i++;
			
			if(file_str[i] == 'o' &&
			   file_str[i + 1] == 'f' && 
			   file_str[i + 2] == 'f')
			{
				i += 3;
			}
			else
			{
				while(file_str[i] != '\n') i++;
			}
			
		}
		else if(file_str[i] == 's')
		{
			while(file_str[i] != '\n') i++;
		}
		else
		{
			i++;	
		}
	}
	
	
	if(mesh_data_count >= mesh_data_list_size)
	{
		mesh = malloc(sizeof(mesh_t) * (mesh_data_list_size + 16));
		itemp = malloc(sizeof(int) * (mesh_data_list_size + 16));
		memcpy(mesh, mesh_data, sizeof(mesh_t) * mesh_data_list_size);
		memcpy(itemp, alloc_handles, sizeof(int) * mesh_data_list_size);
		free(mesh_data);
		free(alloc_handles);
		mesh = mesh_data;
		alloc_handles = itemp;
		mesh_data_list_size += 16;
	}
	
	mesh_index = mesh_data_count++;
	mesh = &mesh_data[mesh_index];
	mesh->vertices = malloc(sizeof(vertex_t) * total_vert_count);
	
	
	k = 0;
	
	for(i = 0; i < face_indexes_data_count; i++)
	{
		current_face_indexes = &face_indexes[i];
		for(c = 0; c < current_face_indexes->vert_count; c++)
		{
			mesh->vertices[k + c].position = verts[current_face_indexes->indexes[c].ints[0]];
			mesh->vertices[k + c].normal = norms[current_face_indexes->indexes[c].ints[2]];
			
			/*if(current_face_indexes->indexes[c].ints[1] > -1)
			{
				mesh->vertices[k + c].tex_coord.x = verts[current_face_indexes->indexes[c].ints[1]].x;
				mesh->vertices[k + c].tex_coord.y = verts[current_face_indexes->indexes[c].ints[1]].y;
			}*/
		}
		
		k += current_face_indexes->vert_count;
	}
	
	name_len = strlen(model_name) + 1;
	
	/* make strings a multiple of 4, so fast multibyte
	comparission can be used to find a mesh_t by
	name if needed... */
	name_len = (name_len + 3) & (~3);
	mesh->name = calloc(name_len, 1);
	strcpy(mesh->name, model_name);
	
	mesh->vert_count = total_vert_count;
	mesh->draw_mode = GL_TRIANGLES;
	
	alloc_handles[mesh_index] = gpu_Alloc(sizeof(vertex_t) * total_vert_count);
	mesh->start = gpu_GetAllocStart(alloc_handles[mesh_index]) / sizeof(vertex_t);
	gpu_Write(alloc_handles[mesh_index], 0, mesh->vertices, sizeof(vertex_t) * total_vert_count, 0);
	
	free(file_str);
	free(verts);
	free(norms);
	free(tex_coords);
	
	for(i = 0; i < face_indexes_data_count; i++)
	{
		free(face_indexes[i].indexes);
	}
	
	free(face_indexes);
}

mesh_t *mesh_GetModel(char *model_name)
{
	int i;
	
	for(i = 0; i < mesh_data_count; i++)
	{
		if(!strcmp(mesh_data[i].name, model_name))
		{
			return &mesh_data[i];
		}
	}
	
	return NULL;
}




















