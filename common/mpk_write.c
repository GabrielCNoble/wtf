#include "mpk_write.h"
#include "vector.h"
#include "model.h"
#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct
{
	int vertex_index;
	int tex_coord_index;
	int normal_index;
	int used;
	char *material_name;
}face_index_t;




void calculate_tangents(vertex_t *vertices, int vertice_count)
{
	int i;
	int count = vertice_count;
	
	vec3_t a;
	vec3_t b;
	vec3_t c;
	vec3_t ab;
	vec3_t ac;
	vec3_t t;
	vec3_t bt;
	vec3_t t1;
	vec3_t bt1;
	
	vec2_t duv1;
	vec2_t duv2;
	
	
	
	float x;
	float y;
	float z;
	float w;
	
	float q;
	
	if(vertice_count < 3)
	{
		return;
	}
	
	//*tangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	//*bitangent_data = (float *)calloc(vert_count, sizeof(float) * 3);
	
	for(i = 0; i < count;)
	{
		duv1.x = vertices[i + 1].tex_coord.x - vertices[i].tex_coord.x;
		duv1.y = vertices[i + 1].tex_coord.y - vertices[i].tex_coord.y;
		
		duv2.x = vertices[i + 2].tex_coord.x - vertices[i].tex_coord.x;
		duv2.y = vertices[i + 2].tex_coord.y - vertices[i].tex_coord.y;
		
		//q = 1.0 / (duv1.x * duv2.y - duv1.y * duv2.x);
		
		q = (duv1.x * duv2.y - duv1.y * duv2.x);
		
		a.x = vertices[i].position.x;
		a.y = vertices[i].position.y;
		a.z = vertices[i].position.z;
		
		b.x = vertices[i + 1].position.x;
		b.y = vertices[i + 1].position.y;
		b.z = vertices[i + 1].position.z;
		
		c.x = vertices[i + 2].position.x;
		c.y = vertices[i + 2].position.y;
		c.z = vertices[i + 2].position.z;
		
		ab = sub3(b, a);
		ac = sub3(c, a);
		
		t1.floats[0] = (ab.floats[0] * duv2.floats[1] - ac.floats[0] * duv1.floats[1]) / q;
		t1.floats[1] = (ab.floats[1] * duv2.floats[1] - ac.floats[1] * duv1.floats[1]) / q;
		
		t = gs_orthg(vertices[i].normal, t1);
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;

		i++;
		
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;
		
		i++;
		
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;
		
		i++;
	}
	return;
}


void load_obj(FILE *file, vertex_t **out_vertices, int *out_vert_count, material_record_t **out_material_records, int *out_material_records_count, mpk_vertex_record_t **out_vertex_records, int *out_vertex_record_count)
{
	int i;
	int j;
	int c;
	int cursor;
	
	FILE *mtl;
	
	unsigned int position_count = 0;
	unsigned int normal_count = 0;
	unsigned int tex_coord_count = 0;
	unsigned int vertice_count = 0;
	unsigned int face_index_count = 0;
	unsigned int material_refs = 0;
	unsigned int mtllib_count = 0;
	
	char *file_buffer;
	char *current_material;
	char t;
	unsigned long file_size;
	
	vertex_t *vertices = NULL;
	
	int vertex_records_cursor = 0;
	mpk_vertex_record_t *vertex_records = NULL;
	mpk_vertex_record_t *vertex_record = NULL;
	
	int positions_cursor = 0;
	vec3_t *positions = NULL;
	
	int normals_cursor = 0;
	vec3_t *normals = NULL;
	
	int tex_coords_cursor = 0;
	vec2_t *tex_coords = NULL;
	
	int face_index_cursor = 0;
	face_index_t *face_indexes = NULL;
	
	int referenced_material_cursor = 0;
	char **referenced_materials = NULL;
	
	int mtllib_cursor = 0;
	char **mtllibs = NULL;
	
	int material_records_cursor = 0;
	material_record_t *material_records = NULL;
	material_record_t *material_record = NULL;
	
	int texture_record_cursor = 0;
	texture_record_t *texture_records = NULL;
	
	int value_str_cursor = 0;
	char value_str[1024];
	
	vec3_t v;
	
	float q;
	
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	file_buffer = calloc(file_size + 128, 1);
	fread(file_buffer, 1, file_size, file);
	
	fclose(file);
	
	/*================================================================================================*/
	/*================================================================================================*/
	/*================================================================================================*/
	
	cursor = 0;
	
	while(file_buffer[cursor] != '\0')
	{
		switch(file_buffer[cursor])
		{
			case 'v':
				cursor++;
				
				if(file_buffer[cursor] == 'n')
				{
					cursor++;
					normal_count++;
				}
				else if(file_buffer[cursor] == 't')
				{
					cursor++;
					tex_coord_count++;
				}
				else
				{
					position_count++;
				}
			break;
			
			case 'f':
				cursor++;
				while(file_buffer[cursor] == ' ') cursor++;
				
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
				{
					while(file_buffer[cursor] == ' ') cursor++;
					
					while(file_buffer[cursor] != '/')cursor++;
					cursor++;
					while(file_buffer[cursor] != '/')cursor++;
					cursor++;
					while(file_buffer[cursor] != ' ' && file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')cursor++;
					face_index_count++;
					vertice_count++;
				}
				
				face_index_count++;			/* necessary to add invalid indexes in between valid face indexes... */
			break;
			
			case 'm':
				if(file_buffer[cursor + 1] == 't' &&
				   file_buffer[cursor + 2] == 'l' &&
				   file_buffer[cursor + 3] == 'l' &&
				   file_buffer[cursor + 4] == 'i' &&
				   file_buffer[cursor + 5] == 'b')
				{
					cursor += 6;
					while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					mtllib_count++;
				}
			break;
			
			case 'u':
				if(file_buffer[cursor + 1] == 's' &&
				   file_buffer[cursor + 2] == 'e' &&
				   file_buffer[cursor + 3] == 'm' &&
				   file_buffer[cursor + 4] == 't' &&
				   file_buffer[cursor + 5] == 'l')
				{
					cursor += 6;
					while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					material_refs++;
				}
			break;
			
			case 's':
				cursor++;
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
			break;
			
			case '#':
				cursor++;
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
			break;
			
			case 'o':
				cursor++;
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
			break;
		}
		
		cursor++;	
	}
	
	/*================================================================================================*/
	/*================================================================================================*/
	/*================================================================================================*/
	
	positions = malloc(sizeof(vec3_t) * position_count);
	normals = malloc(sizeof(vec3_t) * normal_count);
	
	if(tex_coord_count)
	{
		tex_coords = malloc(sizeof(vec3_t) * tex_coord_count);
	}
	
	face_indexes = malloc(sizeof(face_index_t) * face_index_count);
	
	if(material_refs)
	{
		referenced_materials = malloc(sizeof(char *) * material_refs);
	}
	
	if(mtllib_count)
	{
		mtllibs = malloc(sizeof(char *) * mtllib_count);
	}

	cursor = 0;
	current_material = NULL;
	while(file_buffer[cursor] != '\0')
	{
		switch(file_buffer[cursor])
		{
			case 'v':
				cursor++;
				
				if(file_buffer[cursor] == 'n')
				{
					t = 'n';
					cursor++;
					c = 3;
				}
				else if(file_buffer[cursor] == 't')
				{
					t = 't';
					cursor++;
					c = 2;
				}
				else
				{
					t = '\0';
					c = 3;
				}
				
				
				for(i = 0; i < c; i++)
				{
					while(file_buffer[cursor] == ' ') cursor++;
					value_str_cursor = 0;
					while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != ' ')
					{
						value_str[value_str_cursor] = file_buffer[cursor];
						value_str_cursor++;
						cursor++;
					}
					value_str[value_str_cursor] = '\0';
					v.floats[i] = atof(value_str);
				}
				
				switch(t)
				{
					case 'n':
						normals[normals_cursor] = v;
						normals_cursor++;
					break;
					
					case 't':
						tex_coords[tex_coords_cursor] = v.vec2;
						tex_coords_cursor++;
					break;
					
					case '\0':
						positions[positions_cursor] = v;
						positions_cursor++;
					break;
				}
				
			break;
			
			case 'f':
				cursor++;
				while(file_buffer[cursor] == ' ') cursor++;
				
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
				{
					
					while(file_buffer[cursor] == ' ') cursor++;
					
					
					/* vertice index... */
					value_str_cursor = 0;
					while(file_buffer[cursor] != '/')
					{
						value_str[value_str_cursor] = file_buffer[cursor];
						value_str_cursor++;
						cursor++;
					}
					cursor++;
					value_str[value_str_cursor] = '\0';
					
					face_indexes[face_index_cursor].vertex_index = atoi(value_str) - 1;
					
					
					/* tex coord index (if any)... */
					value_str_cursor = 0;
					while(file_buffer[cursor] != '/')
					{
						value_str[value_str_cursor] = file_buffer[cursor];
						value_str_cursor++;
						cursor++;
					}
					if(value_str_cursor)
					{
						value_str[value_str_cursor] = '\0';
						face_indexes[face_index_cursor].tex_coord_index = atoi(value_str) - 1;
					}
					else
					{
						face_indexes[face_index_cursor].tex_coord_index = -1;
					}
					cursor++;
					
					value_str_cursor = 0;
					while(file_buffer[cursor] != ' ' && file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
					{
						value_str[value_str_cursor] = file_buffer[cursor];
						value_str_cursor++;
						cursor++;
					}
					
					value_str[value_str_cursor] = '\0';
					face_indexes[face_index_cursor].normal_index = atoi(value_str) - 1;
					face_indexes[face_index_cursor].material_name = current_material;
					face_indexes[face_index_cursor].used = 0;
					face_index_cursor++;
				}
				
				/* negative indexes flag the end of a face... */
				face_indexes[face_index_cursor].vertex_index = -1;
				face_indexes[face_index_cursor].tex_coord_index = -1;
				face_indexes[face_index_cursor].normal_index = -1;
				face_index_cursor++;
			break;
			
			case 'm':
				if(file_buffer[cursor + 1] == 't' &&
				   file_buffer[cursor + 2] == 'l' &&
				   file_buffer[cursor + 3] == 'l' &&
				   file_buffer[cursor + 4] == 'i' &&
				   file_buffer[cursor + 5] == 'b')
				{
					cursor += 6;
					while(file_buffer[cursor] == ' ') cursor++;
					
					value_str_cursor = 0;
					while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
					{
						value_str[value_str_cursor] = file_buffer[cursor];
						value_str_cursor++;
						cursor++;
					}
					value_str[value_str_cursor] = '\0';
					
					/* check to see if we didn't find this mtllib inside the file already... */
					for(i = 0; i < mtllib_cursor; i++)
					{
						if(!strcmp(value_str, mtllibs[i]))
						{
							break;
						}
					}
					
					/* nope, so append it to the list... */
					if(i >= mtllib_cursor)
					{
						mtllibs[mtllib_cursor] = strdup(value_str);
						mtllib_cursor++;
					}
				}
			break;
			
			case 'u':
				if(file_buffer[cursor + 1] == 's' &&
				   file_buffer[cursor + 2] == 'e' &&
				   file_buffer[cursor + 3] == 'm' &&
				   file_buffer[cursor + 4] == 't' &&
				   file_buffer[cursor + 5] == 'l')
				{
					cursor += 6;
					while(file_buffer[cursor] == ' ') cursor++;
					
					value_str_cursor = 0;
					while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
					{
						value_str[value_str_cursor] = file_buffer[cursor];
						value_str_cursor++;
						cursor++;
					}
					value_str[value_str_cursor] = '\0';
					
					/* check to see if we didn't reference this material before... */
					for(i = 0; i < referenced_material_cursor; i++)
					{
						if(!strcmp(value_str, referenced_materials[i]))
						{
							break;
						}
					}
					
					/* nope, so add this reference to the list... */
					if(i >= referenced_material_cursor)
					{
						referenced_materials[referenced_material_cursor] = strdup(value_str);
						referenced_material_cursor++;
					}
					
					/* every face declared in the file
					will use this material as source
					from now on... */	
					current_material = referenced_materials[i];
				}
			break;
			
			case 's':
				cursor++;
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0') cursor++;
			break;
			
			case '#':
				cursor++;
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0') cursor++;
			break;
			
			case 'o':
				cursor++;
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0') cursor++;
			break;
		}
		
		cursor++;	
	}
	
	
	/*================================================================================================*/
	/*================================================================================================*/
	/*================================================================================================*/
	
	
	free(file_buffer);
	
	vertices = malloc(sizeof(vertex_t) * vertice_count);
	
	material_records = malloc(referenced_material_cursor * sizeof(material_record_t));
	material_records_cursor = -1;
	
	file_buffer = NULL;
	
	/* try to read all the mtllibs found in the file... */
	for(i = 0; i < mtllib_cursor; i++)
	{
		mtl = fopen(mtllibs[i], "rb");
		
		if(!mtl)
		{
			printf("couldn't find mtl file [%s]!\n", mtllibs[i]);
			continue;
		}
		
		fseek(mtl, 0, SEEK_END);
		file_size = ftell(mtl);
		rewind(mtl);
		
		if(file_buffer)
		{
			free(file_buffer);
		}
		
		file_buffer = malloc(file_size);
		fread(file_buffer, file_size, 1, mtl);
		fclose(mtl);
		
		
		cursor = 0;
		
		
		while(file_buffer[cursor] != '\0')
		{
			switch(file_buffer[cursor])
			{
				
				case 'n':
					cursor++;
					
					if(file_buffer[cursor] == 'e' &&
					   file_buffer[cursor + 1] == 'w' &&
					   file_buffer[cursor + 2] == 'm' &&
					   file_buffer[cursor + 3] == 't' &&
					   file_buffer[cursor + 4] == 'l')
					{
						
						material_records_cursor++;
						
						cursor += 5;
						
						while(file_buffer[cursor] == ' ')cursor++;
						
						value_str_cursor = 0;
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != ' ' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
						{
							value_str[value_str_cursor] = file_buffer[cursor];
							value_str_cursor++;
							cursor++;
						}
						
						value_str[value_str_cursor] = '\0';
			
						strcpy(material_records[material_records_cursor].separate_names.material_name, value_str);
						
						for(i = 0; i < PATH_MAX; i++)
						{
							material_records[material_records_cursor].separate_names.diffuse_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.normal_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.height_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.roughness_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.metalness_texture_name[i] = '\0';
						}
					}
				break;
				
				case 'K':
					cursor++;
					if(file_buffer[cursor] == 'a')
					{
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					}
					else if(file_buffer[cursor] == 'd')
					{
						cursor++;
						
						for(i = 0; i < 3; i++)
						{
							while(file_buffer[cursor] == ' ') cursor++;
						
							value_str_cursor = 0;
							while(file_buffer[cursor] != ' ' && file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
							{
								value_str[value_str_cursor] = file_buffer[cursor];
								value_str_cursor++;
								cursor++;
							}
							value_str[value_str_cursor] = '\0';
							
							material_records[material_records_cursor].base.floats[i] = atof(value_str);
							
							if(file_buffer[cursor] == '\n' || file_buffer[cursor] == '\r' || file_buffer[cursor] == '\0')
							{
								/* just the red component was specified, so propagate its value to the other components... */
								if(!i)
								{
									material_records[material_records_cursor].base.g = material_records[material_records_cursor].base.r;
									material_records[material_records_cursor].base.b = material_records[material_records_cursor].base.r;
									break;
								}
							}
						}
						

					}
					else if(file_buffer[cursor] == 's')
					{
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					}
				break;
				
				case 'T':
					cursor++;
					
					if(file_buffer[cursor] == 'f')
					{
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					}
				break;
				
				case 'i':
					cursor++;
					
					if(file_buffer[cursor] == 'l' &&
					   file_buffer[cursor + 1] == 'l' &&
					   file_buffer[cursor + 2] == 'u' &&
					   file_buffer[cursor + 3] == 'm')
					{
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					}
				break;
				
				case 'd':
					cursor++;
					while(file_buffer[cursor] == ' ') cursor++;
					
					value_str_cursor = 0;
					while(file_buffer[cursor] != ' ' && file_buffer[cursor] != '\n' && file_buffer[cursor] != '\r' && file_buffer[cursor] != '\0')
					{
						value_str[value_str_cursor] = file_buffer[cursor];
						
						value_str_cursor++;
						cursor++;
					}
					
					
					value_str[value_str_cursor] = '\0';
					q = atof(value_str);
					material_records[material_records_cursor].base.a = q;
					
				break;
				
				case 'N':
					cursor++;
					
					if(file_buffer[cursor] == 's')
					{
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					}
					else if(file_buffer[cursor] == 'i')
					{
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					}
				break;
				
				case 's':
					cursor++;
					
					if(file_buffer[cursor] == 'h' &&
					   file_buffer[cursor + 1] == 'a' &&
					   file_buffer[cursor + 2] == 'r' && 
					   file_buffer[cursor + 3] == 'p' &&
					   file_buffer[cursor + 4] == 'n' &&
					   file_buffer[cursor + 5] == 'e' &&
					   file_buffer[cursor + 6] == 's' &&
					   file_buffer[cursor + 7] == 's')
					{
						while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
					}
				break;
				
				case 'm':
					cursor++;
					
					if(file_buffer[cursor] == 'a' &&
					   file_buffer[cursor + 1] == 'p' &&
					   file_buffer[cursor + 2] == '_')
					{
						cursor += 3;
						
						if(file_buffer[cursor] == 'K' &&
						   file_buffer[cursor + 1] == 'd')
						{
							cursor += 2;
							
							while(file_buffer[cursor] == ' ') cursor++;
							
							value_str_cursor = 0;
							
							while(file_buffer[cursor] != ' ' && file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
							{
								value_str[value_str_cursor] = file_buffer[cursor];
								
								if(value_str[value_str_cursor] == '\\')
								{
									value_str[value_str_cursor] = '/';
								}
								
								value_str_cursor++;
								cursor++;
								
							}
							
							value_str[value_str_cursor] = '\0';
							strcpy(material_records[material_records_cursor].separate_names.diffuse_texture_name, value_str);	
						}
						else if(file_buffer[cursor] == 'K' &&
						        file_buffer[cursor + 1] == 'a')
						{
							while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
						}
						else if(file_buffer[cursor] == 'K' &&
						        file_buffer[cursor + 1] == 's')
						{
							while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
						}
						else if(file_buffer[cursor] == 'B' &&
								file_buffer[cursor + 1] == 'u' &&
								file_buffer[cursor + 2] == 'm' &&
								file_buffer[cursor + 3] == 'p')
						{
							cursor += 4;
							
							while(file_buffer[cursor] == ' ') cursor++;
							
							value_str_cursor = 0;
							
							while(file_buffer[cursor] != ' ' && file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
							{
								value_str[value_str_cursor] = file_buffer[cursor];
								
								if(value_str[value_str_cursor] == '\\')
								{
									value_str[value_str_cursor] = '/';
								}
								
								value_str_cursor++;
								cursor++;
								
							}
							
							value_str[value_str_cursor] = '\0';		
							strcpy(material_records[material_records_cursor].separate_names.normal_texture_name, value_str);	
						}
						
					}   
				break;
			}
			
			cursor++;
		}
		
	}

	if(file_buffer)
	{
		free(file_buffer);
	}
	
	material_records_cursor++;
	
	//if(material_records_cursor > -1)
	//{
		/* material_records_cursors represent the current
		material to receive stuff from the file in the 
		previous loop. So if there's a single material,
		for instance, material_records_cursors == 0. To
		use its value as how many materials are there,
		we increment it here...*/
	//	material_records_cursor++;
	//}
	//else
//	{
		/* no materials read... */
//		material_records_cursor = 0;
//	}
	
	
	/*================================================================================================*/
	/*================================================================================================*/
	/*================================================================================================*/
	
	vertex_records = malloc(sizeof(mpk_vertex_record_t) * (material_records_cursor + 1));
	vertex_records_cursor = 0;
	
	for(j = 0; j < material_records_cursor; j++)
	{
		
		material_record = &material_records[j];
		
		vertex_record = &vertex_records[vertex_records_cursor];
		
		vertex_record->material_name[0] = '\0';
		vertex_record->offset = 0;
		vertex_record->vertice_count = 0;
		
		if(vertex_records_cursor)
		{
			vertex_record->offset = vertex_records[vertex_records_cursor - 1].offset + vertex_records[vertex_records_cursor - 1].offset;
		}
		
		vertex_records_cursor++;
		
		for(i = 0; i < face_index_cursor; i++)
		{
			if(face_indexes[i].vertex_index > -1)
			{
				if(!strcmp(face_indexes[i].material_name, material_records[j].separate_names.material_name))
				{
					vertices[vertex_record->offset + vertex_record->vertice_count].position = positions[face_indexes[i].vertex_index];
					vertices[vertex_record->offset + vertex_record->vertice_count].normal = normals[face_indexes[i].normal_index];
					
					if(face_indexes[i].tex_coord_index > -1)
					{
						vertices[vertex_record->offset + vertex_record->vertice_count].tex_coord = tex_coords[face_indexes[i].tex_coord_index];
					}
					else
					{
						vertices[vertex_record->offset + vertex_record->vertice_count].tex_coord.x = 0.0;
						vertices[vertex_record->offset + vertex_record->vertice_count].tex_coord.y = 0.0;
					}
					
					face_indexes[i].used = 1;
					
					vertex_record->vertice_count++;
					strcpy(vertex_record->material_name, face_indexes[i].material_name);			
				}
			}
		}
		
		/* no vertices referenced this material... */
		if(!vertex_record->vertice_count)
		{
			/* ...so get rid of the vertex record... */
			vertex_records_cursor--;
			
			/* ...and of the material... */
			if(j < material_records_cursor - 1)
			{
				c = material_records_cursor - 1;
				strcpy(material_records[j].separate_names.material_name, material_records[c].separate_names.material_name);
				strcpy(material_records[j].separate_names.diffuse_texture_name , material_records[c].separate_names.diffuse_texture_name);
				strcpy(material_records[j].separate_names.normal_texture_name, material_records[c].separate_names.normal_texture_name);
				strcpy(material_records[j].separate_names.height_texture_name, material_records[c].separate_names.height_texture_name);
				strcpy(material_records[j].separate_names.roughness_texture_name, material_records[c].separate_names.roughness_texture_name);
				strcpy(material_records[j].separate_names.metalness_texture_name, material_records[c].separate_names.metalness_texture_name);
				
				
				material_records[j].base = material_records[c].base;
				material_records[j].roughness = material_records[c].roughness;
				material_records[j].metalness = material_records[c].metalness;
				material_records[j].bm_flags = material_records[c].bm_flags; 
			}
			
			material_records_cursor--;
			j--;
		}
	}
	
	vertex_record = &vertex_records[vertex_records_cursor];
	strcpy(vertex_record->material_name, "default");
	vertex_record->offset = 0;
	vertex_record->vertice_count = 0;
		
	if(vertex_records_cursor)
	{
		vertex_record->offset = vertex_records[vertex_records_cursor - 1].offset + vertex_records[vertex_records_cursor - 1].vertice_count; 
	}
		
	vertex_records_cursor++;
	
	/* go over all the vertices and assign the not
	used ones to the default material. Those are
	the indigent vertices... */
	for(i = 0; i < face_index_cursor; i++)
	{
		if(face_indexes[i].vertex_index > -1)
		{
			if(!face_indexes[i].used)
			{
				vertices[vertex_record->offset + vertex_record->vertice_count].position = positions[face_indexes[i].vertex_index];
				vertices[vertex_record->offset + vertex_record->vertice_count].normal = normals[face_indexes[i].normal_index];
				
				if(face_indexes[i].tex_coord_index > -1)
				{
					vertices[vertex_record->offset + vertex_record->vertice_count].tex_coord = tex_coords[face_indexes[i].tex_coord_index];
				}
				else
				{
					vertices[vertex_record->offset + vertex_record->vertice_count].tex_coord.x = 0.0;
					vertices[vertex_record->offset + vertex_record->vertice_count].tex_coord.y = 0.0;
				}
				
				vertex_record->vertice_count++;
			}
		}
	}
	
	if(!vertex_record->vertice_count)
	{
		vertex_records_cursor--;
	}
	
	
	if(vertice_count)
	{
		calculate_tangents(vertices, vertice_count);
		
		*out_vertex_records = vertex_records;
		*out_vertex_record_count = vertex_records_cursor;
		
		*out_material_records = material_records;
		*out_material_records_count = material_records_cursor;
		
		*out_vertices = vertices;
		*out_vert_count = vertice_count;
	}
}

void mpk_write(char *output_name, vertex_t *vertices, int vertice_count, mpk_vertex_record_t *vertex_records, int vertex_record_count)
{
	FILE *file;
	
	mpk_header_t *header;
	mpk_vertex_record_t *record;
	vertex_t *verts;
	
	int i;
	
	unsigned int file_size;
	char *file_buffer;
	char *out;
	
	if(vertice_count)
	{
		file_size = sizeof(mpk_header_t) + sizeof(mpk_vertex_record_t) * vertex_record_count + sizeof(vertex_t) * vertice_count;
		file_buffer = calloc(file_size, 1);
		
		out = file_buffer;
		
		header = (mpk_header_t *)out;
		out += sizeof(mpk_header_t);
		
		header->mpk0 = MPK_CONSTANT0;
		header->mpk1 = MPK_CONSTANT1;
		
		header->reserved0 = 0;
		header->reserved1 = 0;
		header->reserved2 = 0;
		header->reserved3 = 0;
		header->reserved4 = 0;
		header->reserved5 = 0;
		header->reserved6 = 0;
		header->reserved7 = 0;
		
		header->version = MPK_VERSION;
		header->vertice_count = vertice_count;
		header->vertex_record_count = vertex_record_count;
		
		for(i = 0; i < vertex_record_count; i++)
		{
			record = (mpk_vertex_record_t *)out;
			out += sizeof(mpk_vertex_record_t);
			
			strcpy(record->material_name, vertex_records[i].material_name);
			
			record->offset = vertex_records[i].offset;
			record->vertice_count = vertex_records[i].vertice_count;
		}
		
		verts = (vertex_t *)out;
		out += sizeof(vertex_t) * vertice_count;
		
		for(i = 0; i < vertice_count; i++)
		{
			verts[i] = vertices[i];
		}
		
		file = fopen(output_name, "wb");
		
		fwrite(file_buffer, file_size, 1, file);
		fclose(file);
	}
}

void mpk_convert(char *output_name, char *input_file)
{
	
	FILE *file;
	
	int i;
	
	vertex_t *vertices;
	int vertice_count;
	
	material_record_t *material_records;
	int material_record_count;
	
	mpk_vertex_record_t *vertex_records;
	int vertex_record_count;
	
	mpk_header_t *header;
	mpk_vertex_record_t *record;
	vertex_t *verts;
	
	unsigned int file_size;
	char *file_buffer;
	char *out;
	
	file = fopen(input_file, "rb");
	
	if(!file)
	{
		printf("couldn't open file %s!\n", input_file);
		return;
	}
	
	load_obj(file, &vertices, &vertice_count, &material_records, &material_record_count, &vertex_records, &vertex_record_count);
	
	if(vertice_count)
	{
		mpk_write(output_name, vertices, vertice_count, vertex_records, vertex_record_count);
		
		free(material_records);
		free(vertices);
		free(vertex_records);
	}
}






