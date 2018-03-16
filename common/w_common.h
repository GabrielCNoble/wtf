#ifndef W_COMMON_H
#define W_COMMON_H


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



/* 
	drawing with triangle groups...
	
	triangle groups allow to draw things with an
	arbitrary number of materials with the smallest
	amount of material permutations possible.
	
	a model that uses material groups have three
	main things:
	
		- an array of vertex_t, each containing position,
		normal, texture coordinates and tangent, and each
		three of those vertex_t's represent a triangle. No
		information of which material to be used exists here.
		
		- an array of triangle_group_t. Each one of those contain
		information to access an index buffer (which in turn indexes
		into the vertex_t array), and contains three fields:
			
			- material_index: which material is to be used with
			this triangle;
			
			- start: the offset from the beginning of the index
			buffer, which represent the start of the block that 
			represents that triangle group within the index buffer; 
			
			- next: used to keep track of the next entry in 
			the index buffer and to count how many indexes
			exist in this triangle group;
			
		
		the index buffer indexes into the vertex_t array. This
		buffer is partitioned in blocks, and each block belongs to
		a material group. All the indexes (one for each vertex of
		a triangle) contained within a same block represent triangles 
		that share the same material.
		
	
	to draw the model using this mechanism suffices to iterate
	over the triangle groups, setting material parameters accordingly
	and using indexed drawing to clear all the triangles using that 
	material.
	
	triangle groups remain valid as long as the triangle count and order doesn't
	change. So it's possible to reuse the same triangle groups information
	for several instances of the same model (animated or not) as long
	they don't LOD. For different LODs, different triangle groups have
	to be computed. This in turn makes possible to represent a different
	LOD just by using a different list of triangle groups.
		
*/



#endif





