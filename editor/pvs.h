#ifndef PVS_H
#define PVS_H


#include "bsp_common.h"
#include "bsp_cmp.h"

typedef struct bsp_portal_t
{
	struct bsp_portal_t *next;
	bsp_leaf_t *leaf0;
	bsp_leaf_t *leaf1;
	bsp_polygon_t *portal_polygon;
	float approx_area;
	short pass_through0;
	short pass_through1;
	short go_through;
}bsp_portal_t;

typedef struct
{
	struct timedout_leaf_t *next;
	bsp_leaf_t *leaf;
	float last_time_out;
}timedout_leaf_t;

enum PORTAL_PLANE
{
	PORTAL_FRONT,
	PORTAL_BACK,
	PORTAL_STRADDLING,
	PORTAL_CONTAINED
};


typedef struct
{
	int out_valid_dst_portal_count;
	int clipplane_count;
	
	bsp_portal_t valid_portals[512];
	bsp_portal_t *out_valid_dst_portals[512];
	bsp_clipplane_t clipplanes[512];
	
	bsp_leaf_t *dst_dst_leaf;
	bsp_portal_t *dst_dst_portal;
	
	bsp_leaf_t *src_leaf;
	bsp_leaf_t *dst_leaf;
	
	bsp_portal_t *src_portal;
	bsp_polygon_t *src_portal_polygon;
	
	bsp_portal_t *dst_portal;
	bsp_polygon_t *dst_portal_polygon;
	
}recursive_pvs_for_leaf_stack_t;


typedef struct
{
	bsp_portal_t **portals;
	bsp_leaf_t *dst_leaf;
	int in_dst_portal_count;
	bsp_portal_t *in_dst_portals[512];
	
	recursive_pvs_for_leaf_stack_t *recursive_stack;
	int recursive_stack_pointer;
}pvs_for_leaf_stack_t;


int bsp_ClassifyPortalVertex(bsp_pnode_t *node, vec3_t point);

int bsp_ClassifyPortal(bsp_pnode_t *node, bsp_portal_t *portal);

void bsp_SplitPortal(bsp_pnode_t *node, bsp_portal_t *portal, bsp_portal_t **front, bsp_portal_t **back);

void bsp_BspBounds(bsp_node_t *bsp, vec3_t *maxs, vec3_t *mins);

void bsp_ClipPortalsToBounds(bsp_node_t *bsp, bsp_portal_t *portals);

bsp_portal_t *bsp_ClipPortalToBsp(bsp_node_t *node, bsp_portal_t *portal);

void bsp_ClipPortalsToBsp(bsp_node_t *bsp, bsp_portal_t **portals);

void bsp_RemoveBadPortals(bsp_portal_t **portals);

void bsp_LinkBack(bsp_portal_t *portals);

void bsp_BuildNodePolygons(bsp_node_t *root, bsp_polygon_t **node_polygons);

void bsp_GeneratePortals(bsp_node_t *bsp, bsp_portal_t **portals);

void bsp_DeletePortals(bsp_portal_t *portals);

void bsp_PvsForLeaf(bsp_leaf_t *leaf);

void bsp_PvsForLeaves(bsp_node_t *bsp, bsp_portal_t *portals);

void bsp_CalculatePvs(bsp_node_t *bsp);

int bsp_CalculatePvsAssync(void *data);




//void bsp_AllocPvs(bsp_node_t *bsp, bsp_portal_t *portals);

//void bsp_ApproximatePvsForLeaf(bsp_leaf_t *src_leaf, vec3_t *src_center, bsp_node_t *node, bsp_node_t *bsp);

//void bsp_ApproximatePvsForLeaves(bsp_node_t *node, bsp_node_t *bsp, int leaf_count);

//void bsp_CalculateApproximatePvs(bsp_node_t *bsp);

int bsp_LineOfSight(bsp_node_t *root, vec3_t *start, vec3_t *end);

void bsp_DrawPortals();

void bsp_NextPortal();









#endif
