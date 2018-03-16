#define NO_GL

#include "mcvtr.h"


int main(int argc, char *argv[]) 
{
	if(argc < 2)
	{
		printf("*** No input files ***\n");
		return 1;
	}
	
	//convert("bus_stop2.obj");
	//load("bus_stop2.mpk");
	
	//convert("Cargo_container_01.obj");
	//load("Cargo_container_01.mpk");
	
	//convert("multi_cube.obj");
	//load("multi_cube.mpk");
	convert(argv[1]);
	
	//convert("portal_gun4.obj");
}



void calculate_tangents(vertex_t *vertices, int vertice_count)
{
	int i;
	int count = vertice_count;
	//float *tangent_data = NULL;
	
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
		
		q = 1.0 / (duv1.x * duv2.y - duv1.y * duv2.x);
		
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
		
		t1.floats[0] = (ab.floats[0] * duv2.floats[1] - ac.floats[0] * duv1.floats[1])*q;
		t1.floats[1] = (ab.floats[1] * duv2.floats[1] - ac.floats[1] * duv1.floats[1])*q;
		
		t = gs_orthg(vertices[i].normal, t1);
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;

		i++;
		
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;
		
		i++;
		
		//t = gs_orthg(vertices[i].normal, t1);
		vertices[i].tangent.x = t.x;
		vertices[i].tangent.y = t.y;
		vertices[i].tangent.z = t.z;
		
		i++;

		
	}
	return;
}



int convert(char *file_name)
{
	FILE *file;
	
	static unsigned long long file_size;
	char *file_buffer;
	int file_path_len;
	char save_path[512];
	
	int value_str_cursor = 0;
	char value_str[512];
	int cursor;	
	
	vec3_t *v_ptr;
	vec2_t *v2_ptr;
	int *cursor_ptr;
	
	int position_count = 0;
	int position_cursor = 0;
	vec3_t *positions;
	vec3_t v;

	int normal_count = 0;
	int normal_cursor = 0;
	vec3_t *normals;
	
	int tex_coord_count = 0;
	int tex_coord_cursor = 0;
	vec2_t *tex_coords;
	
	int face_index_count = 0;
	int face_index_cursor = 0;
	face_index_t *face_indexes;
	
	int material_refs = 0;
	int referenced_material_cursor = 0;
	char **referenced_materials = NULL;
	char *current_material;
	
	char **mtllibs = NULL;
	int mtllib_count = 0;
	int mtllib_cursor = 0;
	
	
	unsigned int mpk_file_size = 0;
	unsigned int mpk_file_buffer_cursor = 0;
	char *mpk_file_buffer = NULL;
	
	int vertice_count = 0;
	vertex_t *vertices = NULL;
	vertex_t *vertice;
	
	int texture_record_cursor = 0;
	extended_texture_record_t *texture_records = NULL;
	
	
	int material_records_cursor;
	extended_material_record_t *material_records = NULL;
	material_record_t *material_record;
	mpk_vertex_record_t *vertex_record;
	texture_record_t *texture_record;
	mpk_header_t *header;
	
	int i;
	int j;
	int c;
	
	float q;
	
	int material_record_offset = 0;
	
	//mtllib_ref_t *mtl_refs = NULL;	
	//mtllib_ref_t *ref = NULL;
	
	i = strlen(file_name);
	
	while(file_name[i] != '.' && i > 0) i--;
	
	if(strcmp(file_name + i, ".obj"))
	{
		printf("*** Incorrect input file! File must be a Wavefront (.obj) ***\n");
		return 1;
	}
	
	
	if(!(file = fopen(file_name, "rb")))
	{
		printf("*** Couldn't open file [%s]! ***\n", file_name);
		return 1;
	}
	
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	file_buffer = calloc(file_size + 128, 1);
	fread(file_buffer, 1, file_size, file);
	
	fclose(file);
	
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
	
	
	//material_records = calloc(mtllib_count, sizeof(extended_material_record_t));
	
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
					cursor++;
					v_ptr = normals;
					cursor_ptr = &normal_cursor;
					c = 3;
				}
				else if(file_buffer[cursor] == 't')
				{
					cursor++;
					v2_ptr = tex_coords;
					cursor_ptr = &tex_coord_cursor;
					c = 2;
				}
				else
				{
					v_ptr = positions;
					cursor_ptr = &position_cursor;
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
				
				if(c == 2)
				{
					/*v2_ptr[*cursor_ptr].x = v.x;
					v2_ptr[*cursor_ptr].y = v.y;*/
					
					tex_coords[tex_coord_cursor].x = v.x;
					tex_coords[tex_coord_cursor].y = v.y;
					
					tex_coord_cursor++;
					
				}
				else
				{
					v_ptr[*cursor_ptr] = v;
					(*cursor_ptr)++;
				}
				
					
				
			break;
			
			case 'f':
				cursor++;
				while(file_buffer[cursor] == ' ') cursor++;
				
				while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
				{
					
					while(file_buffer[cursor] == ' ') cursor++;
					
					
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
					
					for(i = 0; i < mtllib_cursor; i++)
					{
						if(!strcmp(value_str, mtllibs[i]))
						{
							break;
						}
					}
					
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
					
					
					for(i = 0; i < referenced_material_cursor; i++)
					{
						if(!strcmp(value_str, referenced_materials[i]))
						{
							break;
						}
					}
					
					if(i >= referenced_material_cursor)
					{
						referenced_materials[referenced_material_cursor] = strdup(value_str);
						referenced_material_cursor++;
					}
						
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
	
	free(file_buffer);
	
	
	material_records = malloc(referenced_material_cursor * sizeof(extended_material_record_t));
	material_records_cursor = -1;
	
	/* each material referenced can reference two textures... */
	texture_records = malloc(sizeof(extended_texture_record_t ) * referenced_material_cursor * 2);
	
	
	/* try to read all the mtllibs found in the file,
	and put them into extended_material_record_t's... */
	for(i = 0; i < mtllib_cursor; i++)
	{
		file = fopen(mtllibs[i], "rb");
		
		if(!file)
		{
			printf("couldn't find mtl file [%s]!\n", mtllibs[i]);
			continue;
		}
		
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		rewind(file);
		
		file_buffer = malloc(file_size * 5);
		fread(file_buffer, file_size, 1, file);
		fclose(file);
		
		
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
			
						strcpy(material_records[material_records_cursor].material_full_name, value_str);
						
						for(j = 0; j < MPK_MAX_NAME_LEN; j++)
						{
							material_records[material_records_cursor].diffuse_texture[j] = '\0';
							material_records[material_records_cursor].normal_texture[j] = '\0';
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
							
							material_records[material_records_cursor].record.base.floats[i] = atof(value_str);
							
							if(file_buffer[cursor] == '\n' || file_buffer[cursor] == '\r' || file_buffer[cursor] == '\0')
							{
								/* just the red component was specified, so propagate its value to the other components... */
								if(!i)
								{
									material_records[material_records_cursor].record.base.g = material_records[material_records_cursor].record.base.r;
									material_records[material_records_cursor].record.base.b = material_records[material_records_cursor].record.base.r;
									break;
								}
							}
						}
						
						//material_records[material_records_cursor].record.base.a = 1.0;
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
					material_records[material_records_cursor].record.base.a = q;
					
					//while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r') cursor++;
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
							
							strcpy(material_records[material_records_cursor].diffuse_texture, value_str);	
							
							for(i = 0; i < texture_record_cursor; i++)
							{
								if(!strcmp(texture_records[i].texture_name, value_str))
								{
									break;
								}
							}
							
							/* add this texture to the record list if it wasn't
							added yet... */
							if(i >= texture_record_cursor)
							{
								strcpy(texture_records[i].texture_name, value_str);
								texture_records[i].record.bm_texture_flags = 0;
								texture_record_cursor++;
							}
								
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
							
							strcpy(material_records[material_records_cursor].normal_texture, value_str);	
							
							for(i = 0; i < texture_record_cursor; i++)
							{
								if(!strcmp(texture_records[i].texture_name, value_str))
								{
									break;
								}
							}
							
							/* add this texture to the record list if it wasn't
							added yet... */
							if(i >= texture_record_cursor)
							{
								strcpy(texture_records[i].texture_name, value_str);
								texture_records[i].record.bm_texture_flags = 0;
								texture_record_cursor++;
							}
						}
						
					}   
				break;
			}
			
			cursor++;
		}
		
	}
	
	if(material_records_cursor > -1)
	{
		material_records_cursor++;
	}
	else
	{
		material_records_cursor = 0;
	}
	
																										/* number of materials found + 1 extra for indigent vertices (if any)...*/
	mpk_file_size = sizeof(mpk_header_t) + sizeof(extended_material_record_t) * material_records_cursor + sizeof(mpk_vertex_record_t) * (material_records_cursor + 1) + sizeof(extended_texture_record_t) * texture_record_cursor + sizeof(vertex_t) * vertice_count;
	mpk_file_buffer = calloc(mpk_file_size, 1);
	
	header = (mpk_header_t *)mpk_file_buffer;
	header->mpk0 = MPK_CONSTANT0;
	header->mpk1 = MPK_CONSTANT1;
	header->version = MPK_VERSION;
	header->vertice_count = vertice_count;
	header->material_count = material_records_cursor;
	header->texture_count = texture_record_cursor;
	header->vertex_record_count = 0;
	
	mpk_file_buffer_cursor += sizeof(mpk_header_t);
		
	
	for(i = 0; i < texture_record_cursor; i++)
	{
		texture_record = (texture_record_t *)(mpk_file_buffer + mpk_file_buffer_cursor);
		mpk_file_buffer_cursor += sizeof(texture_record_t);
		
		texture_record->bm_texture_flags = texture_records[i].record.bm_texture_flags;
		strcpy(mpk_file_buffer + mpk_file_buffer_cursor, texture_records[i].texture_name);
		j = strlen(texture_records[i].texture_name);
		mpk_file_buffer_cursor += j + 1;
		
		while((texture_records[i].texture_name[j] != '\\' || texture_records[i].texture_name[j] != '/') && j > 0) j--;
		
		if(j)
			j++;
		
		strcpy(mpk_file_buffer + mpk_file_buffer_cursor, texture_records[i].texture_name + j);	
		mpk_file_buffer_cursor += strlen(texture_records[i].texture_name + j) + 1;
	}
	
	
	
	/* save the materials found... */
	for(i = 0; i < material_records_cursor; i++)
	{
		material_record = (material_record_t *)(mpk_file_buffer + mpk_file_buffer_cursor);
		material_record->bm_flags = 0;
		material_record->base = material_records[i].record.base;
		strcpy(material_record->name, material_records[i].material_full_name);
		mpk_file_buffer_cursor += sizeof(material_record_t) + strlen(material_record->name) + 1;
		
		if(material_records[i].diffuse_texture[0])
		{
			material_record->bm_flags |= MATERIAL_USE_DIFFUSE_TEXTURE;
		}
		
		if(material_records[i].normal_texture[0])
		{
			material_record->bm_flags |= MATERIAL_USE_NORMAL_TEXTURE;
		}
		
		if(material_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
		{
			strcpy(mpk_file_buffer + mpk_file_buffer_cursor, material_records[i].diffuse_texture);
			mpk_file_buffer_cursor += strlen(material_records[i].diffuse_texture) + 1;
		}
		
		if(material_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
		{
			strcpy(mpk_file_buffer + mpk_file_buffer_cursor, material_records[i].normal_texture);
			mpk_file_buffer_cursor += strlen(material_records[i].normal_texture) + 1;
		}
		
		
		vertex_record = (mpk_vertex_record_t *)(mpk_file_buffer + mpk_file_buffer_cursor);
		vertex_record->material_index = i;
		vertex_record->vertice_count = 0;
		
		header->vertex_record_count++;
		
		mpk_file_buffer_cursor += sizeof(mpk_vertex_record_t);
		
		for(j = 0; j < face_index_cursor; j++)
		{
			if(face_indexes[j].vertex_index > -1)
			{
				/* test to see if this vertice is referencing the current material */
				if(!strcmp(material_records[i].material_full_name, face_indexes[j].material_name))
				{
					vertex_record->vertice_count++;
					vertice = (vertex_t *)(mpk_file_buffer + mpk_file_buffer_cursor);
					
					vertice->position = positions[face_indexes[j].vertex_index];
					vertice->normal = normals[face_indexes[j].normal_index];
					
					if(face_indexes[j].tex_coord_index > -1)
					{
						vertice->tex_coord = tex_coords[face_indexes[j].tex_coord_index];
					}
					else
					{
						vertice->tex_coord.x = 0.0;
						vertice->tex_coord.y = 0.0;
					}
					
					mpk_file_buffer_cursor += sizeof(vertex_t); 
					
					/* mark the indexes used to allow finding indigent vertices later... */
					face_indexes[j].used = 1;
				}
			}
		}
		
		
		/* although found, this material wasn't 
		referenced by any vertice, so drop it
		the record... */
		if(!vertex_record->vertice_count)
		{
			mpk_file_buffer_cursor -= sizeof(mpk_vertex_record_t);
			header->vertex_record_count--;
		}
		else
		{
			vertice = (vertex_t *)((char *)vertex_record + sizeof(mpk_vertex_record_t));
			calculate_tangents(vertice, vertex_record->vertice_count);
		}
	}
	
	/* vertex_record_t for indigent vertices (which doesn't reference any material found...) */
	vertex_record = (mpk_vertex_record_t *)(mpk_file_buffer + mpk_file_buffer_cursor);
	vertex_record->material_index = -1;					
	vertex_record->vertice_count = 0;
	mpk_file_buffer_cursor += sizeof(mpk_vertex_record_t);
	
	for(i = 0; i < face_index_cursor; i++)
	{
		if(face_indexes[i].vertex_index > -1)
		{
			if(!face_indexes[i].used)
			{
				vertex_record->vertice_count++;
				vertice = (vertex_t *)(mpk_file_buffer + mpk_file_buffer_cursor);
					
				vertice->position = positions[face_indexes[i].vertex_index];
				vertice->normal = normals[face_indexes[i].normal_index];
					
				if(face_indexes[i].tex_coord_index > -1)
				{
					vertice->tex_coord = tex_coords[face_indexes[i].tex_coord_index];
				}
					
				mpk_file_buffer_cursor += sizeof(vertex_t); 
					
			}
		}
	}
	
	/* if there are indigent vertices, keep this record... */
	if(vertex_record->vertice_count)
	{
		header->vertex_record_count++;
		vertice = (vertex_t *)((char *)vertex_record + sizeof(mpk_vertex_record_t));
		//vertice -= sizeof(vertex_t) * vertex_record->vertice_count;
		calculate_tangents(vertice, vertex_record->vertice_count);
	}
	
	printf("vertices: %d\nvertice records: %d\nmaterials: %d\ntextures: %d\nindigent vertices: ", header->vertice_count, header->vertex_record_count, header->material_count, header->texture_count);
	
	if(header->vertex_record_count != header->material_count)
	{
		printf("yes\n");
	}
	else
	{
		printf("no\n");
	}
	
	
	strcpy(save_path, file_name);
	
	i = strlen(save_path);
	
	while(save_path[i] != '.' && i > 0) i--;
	save_path[i] = '\0';
	strcat(save_path, ".mpk");
	
	
	
	file = fopen(save_path, "wb");
	fwrite(mpk_file_buffer, mpk_file_buffer_cursor, 1, file);
	fflush(file);
	fclose(file);
	
		
	
	free(positions);
	free(normals);
	free(tex_coords);
	free(material_records);
	free(mpk_file_buffer);
	
	for(i = 0; i < referenced_material_cursor; i++)
	{
		free(referenced_materials[i]);
	}
	free(referenced_materials);
	
	
	free(texture_records);
}


void load(char *file_name)
{
	FILE *file;
	unsigned long long file_size;
	char *file_buffer;
	int file_buffer_cursor = 0;
	int i;
	int j;
	
	char texture_name[MPK_MAX_NAME_LEN];
	
	mpk_header_t *header;
	material_record_t *material_record;
	mpk_vertex_record_t *vertex_record;
	texture_record_t *texture_record;
	
	file = fopen(file_name, "rb");
	
	if(!file)
	{
		printf("couldn't open %s!\n", file_name);
		return;
	}
	
	
	
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);
	
	
	file_buffer = calloc(file_size, 1);
	fread(file_buffer, file_size, 1, file);
	fclose(file);

	
	header = (mpk_header_t *)file_buffer;
	printf("\n\nvertices: %d\nmaterials: %d\nvertice records: %d\n", header->vertice_count, header->material_count, header->vertex_record_count);
	
	file_buffer_cursor += sizeof(mpk_header_t );
	
	for(i = 0; i < header->texture_count; i++)
	{
		texture_record = (texture_record_t *)(file_buffer + file_buffer_cursor);
		file_buffer_cursor += sizeof(texture_record_t );
		
		
		strcpy(texture_name, file_buffer + file_buffer_cursor);
		file_buffer_cursor += strlen(texture_name) + 1;
		
		printf("texture %s ", texture_name);
		
		strcpy(texture_name, file_buffer + file_buffer_cursor);
		file_buffer_cursor += strlen(texture_name) + 1;
		printf("[%s]...\n", texture_name);
		
	}
	
	
	
	for(i = 0; i < header->material_count; i++)
	{
		material_record = (material_record_t *)(file_buffer + file_buffer_cursor);
		printf("material %s...\n", material_record->name);
		
		printf("base color [%f %f %f %f]\n", material_record->base.r, material_record->base.g, material_record->base.b, material_record->base.a);
		
		file_buffer_cursor += sizeof(material_record_t) + strlen(material_record->name);
		
		if(material_record->bm_flags & MATERIAL_USE_DIFFUSE_TEXTURE)
		{
			strcpy(texture_name, file_buffer + file_buffer_cursor);
			printf("diffuse texture %s\n", texture_name);
			
			j = strlen(texture_name) + 1;
			
			file_buffer_cursor += j;
			
		}
		
		if(material_record->bm_flags & MATERIAL_USE_NORMAL_TEXTURE)
		{
			strcpy(texture_name, file_buffer + file_buffer_cursor);
			printf("normal texture %s\n", texture_name);
			
			j = strlen(texture_name) + 1;
			
			file_buffer_cursor += j;
		}
	}
	
	
	free(file_buffer);
	
}











