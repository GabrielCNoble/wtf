#ifndef W_COMMON_H
#define W_COMMON_H

typedef struct
{
	int first_vertex;										   
	int triangle_group_index;				/* references the triangle group this triangle is part of... */
}world_mesh_triangle_t;

/* A triangle_group_t groups triangles which share the same material. It keeps
an entry in a GL_ELEMENT_ARRAY_BUFFER, which contain the indexes (laid contiguously on said buffer)
of those triangles. The triangle_group_t keeps how many vertices exist under that group, 
and also the material that is to be used when rendering that triangles. */
typedef struct
{
	//unsigned int vertex_count;
	int material_index;
	unsigned int start;								/* offset from the beginning of the GL_ELEMENT_ARRAY_BUFFER */
	unsigned int next;								/* offset to put the next vertex... */
}triangle_group_t;


#endif
