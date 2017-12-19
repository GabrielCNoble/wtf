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
}bsp_portal_t;

enum PORTAL_PLANE
{
	PORTAL_FRONT,
	PORTAL_BACK,
	PORTAL_STRADDLING,
	PORTAL_CONTAINED
};

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

void bsp_DrawPortals();

void bsp_NextPortal();







#endif
