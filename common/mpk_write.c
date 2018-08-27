#include "mpk_write.h"
#include "gmath/vector.h"
#include "model.h"
#include "texture.h"
#include "c_memory.h"

#include "material.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>





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

		//ab = sub3(b, a);
		//ac = sub3(c, a);

		ab.x = b.x - a.x;
		ab.y = b.y - a.y;
		ab.z = b.z - a.z;

		ac.x = c.x - a.x;
		ac.y = c.y - a.y;
		ac.z = c.z - a.z;


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


//void load_obj(FILE *file, vertex_t **out_vertices, int *out_vert_count, material_record_t **out_material_records, int *out_material_records_count, mpk_vertex_record_t **out_vertex_records, int *out_vertex_record_count)

void load_obj(FILE *file, struct input_params_t *in_params)
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

	int read_data = 0;

	int vertex_records_cursor = 0;
	//struct mpk_vertex_record_t *vertex_records = NULL;
	//struct mpk_vertex_record_t *vertex_record = NULL;

	struct mpk_batch_t *batches = NULL;
	struct mpk_batch_t *batch = NULL;
	int batch_cursor = 0;

	int positions_cursor = 0;
	vec3_t *positions = NULL;

	int normals_cursor = 0;
	vec3_t *normals = NULL;

	int tex_coords_cursor = 0;
	vec2_t *tex_coords = NULL;

	int face_index_cursor = 0;
	struct face_index_t *face_indexes = NULL;

	int referenced_material_cursor = 0;
	char **referenced_materials = NULL;

	int mtllib_cursor = 0;
	char **mtllibs = NULL;

	int material_records_cursor = 0;
	struct material_record_t *material_records = NULL;
	struct material_record_t *material_record = NULL;

	int texture_record_cursor = 0;
	texture_record_t *texture_records = NULL;

	int value_str_cursor = 0;
	char value_str[1024];

	vec3_t v;

	float q;

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	rewind(file);

	file_buffer = memory_Calloc(file_size, 1);
	fread(file_buffer, 1, file_size, file);

	fclose(file);

	/*================================================================================================*/
	/*================================================================================================*/
	/*================================================================================================*/

	for(read_data = 0; read_data <= 1; read_data++)
	{
		cursor = 0;
		while(file_buffer[cursor] != '\0')
		{
			switch(file_buffer[cursor])
			{
				case 'v':
					cursor++;

					if(file_buffer[cursor] == 'n')
					{
						if(read_data)
						{
							t = 'n';
							c = 3;
						}
						else
						{
							normal_count++;
						}

						cursor++;
					}
					else if(file_buffer[cursor] == 't')
					{
						if(read_data)
						{
							t = 't';
							c = 2;
						}
						else
						{
							tex_coord_count++;
						}

						cursor++;
					}
					else
					{
						if(read_data)
						{
							t = 'p';
							c = 3;
						}
						else
						{
							position_count++;
						}
					}

					if(read_data)
					{
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

							case 'p':
								positions[positions_cursor] = v;
								positions_cursor++;
							break;
						}
					}


				break;

				case 'f':
					cursor++;
					while(file_buffer[cursor] == ' ') cursor++;

					while(file_buffer[cursor] != '\n' && file_buffer[cursor] != '\0' && file_buffer[cursor] != '\r')
					{

						while(file_buffer[cursor] == ' ') cursor++;

						if(read_data)
						{
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
						else
						{
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
						}


					}

					if(read_data)
					{
						/* negative indexes flag the end of a face... */
						face_indexes[face_index_cursor].vertex_index = -1;
						face_indexes[face_index_cursor].tex_coord_index = -1;
						face_indexes[face_index_cursor].normal_index = -1;
						face_index_cursor++;
					}

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

						if(read_data)
						{
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
						else
						{
							mtllib_count++;
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

						if(read_data)
						{
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
							will use this material from now on... */
							current_material = referenced_materials[i];
						}
						else
						{
							material_refs++;
						}
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

		if(read_data)
		{
			break;
		}


		current_material = NULL;
		positions = memory_Malloc(sizeof(vec3_t) * position_count);
		normals = memory_Malloc(sizeof(vec3_t) * normal_count);

		if(tex_coord_count)
		{
			tex_coords = memory_Malloc(sizeof(vec3_t) * tex_coord_count);
		}

		face_indexes = memory_Malloc(sizeof(struct face_index_t) * face_index_count);

		if(material_refs)
		{
			referenced_materials = memory_Malloc(sizeof(char *) * material_refs);
		}

		if(mtllib_count)
		{
			mtllibs = memory_Malloc(sizeof(char *) * mtllib_count);
		}
	}


	/*================================================================================================*/
	/*================================================================================================*/
	/*================================================================================================*/


	memory_Free(file_buffer);

	vertices = memory_Malloc(sizeof(vertex_t) * vertice_count);

	material_records = memory_Malloc(referenced_material_cursor * sizeof(struct material_record_t));
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

		file_buffer = memory_Malloc(file_size);
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

						//strcpy(material_records[material_records_cursor].separate_names.material_name, value_str);
						memset(&material_records[material_records_cursor], 0, sizeof(struct material_record_t));
						strcpy(material_records[material_records_cursor].material_name, value_str);
						/*for(i = 0; i < PATH_MAX; i++)
						{
							material_records[material_records_cursor].separate_names.diffuse_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.normal_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.height_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.roughness_texture_name[i] = '\0';
							material_records[material_records_cursor].separate_names.metalness_texture_name[i] = '\0';
						}*/
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
							strcpy(material_records[material_records_cursor].diffuse_texture_name, value_str);
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
							strcpy(material_records[material_records_cursor].normal_texture_name, value_str);
						}

					}
				break;
			}

			cursor++;
		}

	}

	if(file_buffer)
	{
		memory_Free(file_buffer);
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

	batches = memory_Malloc(sizeof(struct mpk_batch_t) * (material_records_cursor + 1));
	batch_cursor = 0;

	for(j = 0; j < material_records_cursor; j++)
	{

		material_record = &material_records[j];

		batch = &batches[batch_cursor];

		batch->material_name[0] = '\0';
		batch->indice_start = 0;
		batch->indice_count = 0;
		//batch->vertice_count = 0;

		if(batch_cursor)
		{
			batch->indice_start = batches[batch_cursor - 1].indice_start + batches[batch_cursor - 1].indice_count;
		}

		batch_cursor++;

		for(i = 0; i < face_index_cursor; i++)
		{
			/* go over all the indexes found. A index of -1
			marks the end of a face... */
			if(face_indexes[i].vertex_index > -1)
			{
				/* if this index belongs to a face that uses this material... */
				if(!strcmp(face_indexes[i].material_name, material_records[j].material_name))
				{
					/* append it to the list of vertices */
					vertices[batch->indice_start + batch->indice_count].position = positions[face_indexes[i].vertex_index];
					vertices[batch->indice_start + batch->indice_count].normal = normals[face_indexes[i].normal_index];

					if(face_indexes[i].tex_coord_index > -1)
					{
						vertices[batch->indice_start + batch->indice_count].tex_coord = tex_coords[face_indexes[i].tex_coord_index];
					}
					else
					{
						vertices[batch->indice_start + batch->indice_count].tex_coord.x = 0.0;
						vertices[batch->indice_start + batch->indice_count].tex_coord.y = 0.0;
					}

					/* mark this index as used, so to any non-marked
					index the default material gets assigned...  */
					face_indexes[i].used = 1;

					batch->indice_count++;
					strcpy(batch->material_name, face_indexes[i].material_name);
				}
			}
		}

		/* no vertices referenced this material... */
		if(!batch->indice_count)
		{
			/* ...so get rid of the vertex record... */
			batch_cursor--;

			/* ...and of the material... */
			if(j < material_records_cursor - 1)
			{
				c = material_records_cursor - 1;
				strcpy(material_records[j].material_name, material_records[c].material_name);
				strcpy(material_records[j].diffuse_texture_name , material_records[c].diffuse_texture_name);
				strcpy(material_records[j].normal_texture_name, material_records[c].normal_texture_name);
				strcpy(material_records[j].height_texture_name, material_records[c].height_texture_name);
				strcpy(material_records[j].roughness_texture_name, material_records[c].roughness_texture_name);
				strcpy(material_records[j].metalness_texture_name, material_records[c].metalness_texture_name);


				material_records[j].base = material_records[c].base;
				material_records[j].roughness = material_records[c].roughness;
				material_records[j].metalness = material_records[c].metalness;
				material_records[j].flags = material_records[c].flags;
			}

			material_records_cursor--;
			j--;
		}
	}


	batch = &batches[batch_cursor];
	strcpy(batch->material_name, "default");
	batch->indice_start = 0;
	batch->indice_count = 0;

	if(batch_cursor)
	{
		batch->indice_start = batches[batch_cursor - 1].indice_start + batches[batch_cursor - 1].indice_count;
	}

	batch_cursor++;

	/* go over all the vertices and assign the not
	used ones to the default material. Those are
	the indigent vertices... */
	for(i = 0; i < face_index_cursor; i++)
	{
		if(face_indexes[i].vertex_index > -1)
		{
			if(!face_indexes[i].used)
			{
				vertices[batch->indice_start + batch->indice_count].position = positions[face_indexes[i].vertex_index];
				vertices[batch->indice_start + batch->indice_count].normal = normals[face_indexes[i].normal_index];

				if(face_indexes[i].tex_coord_index > -1)
				{
					vertices[batch->indice_start + batch->indice_count].tex_coord = tex_coords[face_indexes[i].tex_coord_index];
				}
				else
				{
					vertices[batch->indice_start + batch->indice_count].tex_coord.x = 0.0;
					vertices[batch->indice_start + batch->indice_count].tex_coord.y = 0.0;
				}

				batch->indice_count++;
			}
		}
	}

	if(!batch->indice_count)
	{
		batch_cursor--;
	}


	if(vertice_count)
	{
		calculate_tangents(vertices, vertice_count);

		in_params->in_batches = batches;
		in_params->in_batches_count = batch_cursor;

		in_params->in_vertices = vertices;
		in_params->in_vertices_count = vertice_count;
	}

}

void mpk_write(char *output_name, struct input_params_t *in_params)
{
	FILE *file;

	struct mpk_header_t *header;
	struct mpk_batch_t *batch;
	struct mpk_lod_t *lod;

	struct output_params_t out_params;

	vertex_t *verts;

	int i;
	int j;

	unsigned int file_size;
	char *file_buffer;
	char *out;

	if(in_params->in_vertices_count)
	{

		mpk_index(in_params);
		mpk_optmize(in_params, &out_params);
		mpk_lod(in_params, &out_params, 1);

		file_size = sizeof(struct mpk_header_t);
		file_size += sizeof(vertex_t ) * in_params->in_vertices_count;

		for(i = 0; i < out_params.out_lods_count; i++)
		{
			file_size += sizeof(struct mpk_lod_t);
			file_size += sizeof(int) * out_params.out_lods[i].indice_count;
			file_size += sizeof(struct mpk_batch_t) * out_params.out_batches_count;
		}

		file_buffer = memory_Calloc(file_size, 1);

		out = file_buffer;

		header = (struct mpk_header_t *)out;
		out += sizeof(struct mpk_header_t);

        memset(header, 0, sizeof(struct mpk_header_t ));
        strcpy(header->tag, mpk_header_tag);

		header->batch_count = out_params.out_batches_count;
		header->vertice_count = out_params.out_vertices_count;
		header->indice_count = out_params.out_indices_count;
		header->lod_count = out_params.out_lods_count;

		memcpy(out, out_params.out_vertices, sizeof(vertex_t) * header->vertice_count);
		out += sizeof(struct vertex_t) * header->vertice_count;

		for(j = 0; j < header->lod_count; j++)
		{
            lod = (struct mpk_lod_t *)out;
			out += sizeof(struct mpk_lod_t);

            lod->indice_count = out_params.out_lods[j].indice_count;
            memcpy(out, out_params.out_lods_indices[j], sizeof(int) * lod->indice_count);

            out += sizeof(int) * lod->indice_count;

            for(i = 0; i < out_params.out_batches_count; i++)
			{
                batch = (struct mpk_batch_t *)out;
				out += sizeof(struct mpk_batch_t);

				memcpy(batch, out_params.out_lods_batches[j * out_params.out_batches_count + i], sizeof(struct mpk_batch_t));
			}
		}

		file = fopen(output_name, "wb");

		fwrite(file_buffer, file_size, 1, file);
		fclose(file);
		memory_Free(file_buffer);

		printf("statistics:\n---\n");
		printf("batches: %d\n", in_params->in_batches_count);
		printf("input vertice count: %d\n", in_params->in_vertices_count);
		printf("output vertice count: %d\n", out_params.out_vertices_count);


	}

}





void mpk_index(struct input_params_t *params)
{
	struct mpk_triangle_t *triangles;
	struct mpk_triangle_t *triangle;

	struct mpk_batch_t *batches;
	struct mpk_batch_t *batch;

	int *indices;


	vertex_t *vertices;

	int triangle_count;
	int vertice_count;
	int batch_count;

	int i;
	int j;

	batch_count = params->in_batches_count;
	batches = params->in_batches;

	vertice_count = params->in_vertices_count;
	vertices = params->in_vertices;

	triangle_count = vertice_count / 3;
	triangles = memory_Malloc(sizeof(struct mpk_triangle_t) * triangle_count);

	triangle_count = 0;

	indices = memory_Malloc(sizeof(int) * vertice_count);

	for(i = 0; i < batch_count; i++)
	{
        batch = batches + i;

        for(j = 0; j < batch->indice_count;)
		{
			triangle = triangles + triangle_count;
			triangle->material_name = batch->material_name;
			triangle->verts = indices + triangle_count * 3;

			triangle->verts[0] = j + batch->indice_start;
			j++;

			triangle->verts[1] = j + batch->indice_start;
			j++;

			triangle->verts[2] = j + batch->indice_start;
			j++;

			triangle_count++;
		}
	}



/*	for(i = 0; i < vertice_count; i++)
	{
		indices[i] = i;
	}*/


	/*for(i = 0, j = 0; i < triangle_count; i++)
	{
		triangle = triangles + i;

        indices[j] = triangle->verts[0];
		j++;
		indices[j] = triangle->verts[1];
		j++;
		indices[j] = triangle->verts[2];
		j++;
	}*/

	params->in_triangles = triangles;
	params->in_triangles_count = triangle_count;

    params->in_indices = indices;
    params->in_indices_count = vertice_count;
}

#define MAX_LODS 4

void mpk_lod(struct input_params_t *in_params, struct output_params_t *out_params, int max_lod)
{
	int i;

	int *indices;
	int indices_count;
	int original_vert_count;

    if(max_lod < 0)
	{
		max_lod = 0;
	}
	else if(max_lod > MAX_LODS)
	{
		max_lod = MAX_LODS;
	}

	max_lod = 0;

	//original_vert_count = params->out_vertices_count;

	out_params->out_batches_count = in_params->in_batches_count;
	//out_params->out_vertices_count = in_params->in_vertices_count;

    out_params->out_lods_count = max_lod + 1;
    out_params->out_lods = memory_Malloc(sizeof(struct mpk_lod_t) * out_params->out_lods_count);

    out_params->out_lods_indices = memory_Malloc(sizeof(int *) * out_params->out_lods_count);
    out_params->out_lods_batches = memory_Malloc(sizeof(struct mpk_batch_t *) * out_params->out_lods_count);



	out_params->out_lods[0].indice_count = out_params->out_indices_count;
    out_params->out_lods_indices[0] = memory_Malloc(sizeof(int) * out_params->out_lods[0].indice_count);
    memcpy(out_params->out_lods_indices[0], out_params->out_indices, sizeof(int) * out_params->out_lods[0].indice_count);

    out_params->out_lods_batches[0] = memory_Malloc(sizeof(struct mpk_batch_t) * in_params->in_batches_count);

	//out_params->out_indices_count = 0;

    for(i = 0; i < in_params->in_batches_count; i++)
	{
		strcpy(out_params->out_lods_batches[i]->material_name, in_params->in_batches[i].material_name);

		out_params->out_lods_batches[i]->indice_start = in_params->in_batches[i].indice_start;
		out_params->out_lods_batches[i]->indice_count = in_params->in_batches[i].indice_count;

		//out_params->out_indice_count += in_params->in_batches[i].indice_count;
	}
}

void mpk_optmize(struct input_params_t *in_params, struct output_params_t *out_params)
{
    //struct mpk_vertex_record_t *vertex_records;
    //struct mpk_vertex_record_t *vertex_record;
    //int vertex_records_count;

	struct mpk_batch_t *batches;
	struct mpk_batch_t *batch;
	int batch_count;

    vertex_t *vertices;
    vertex_t *out_vertices;
    int vertice_count;
    int out_vertice_count;

    int *indices;

    struct mpk_triangle_t *triangles;
    struct mpk_triangle_t *triangle;
    int triangle_count = 0;

    int i;
    int j;
    int k;

    int cur_vertex_index;
    int test_vertex_index;
    int rejected_vertices;
    //int real_cur_vertex_index;
    //int real_test_vertex_index;

    int r;
    int m;

    float diff;


    vertex_t *cur_vertex;
    vertex_t *test_vertex;





    //vertex_records = *params->out_vertex_records;
    //vertex_records_count = *params->out_vertex_records_count;

    batches = in_params->in_batches;
    batch_count = in_params->in_batches_count;

    vertices = in_params->in_vertices;
    vertice_count = in_params->in_vertices_count;

    triangles = in_params->in_triangles;
    triangle_count = in_params->in_triangles_count;

    indices = in_params->in_indices;

	/* Find duplicate verts, and make the triangles using those duplicate verts
	point to the same vert... */

    if(vertices)
	{
		vertices = memory_Malloc(sizeof(vertex_t) * vertice_count);
		memcpy(vertices, in_params->in_vertices, sizeof(vertex_t) * vertice_count);

		//out_vertice_count = vertice_count;

		#if 0

		for(i = 0; i < batch_count; i++)
		{
			batch = batches + i;

			if(batch->indice_count > 3)
			{
				for(cur_vertex_index = 0, rejected_vertices = 0; cur_vertex_index < batch->indice_count; cur_vertex_index++)
				{
					cur_vertex = vertices + indices[batch->indice_start + cur_vertex_index];

					if(*(int *)&cur_vertex->position.x == 0xffffffff)
					{
						/* this vertices was markead as invalid, but it can't
						be removed from the list yet given that might be triangles
						that points to vertices further ahead of this one, and moving
						them around would mess the indices the triangles keep into
						this list... */
						rejected_vertices++;
						continue;
					}

					/*printf("[%f %f %f] -- [%f %f %f] -- [%f %f]\n", cur_vertex->position.x, cur_vertex->position.y, cur_vertex->position.z,
																	cur_vertex->normal.x, cur_vertex->normal.y, cur_vertex->normal.z,
																	cur_vertex->tex_coord.x, cur_vertex->tex_coord.y);*/



					for(test_vertex_index = cur_vertex_index + 1; test_vertex_index < batch->indice_count; test_vertex_index++)
					{
						test_vertex = vertices + indices[batch->indice_start + test_vertex_index];

						//if(_isnan(test_vertex->position.x))
						if(*(int *)&test_vertex->position.x == 0xffffffff)
						{
							continue;
							//test_vertex_index--;
						}


						/* position... */
						for(r = 0; r < 3; r++)
						{
							diff = cur_vertex->position.floats[r] - test_vertex->position.floats[r];

                            if(diff > FLT_EPSILON * 2.0 || diff < -FLT_EPSILON * 2.0)
							{
								break;
							}
						}



						if(r >= 3)
						{
							/* normal... */
							for(r = 0; r < 3; r++)
							{
								diff = cur_vertex->normal.floats[r] - test_vertex->normal.floats[r];

								if(diff > FLT_EPSILON * 2.0 || diff < -FLT_EPSILON * 2.0)
								{
									break;
								}
							}
						}
						else
						{
							continue;
						}



						if(r >= 3)
						{
							/* tex coords... */
							for(r = 0; r < 2; r++)
							{
								diff = cur_vertex->tex_coord.floats[r] - test_vertex->tex_coord.floats[r];

								if(diff > FLT_EPSILON * 2.0 || diff < -FLT_EPSILON * 2.0)
								{
									break;
								}
							}
						}
						else
						{
							continue;
						}



						if(r >= 2)
						{
                            /* duplicate vertice... */
                            for(r = 0; r < triangle_count; r++)
							{
								triangle = triangles + r;

								for(m = 0; m < 3; m++)
								{
                                    if(triangle->verts[m] == test_vertex_index)
									{
										/* make this triangle use the current vertice... */

										/* once all the duplicate vertices get marked as so,
										the list will be compacted, and the invalid vertices
										will be removed.

										In order to have the triangles point only at valid vertices
										we subtract from the current vertice we're testing how many
										vertices were marked as invalid so far, so when the list
										gets compacted, the triangles will be pointing at the
										correct vertices.

										In other words, cur_vertex_index points at the current
										vertice we're testing, and cur_vertex_index - rejected_vertices
										points at the last valid vertice so far... */
										triangle->verts[m] = cur_vertex_index - rejected_vertices;

										//printf("change vertex %d for vertex %d\n", test_vertex_index, cur_vertex_index - rejected_vertices);

										*(int *)&vertices[test_vertex_index].position.x = 0xffffffff;

										vertice_count--;

										r = triangle_count;

										break;
									}
								}
							}
						}
					}
				}
			}
		}

		#endif

		out_params->out_vertices = vertices;
		out_params->out_vertices_count = vertice_count;

		#if 0
		m = in_params->in_vertices_count;

		for(i = 0; i < in_params->in_vertices_count && m; i++)
		{
			while(*(int *)&vertices[i].position.x != 0xffffffff) i++;

			for(j = i; j < m - 1; j++)
			{
				vertices[j] = vertices[j + 1];
			}
			i--;
			m--;
		}

		#endif


		out_params->out_indices = memory_Malloc(sizeof(int) * in_params->in_indices_count);
		out_params->out_indices_count = 0;

		for(i = 0; i < triangle_count; i++)
		{
			triangle = triangles + i;

			for(j = 0; j < 3; j++)
			{
				out_params->out_indices[out_params->out_indices_count] = triangle->verts[j];
				out_params->out_indices_count++;
			}
		}

		for(i = 0; i < out_params->out_indices_count; i++)
		{
			printf("[%f %f %f]\n", vertices[out_params->out_indices[i]].position.x,
								   vertices[out_params->out_indices[i]].position.y,
								   vertices[out_params->out_indices[i]].position.z);
		}
	}
}

void mpk_convert(char *output_name, char *input_file)
{

	FILE *file;

	int i;

	vertex_t *vertices;
	int vertice_count;

	struct material_record_t *material_records;
	int material_record_count;

	struct mpk_vertex_record_t *vertex_records;
	int vertex_record_count;

	struct mpk_header_t *header;
	struct mpk_vertex_record_t *record;
	vertex_t *verts;

	struct input_params_t params;

	unsigned int file_size;
	char *file_buffer;
	char *out;

	file = fopen(input_file, "rb");

	if(!file)
	{
		printf("couldn't open file %s!\n", input_file);
		return;
	}


	load_obj(file, &params);

	if(params.in_vertices_count)
	{
		mpk_write(output_name, &params);
		//memory_Free(material_records);
		//memory_Free(vertices);
		//memory_Free(vertex_records);
	}
}






