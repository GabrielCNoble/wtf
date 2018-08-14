#include "mpk_file.h"

//void mpk_write(char *output_name, char *input_file);



struct face_index_t
{
	int vertex_index;
	int tex_coord_index;
	int normal_index;
	int used;
	char *material_name;
};


//void mpk_write(char *output_name, vertex_t *vertices, int vertice_count, struct mpk_vertex_record_t *vertex_records, int vertex_record_count);
void mpk_write(char *output_name, struct input_params_t *params);

void mpk_index(struct input_params_t *params);

void mpk_lod(struct input_params_t *in_params, struct output_params_t *out_params, int max_lod);

void mpk_optmize(struct input_params_t *in_params, struct output_params_t *out_params);

void mpk_convert(char *output_name, char *input_file);

