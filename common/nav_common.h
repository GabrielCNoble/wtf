#ifndef NAV_COMMON_H
#define NAV_COMMON_H


#include "vector.h"


#define MAX_WAYPOINTS 16000

enum WAYPOINT_FLAGS
{
	WAYPOINT_FLAG_CLOSED = 1,
	WAYPOINT_FLAG_OPEN = 1 << 1,	
	WAYPOINT_FLAG_DEACTIVATED = 1 << 2,
	WAYPOINT_FLAG_INVALID = 1 << 3
};

struct waypoint_link_t
{
	float sqrd_cost;
	int waypoint_index;
};


struct waypoint_t
{
	vec3_t position;
	
	struct waypoint_t *parent;				/* used during path calculations... */
	float route_cost;						/* used during path calculations... */
	float h_cost;							/* used during path calculations... */
	int open_list_index;					/* used during path calculations... */
	
	int flags;
	
	short links_count;
	short max_links;
	struct waypoint_link_t *links;
};





#endif
