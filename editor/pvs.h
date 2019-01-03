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

typedef struct pvs_job_t
{
	struct pvs_job_t *next;
	bsp_leaf_t *leaf;
	float time_out;
	int src_portal_index;
	//float last_time_out;
}pvs_job_t;

enum PORTAL_PLANE
{
	PORTAL_FRONT,
	PORTAL_BACK,
	PORTAL_STRADDLING,
	PORTAL_CONTAINED
};

#define MAX_VALID_PORTALS 64
#define MAX_CLIP_PLANES 64

typedef struct recursive_pvs_for_leaf_stack_t
{
	int out_valid_dst_portal_count;
	int clipplane_count;

	bsp_portal_t valid_portals[MAX_VALID_PORTALS];
	bsp_portal_t *out_valid_dst_portals[MAX_VALID_PORTALS];
	bsp_clipplane_t clipplanes[MAX_CLIP_PLANES];

	bsp_leaf_t *dst_dst_leaf;
	bsp_portal_t *dst_dst_portal;

	bsp_leaf_t *src_leaf;
	bsp_leaf_t *dst_leaf;

	bsp_portal_t *src_portal;
	bsp_polygon_t *src_portal_polygon;

	bsp_portal_t *dst_portal;
	bsp_polygon_t *dst_portal_polygon;

}recursive_pvs_for_leaf_stack_t;


typedef struct pvs_for_leaf_stack_t
{
	bsp_portal_t **portals;
	bsp_leaf_t *dst_leaf;
	int in_dst_portal_count;
	bsp_portal_t *in_dst_portals[MAX_VALID_PORTALS];

	recursive_pvs_for_leaf_stack_t *recursive_stack;
	int recursive_stack_pointer;
}pvs_for_leaf_stack_t;




void bsp_GenVisSamples(struct bsp_node_t *bsp);

void bsp_CalculatePvs2();

void bsp_CalculatePvs3();



int bsp_ClassifyPortalVertex(struct bsp_pnode_t *node, vec3_t point);

int bsp_ClassifyPortal(struct bsp_pnode_t *node, bsp_portal_t *portal);

void bsp_SplitPortal(struct bsp_pnode_t *node, bsp_portal_t *portal, bsp_portal_t **front, bsp_portal_t **back);

void bsp_BspBounds(bsp_node_t *bsp, vec3_t *maxs, vec3_t *mins);

void bsp_ClipPortalsToBounds(bsp_node_t *bsp, bsp_portal_t *portals);

bsp_portal_t *bsp_ClipPortalToBsp(bsp_node_t *node, bsp_portal_t *portal);

void bsp_ClipPortalsToBsp(bsp_node_t *bsp, bsp_portal_t **portals);

void bsp_RemoveBadPortals(bsp_portal_t **portals);

void bsp_LinkBack(bsp_portal_t *portals);

void bsp_BuildNodePolygons(bsp_node_t *root, bsp_polygon_t **node_polygons);

void bsp_GeneratePortals(bsp_node_t *bsp, bsp_portal_t **portals);

void bsp_DeletePortals(bsp_portal_t *portals);

int bsp_PvsForLeaf(bsp_leaf_t *leaf, float time_out, int portal_index);

void bsp_PvsForLeaves(bsp_node_t *bsp, bsp_portal_t *portals);

int bsp_PvsForLeavesThreadFn(void *data);

void bsp_CalculatePvs(bsp_node_t *bsp);

int bsp_CalculatePvsAssync(void *data);

void bsp_BuildPvsJobList(bsp_node_t *bsp);

bsp_leaf_t *bsp_GetNextLeaf();

pvs_job_t *bsp_GetNextJob();

void bsp_RequeueLeaf(bsp_leaf_t *leaf);

void bsp_RequeueJob(pvs_job_t *job);

void bsp_DeletePvsJobList();

void bsp_Stop();



//void bsp_AllocPvs(bsp_node_t *bsp, bsp_portal_t *portals);

//void bsp_ApproximatePvsForLeaf(bsp_leaf_t *src_leaf, vec3_t *src_center, bsp_node_t *node, bsp_node_t *bsp);

//void bsp_ApproximatePvsForLeaves(bsp_node_t *node, bsp_node_t *bsp, int leaf_count);

//void bsp_CalculateApproximatePvs(bsp_node_t *bsp);

int bsp_LineOfSight(bsp_node_t *root, vec3_t *start, vec3_t *end);

void bsp_DrawPortals();

void bsp_NextPortal();









#endif
