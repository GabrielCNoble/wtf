#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

#define MAX_MATERIAL_NAME_LEN 64

typedef struct
{
	vec3_t position;
	vec3_t normal;
	vec3_t tangent;
	vec2_t tex_coord;
}vertex_t;

typedef struct
{
	ivec3_t *indexes;
	int max_verts;
	int vert_count;
	char *material_name;
}face_indexes_t;

typedef struct
{
	int first_vertex;										   
	int triangle_group_index;				/* references the triangle group this triangle is part of... */
}world_mesh_triangle_t;

typedef struct
{
	char *material_name;
	int vertex_count;
	vertex_t *vertex_data;
}triangle_group_t;	

typedef struct
{
	char *name;
	vec4_t diffuse_color;
	char *diffuse_texture;
	char *normal_texture;
}world_material_t;


void load_mtl(char *file_name, world_material_t **materials, int *material_count);

void load_obj(char *file_name, triangle_group_t **triangle_groups, int *triangle_group_count);

void export_world_mesh(char *file_name, triangle_group_t *triangle_groups, int triangle_group_count, world_material_t *materials, int material_count);

void import_world_mesh(char *file_name);


static char *texture_output_path = "textures/world/";

typedef struct
{
	int file_size;
	vec3_t *vertex_position;
	vec3_t *vertex_normal;
}fms_t;


int main(int argc, char *argv[])
{
	int i;
	char *input;
	//vertex_t *vertex_data;
	triangle_group_t *triangle_groups;
	world_material_t *materials;
	int triangle_group_count;
	int material_count;
	
	char obj_file[128];
	char mtl_file[128];
	
	
	
	if(argc > 1)
	{
		i = 0;
		input = argv[argc - 1];
		
		while(input[i] != '.' && input[i] != '\0') i++;
		input[i] = '\0';
		
		strcpy(obj_file, input);
		strcat(obj_file, ".obj");
		
		strcpy(mtl_file, input);
		strcat(mtl_file, ".mtl");
		
		load_mtl(mtl_file, &materials, &material_count);
		load_obj(obj_file, &triangle_groups, &triangle_group_count);
		export_world_mesh(input, triangle_groups, triangle_group_count, materials, material_count);
		
		if(triangle_groups)
		{
			for(i = 0; i < triangle_group_count; i++)
			{
				free(triangle_groups[i].material_name);
				free(triangle_groups[i].vertex_data);
			}
			
			free(triangle_groups);	
		}
		if(materials) free(materials);
		
		
		//import_world_mesh("weapon.fms");
	}
	
}


void load_mtl(char *file_name, world_material_t **materials, int *material_count)
{
	FILE *f;
	char *file_str;
	int i;
	int j;
	unsigned long long file_size;
	unsigned long long tex_file_size;
	
	*materials = NULL;
	*material_count = 0;
	
	char **c;
	
	int max_mats = 8;
	int mat_count = 0;
	world_material_t *mats;
	world_material_t *cur_mat;
	
	int val_str_index;
	char val_str[128];
	char tex_output_path[128];
	
	char *tex_file_data;
	
	if(!(f = fopen(file_name, "r")))
	{
		printf("couldn't open %s\n", file_name);
		return;
	}
	
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);
	
	file_str = calloc(file_size + 1, 1);
	fread(file_str, 1, file_size, f);
	fclose(f);
	
	mats = malloc(sizeof(world_material_t ) * max_mats);
	
	i = 0;
	
	while(file_str[i] != '\0')
	{
		if(file_str[i] == 'n' &&
		   file_str[i + 1] == 'e' &&
		   file_str[i + 2] == 'w' &&
		   file_str[i + 3] == 'm' &&
		   file_str[i + 4] == 't' &&
		   file_str[i + 5] == 'l')
		{
			i += 6;
			cur_mat = &mats[mat_count++];
			
			while(file_str[i] == ' ') i++;
			
			val_str_index = 0;
			while(file_str[i] != ' ' && file_str[i] != '\n' && file_str[i] != '\0')
			{
				val_str[val_str_index] = file_str[i];
				val_str_index++;
				i++;
			}
			
			val_str[val_str_index] = '\0';
			cur_mat->name = strdup(val_str);
			cur_mat->diffuse_texture = NULL;
			cur_mat->normal_texture = NULL;
		}
		else if(file_str[i] == 'K' &&
				file_str[i + 1] == 'd')
		{
			i += 2;
			
			for(j = 0; j < 3; j++)
			{
				while(file_str[i] == ' ') i++;
				
				val_str_index = 0;
				while(file_str[i] != ' ' && file_str[i] != '\n' && file_str[i] != '\0')
				{
					val_str[val_str_index] = file_str[i];
					val_str_index++;
					i++;
				}
				val_str[val_str_index] = '\0';
				
				cur_mat->diffuse_color.floats[j] = atof(val_str);
			}
			
			cur_mat->diffuse_color.a = 1.0;
		}
		else if(file_str[i] == 'm' &&
		        file_str[i + 1] == 'a' &&
				file_str[i + 2] == 'p' &&
				file_str[i + 3] == '_')
		{
			i += 4;
			
			if(file_str[i] == 'K' &&
			   file_str[i + 1] == 'd')
			{
				i += 2;
				c = &cur_mat->diffuse_texture;
			}
			else if(file_str[i] == 'B' &&
			        file_str[i + 1] == 'u' &&
					file_str[i + 2] == 'm' &&
					file_str[i + 3] == 'p')
			{
				i += 4;
				c = &cur_mat->normal_texture;
			}
			
			while(file_str[i] == ' ') i++;
				
			val_str_index = 0;
			while(file_str[i] != '\n' && file_str[i] != '\0')
			{
				val_str[val_str_index] = file_str[i];
				val_str_index++;
				i++;
			}
				
			val_str[val_str_index] = '\0';
				
			while(val_str[val_str_index] != '\\') val_str_index--;				
			val_str_index++;
				
			strcpy(tex_output_path, texture_output_path);
			strcat(tex_output_path, val_str + val_str_index);
			//strcpy(tex_output_path, val_str + val_str_index);
			
			*c = strdup(tex_output_path);
				
			if(f = fopen(tex_output_path, "rb"))
			{
				/* file already exists in the output folder... */
				fclose(f);
			}
			else
			{
				/* copy the texture file to the texture folder... */
				f = fopen(val_str, "rb");
					
				fseek(f, 0, SEEK_END);
				tex_file_size = ftell(f);
				rewind(f);
					
				tex_file_data = malloc(tex_file_size + 1);
				fread(tex_file_data, 1, tex_file_size, f);
				fclose(f);
					
				f = fopen(tex_output_path, "wb");
				fwrite(tex_file_data, 1, tex_file_size, f);
				fclose(f);
			}
				
		}
		else if(file_str[i] == '#')
		{
			i++;
			while(file_str[i] != '\n') i++;
		}
		else
		{
			i++;
		}
	}
	
	*materials = mats;
	*material_count = mat_count;
	
	free(file_str);
}



void load_obj(char *file_name, triangle_group_t **triangle_groups, int *triangle_group_count)
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
	
	vertex_t *vertices;
	
	triangle_group_t *tri_groups;
	
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
	
	int max_triangles;
	int triangle_count;
	world_mesh_triangle_t *triangles;
	
	//mesh_t *mesh;
	
	int val_str_index;
	char val_str[128];
	char usemtl[128] = {"None"};
	
	
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
	
	i = 0;
	
	
	
	/* TODO: reallocations inside this function are causing crashes... find and fix! */
	max_verts = 32000;
	vert_count = 0;
	verts = malloc(sizeof(vec3_t) * max_verts);
	
	max_norms = 32000;
	norm_count = 0;
	norms = malloc(sizeof(vec3_t) * max_norms);
	
	max_tex_coords = 32000;
	tex_coord_count = 0;
	tex_coords = malloc(sizeof(vec2_t) * max_tex_coords);
	
	max_face_indexes_data = 32000;
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
			current_face_indexes->material_name = strdup(usemtl);
			
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
					
					current_face_indexes->material_name = strdup(usemtl);
					
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
				strcpy(usemtl, "None");
			}
			else
			{
				val_str_index = 0;
				while(file_str[i] != ' ' && file_str[i] != '\n' && file_str[i] != '\0')
				{
					usemtl[val_str_index] = file_str[i];
					val_str_index++;
					i++;
				}
				
				usemtl[val_str_index] = '\0';
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
	
	
	//vertices = malloc(sizeof(vertex_t) * total_vert_count);
	
	
	/* clusterize all face data that share the same material... */
	for(i = 0; i < face_indexes_data_count; i++)
	{
		current_face_indexes = &face_indexes[i];
		
		for(c = i + 1; c < face_indexes_data_count; c++)
		{
			if(!strcmp(current_face_indexes->material_name, face_indexes[c].material_name))
			{
				
				iv3temp = malloc(sizeof(ivec3_t) * (current_face_indexes->vert_count + face_indexes[c].vert_count));
				
				for(k = 0; k < current_face_indexes->vert_count; k++)
				{
					iv3temp[k] = current_face_indexes->indexes[k];
				}
				
				for(k = 0; k < face_indexes[c].vert_count; k++)
				{
					iv3temp[k + current_face_indexes->vert_count] = face_indexes[c].indexes[k];
				}
				
				free(current_face_indexes->indexes);
				free(face_indexes[c].indexes);
				free(face_indexes[c].material_name);
				
				current_face_indexes->vert_count += face_indexes[c].vert_count;
				current_face_indexes->indexes = iv3temp;
				
				
				if(c < face_indexes_data_count - 1)
				{
					face_indexes[c] = face_indexes[face_indexes_data_count - 1];
					c--;
				}
				
				face_indexes_data_count--;
				
			}
		}
	}
	
	k = 0;
	
	tri_groups = malloc(sizeof(triangle_group_t) * face_indexes_data_count);
	
	for(i = 0; i < face_indexes_data_count; i++)
	{
		current_face_indexes = &face_indexes[i];
		
		tri_groups[i].vertex_data = malloc(sizeof(vertex_t) * current_face_indexes->vert_count);
		tri_groups[i].material_name = current_face_indexes->material_name;
		tri_groups[i].vertex_count = current_face_indexes->vert_count;
		
		for(c = 0; c < current_face_indexes->vert_count; c++)
		{
			
			tri_groups[i].vertex_data[c].position = verts[current_face_indexes->indexes[c].ints[0]];
			tri_groups[i].vertex_data[c].normal = norms[current_face_indexes->indexes[c].ints[2]];
						
			if(current_face_indexes->indexes[c].ints[1] > -1)
			{
				tri_groups[i].vertex_data[c].tex_coord.x = tex_coords[current_face_indexes->indexes[c].ints[1]].x;
				tri_groups[i].vertex_data[c].tex_coord.y = tex_coords[current_face_indexes->indexes[c].ints[1]].y;
			}
		}
		
		k += current_face_indexes->vert_count;
	}
	
	//*vertex_data = vertices;
	//*vertex_count = total_vert_count;
	
	*triangle_groups = tri_groups;
	*triangle_group_count = face_indexes_data_count;
	
	free(file_str);
	free(verts);
	free(norms);
	free(tex_coords);
	
	for(i = 0; i < face_indexes_data_count; i++)
	{
		free(face_indexes[i].indexes);
		//free(face_indexes[i].material_name);			/* tri_groups take ownership of those pointers... */
	}
	
	free(face_indexes);
}

void export_world_mesh(char *file_name, triangle_group_t *triangle_groups, int triangle_group_count, world_material_t *materials, int material_count)
{
	int i;
	int j;
	int vertex_count;
	int material_name_len;
	FILE *f;
	char *c = file_name;
	char output_name[512];
	char material_name[MAX_MATERIAL_NAME_LEN] = "test_material";
	i = 0;
	
	int buffer_index;
	//material_count = 1;
	int group_index = 0;
	char *buffer;
	
	vec4_t material_color;
	int bm_material_flags;
	
	if(!material_count)
	{
		printf("%s contain no materials! Nothing exported!\n", file_name);
		return;
	}
	
	if(!triangle_group_count)
	{
		printf("%s contain no vertex data! Nothing exported!\n", file_name);
		return;
	}
	
	
	strcpy(output_name, file_name);
	strcat(output_name, ".fms");
	
	f = fopen(output_name, "wb");
	
	vertex_count = 0;
	
	for(i = 0; i < triangle_group_count; i++)
	{
		vertex_count += triangle_groups[i].vertex_count;
	}
	
	
	fwrite(&vertex_count, sizeof(int), 1, f);				/* how many vertices in this file */
	fwrite(&material_count, sizeof(int), 1, f);				/* how many materials in this file */
	
	
	for(i = 0; i < triangle_group_count; i++)
	{
		/* put the material data in the same order they appeared on the obj file... */
		for(j = 0; j < material_count; j++)
		{
			if(!strcmp(triangle_groups[i].material_name, materials[j].name)) break;
		}
		
		strcpy(material_name, materials[j].name);
		fwrite(material_name, MAX_MATERIAL_NAME_LEN, 1, f);	
		bm_material_flags = 0;
		if(materials[j].diffuse_texture)
		{
			bm_material_flags |= 1;
		}
		
		if(materials[j].normal_texture)
		{
			bm_material_flags |= 2;
		}
			
		fwrite(&bm_material_flags, sizeof(int), 1, f);
		fwrite(&materials[j].diffuse_color, sizeof(vec4_t), 1, f);	
		
		if(bm_material_flags & 1)
		{
			strcpy(material_name, materials[j].diffuse_texture);
			fwrite(material_name, MAX_MATERIAL_NAME_LEN, 1, f);
		}
		
		if(bm_material_flags & 2)
		{
			strcpy(material_name, materials[j].normal_texture);
			fwrite(material_name, MAX_MATERIAL_NAME_LEN, 1, f);
		}
	}
	
	/* group 0 uses material 0, group 1 uses material 1, group 2 uses material 2 ... */
	for(j = 0; j < triangle_group_count; j++)
	{
		fwrite(&j, sizeof(int), 1, f);
		fwrite(&triangle_groups[j].vertex_count, sizeof(int), 1, f);
		
		for(i = 0; i < triangle_groups[j].vertex_count; i++)
		{
			fwrite(&triangle_groups[j].vertex_data[i].position, sizeof(vec3_t), 1, f);	
		}
	}
	
	for(j = 0; j < triangle_group_count; j++)
	{
		for(i = 0; i < triangle_groups[j].vertex_count; i++)
		{
			fwrite(&triangle_groups[j].vertex_data[i].normal, sizeof(vec3_t), 1, f);	
		}
	}
	
	for(j = 0; j < triangle_group_count; j++)
	{
		for(i = 0; i < triangle_groups[j].vertex_count; i++)
		{
			fwrite(&triangle_groups[j].vertex_data[i].tex_coord, sizeof(vec2_t), 1, f);	
		}
	}
		
	fclose(f);	
}

/*void import_world_mesh(char *file_name)
{
	int i;
	int vert_count;
	FILE *f;
	
	vec3_t *vertexes;
	
	if(!(f = fopen(file_name, "rb")))
	{
		printf("couldn't open %s\n", file_name);
		return;
	}
	
	fread(&vert_count, sizeof(int), 1, f);
	
	printf("%d\n", vert_count);
	
	vertexes = malloc(sizeof(vec3_t) * vert_count);
	fread(vertexes, sizeof(vec3_t), vert_count, f);
	

	for(i = 0; i < vert_count; i++)
	{
		printf("[%f %f %f]\n", vertexes[i].x, vertexes[i].y, vertexes[i].z);
	}
	
	fclose(f);
	free(vertexes);
	
}*/










