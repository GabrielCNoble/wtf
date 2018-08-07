#include "mpk_file.h"

//void mpk_write(char *output_name, char *input_file);




struct load_params_t
{
    vertex_t *out_vertices;
    int out_vertices_count;

    int *out_indices;						/* indices generated from the indexer... */
    int out_indices_count;					/* generally equal to the amount of vertices read from the input file... */

    struct mpk_lod_t *out_lods;
    int out_lods_count;						/* will always be at least one... */
    int **out_lods_indices;					/* indices generated from the loder. These are the ones that get written... */
    struct mpk_batch_t **out_lods_batches;	/* batches generated from the loder. These are the ones that get written.. */


    struct mpk_batch_t *out_batches;
    int out_batches_count;					/* will always be at least one... */

	struct mpk_triangle_t *out_triangles;
	int out_triangles_count;

	material_record_t *out_material_records;
    int out_material_records_count;
};

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

