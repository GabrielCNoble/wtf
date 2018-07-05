#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H



typedef struct scenegraph_node_t
{
	int transform_component_index;
	scenegraph_node_t *parent;
	
	int child_count;
	scenegraph_node_t **child;
}scenegraph_node_t;






#endif
