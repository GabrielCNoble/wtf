#include "navigation.h"
#include "physics.h"
#include "c_memory.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

int nav_waypoint_count = 0;
int nav_max_waypoints = 0;
int nav_waypoint_stack_top = -1;
int *nav_waypoint_stack = NULL;
struct waypoint_t nav_temp_end_waypoint;
struct waypoint_t nav_temp_start_waypoint;
struct waypoint_t *nav_waypoints = NULL;

static int nav_closed_list_cursor = 0;
static struct waypoint_t **nav_closed_list = NULL;
static int nav_open_list_cursor = 0;
static struct waypoint_t **nav_open_list = NULL;


#ifdef __cplusplus
extern "C"
{
#endif

int navigation_Init()
{

	nav_waypoint_stack = memory_Malloc(sizeof(int) * MAX_WAYPOINTS, "navigation_Init");
	nav_waypoints = memory_Malloc(sizeof(struct waypoint_t) * MAX_WAYPOINTS, "navigation_Init");


	nav_closed_list = memory_Malloc(sizeof(struct waypoint_t *) * MAX_WAYPOINTS, "navigation_Init");
	nav_open_list = memory_Malloc(sizeof(struct waypoint_t *) * MAX_WAYPOINTS, "navigation_Init");
	return 1;
}

void navigation_Finish()
{
	int i;

	for(i = 0; i < nav_waypoint_count; i++)
	{
		if(nav_waypoints[i].flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

		memory_Free(nav_waypoints[i].links);
	}

	memory_Free(nav_waypoint_stack);
	memory_Free(nav_waypoints);

	memory_Free(nav_closed_list);
	memory_Free(nav_open_list);
}


/*
=================================================================
=================================================================
=================================================================
*/

int navigation_CreateWaypoint(vec3_t position)
{
	int waypoint_index;
	struct waypoint_t *waypoint;

	char name[512];

	if(nav_waypoint_stack_top >= 0)
	{
		waypoint_index = nav_waypoint_stack[nav_waypoint_stack_top];
		nav_waypoint_stack_top--;
	}
	else
	{
		waypoint_index = nav_waypoint_count;
		nav_waypoint_count++;
	}

	waypoint = nav_waypoints + waypoint_index;


	waypoint->max_links = 16;
	waypoint->links_count = 0;
	waypoint->links = memory_Malloc(sizeof(struct waypoint_link_t) * waypoint->max_links, "navigation_CreateWaypoint");
	waypoint->position = position;
	waypoint->flags = 0;
	waypoint->parent = NULL;

	sprintf(name, "waypoint.%d", waypoint_index);

	waypoint->name = memory_Strdup(name, "navigation_CreateWaypoint");

	return waypoint_index;
}

void navigation_DestroyWaypoint(int waypoint_index)
{
	int i;
	struct waypoint_t *waypoint;

	if(waypoint_index >= 0 && waypoint_index < nav_waypoint_count)
	{
		if(!(nav_waypoints[waypoint_index].flags & WAYPOINT_FLAG_INVALID))
		{
			waypoint = nav_waypoints + waypoint_index;

			waypoint->flags |= WAYPOINT_FLAG_INVALID;

			for(i = 0; waypoint->links_count; i++)
			{
				navigation_UnlinkWaypoints(waypoint_index, waypoint->links[i].waypoint_index);
			}

			memory_Free(waypoint->name);

			nav_waypoint_stack_top++;
			nav_waypoint_stack[nav_waypoint_stack_top] = waypoint_index;
		}
	}

}

struct waypoint_t *navigation_GetWaypointPointer(int waypoint_index)
{
	if(waypoint_index >= 0 && waypoint_index < nav_waypoint_count)
	{
		if(!(nav_waypoints[waypoint_index].flags & WAYPOINT_FLAG_INVALID))
		{
			return nav_waypoints + waypoint_index;
		}
	}

	return NULL;
}

void navigation_LinkWaypoints(int waypoint_a, int waypoint_b)
{
	struct waypoint_t *a;
	struct waypoint_t *b;
	vec3_t v;
	float sqrd_cost;

	int i;

	struct waypoint_link_t *link;

	a = nav_waypoints + waypoint_a;
	b = nav_waypoints + waypoint_b;

	for(i = 0; i < a->links_count; i++)
	{
		if(a->links[i].waypoint_index == waypoint_b)
		{
			return;
		}
	}

	if(a->links_count >= a->max_links)
	{
		link = memory_Malloc(sizeof(struct waypoint_link_t) * (a->max_links + 16), "navigation_LinkWaypoints");
		memcpy(link, a->links, sizeof(struct waypoint_link_t) * a->max_links);
		memory_Free(a->links);
		a->links = link;
		a->max_links += 16;
	}

	if(b->links_count >= b->max_links)
	{
		link = memory_Malloc(sizeof(struct waypoint_link_t) * (b->max_links + 16), "navigation_LinkWaypoints");
		memcpy(link, b->links, sizeof(struct waypoint_link_t) * b->max_links);
		memory_Free(b->links);
		b->links = link;
		b->max_links += 16;
	}


	//v.x = a->position.x - b->position.x;
	//v.y = a->position.y - b->position.y;
	//v.z = a->position.z - b->position.z;

	//sqrd_cost = dot3(v, v);

	link = a->links + a->links_count;
	link->waypoint_index = waypoint_b;
	//link->sqrd_cost = sqrd_cost;
	a->links_count++;

	link = b->links + b->links_count;
	link->waypoint_index = waypoint_a;
	//link->sqrd_cost = sqrd_cost;
	b->links_count++;

	navigation_UpdateWaypoint(waypoint_a);
	navigation_UpdateWaypoint(waypoint_b);
}

void navigation_UnlinkWaypoints(int waypoint_a, int waypoint_b)
{
	int i;

	struct waypoint_t *a;
	struct waypoint_t *b;

	a = nav_waypoints + waypoint_a;
	b = nav_waypoints + waypoint_b;

	for(i = 0; i < a->links_count; i++)
	{
		if(a->links[i].waypoint_index == waypoint_b)
		{
			if(i < a->links_count - 1)
			{
				a->links[i] = a->links[a->links_count - 1];
			}
			a->links_count--;

			break;
		}
	}

	for(i = 0; i < b->links_count; i++)
	{
		if(b->links[i].waypoint_index == waypoint_a)
		{
			if(i < b->links_count - 1)
			{
				b->links[i] = b->links[b->links_count - 1];
			}

			b->links_count--;

			break;
		}
	}
}

void navigation_BuildLinks()
{
	int i;
	int j;

	struct waypoint_t *current_waypoint;
	struct waypoint_t *waypoint;

	for(i = 0; i < nav_waypoint_count; i++)
	{
		nav_waypoints[i].links_count = 0;
	}


	for(i = 0; i < nav_waypoint_count; i++)
	{
		current_waypoint = &nav_waypoints[i];

		for(j = 0; j < nav_waypoint_count; j++)
		{
			if(i == j)
			{
				continue;
			}

			waypoint = &nav_waypoints[j];

			if(!physics_Raycast(current_waypoint->position, waypoint->position, NULL, NULL))
			{
				navigation_LinkWaypoints(i, j);
			}

		}

	}

}

void navigation_UpdateWaypoint(int waypoint_index)
{
	int i;
	int j;

	struct waypoint_t *waypoint;
	struct waypoint_t *linked_waypoint;

	vec3_t v;
	float sqrd_cost;

	if(waypoint_index >= 0 && waypoint_index < nav_waypoint_count)
	{
		if(nav_waypoints[waypoint_index].flags & WAYPOINT_FLAG_INVALID)
		{
			return;
		}

		waypoint = nav_waypoints + waypoint_index;

		for(i = 0; i < waypoint->links_count; i++)
		{
			linked_waypoint = nav_waypoints + waypoint->links[i].waypoint_index;

			v.x = waypoint->position.x - linked_waypoint->position.x;
			v.y = waypoint->position.y - linked_waypoint->position.y;
			v.z = waypoint->position.z - linked_waypoint->position.z;

			sqrd_cost = dot3(v, v);

			waypoint->links[i].sqrd_cost = sqrd_cost;

			for(j = 0; j < linked_waypoint->links_count; j++)
			{
				if(i == j)
				{
					continue;
				}

				if(linked_waypoint->links[j].waypoint_index == waypoint_index)
				{
					linked_waypoint->links[j].sqrd_cost = sqrd_cost;
				}
			}

		}

	}

}

void navigation_MoveWaypoint(int waypoint_index, vec3_t direction, float amount)
{
	struct waypoint_t *waypoint;

	if(waypoint_index >= 0 && waypoint_index < MAX_WAYPOINTS)
	{
		if(nav_waypoints[waypoint_index].flags & WAYPOINT_FLAG_INVALID)
		{
			return;
		}

		waypoint = nav_waypoints + waypoint_index;

		waypoint->position.x += direction.x * amount;
		waypoint->position.y += direction.y * amount;
		waypoint->position.z += direction.z * amount;

		navigation_UpdateWaypoint(waypoint_index);
	}


}

/*
=================================================================
=================================================================
=================================================================
*/

struct waypoint_t *navigation_GetClosestWaypoint(vec3_t position)
{
	float lowest_d = FLT_MAX;
	struct waypoint_t *closest = NULL;
	float d;
	vec3_t v;
	int i;


	for(i = 0; i < nav_waypoint_count; i++)
	{
		v.x = position.x - nav_waypoints[i].position.x;
		v.y = position.y - nav_waypoints[i].position.y;
		v.z = position.z - nav_waypoints[i].position.z;

		d = v.x * v.x + v.y * v.y + v.z * v.z;


		if(d < lowest_d)
		{
			//if(!physics_Raycast(position, nav_waypoints[i].position, NULL, NULL))
			{
				lowest_d = d;
				closest = nav_waypoints + i;
			}
		}
	}

	return closest;
}

struct waypoint_t **navigation_FindPath(int *waypoint_count, vec3_t from, vec3_t to)
{
	struct waypoint_t *start_point;
	struct waypoint_t *end_point;
	struct waypoint_t *current;
	struct waypoint_t *neighbor;
	struct waypoint_link_t *links;

	float cost;
	float new_cost;

	float lowest_f;
	float f;

	float d;
	vec3_t v;

	int i;
	int current_index;
	int lowest_index;
	int link_count;

	/* find the path backwards, so the list can
	be constructed in the right direction by
	following the parent pointers... */
	start_point = navigation_GetClosestWaypoint(to);
	end_point = navigation_GetClosestWaypoint(from);

	nav_open_list_cursor = 0;
	nav_closed_list_cursor = 0;

	if(!start_point)
	{
		return NULL;
	}

	v.x = start_point->position.x - from.x;
	v.y = start_point->position.y - from.y;
	v.z = start_point->position.z - from.z;


	start_point->parent = NULL;
	start_point->route_cost = 0.0;
	start_point->h_cost = dot3(v, v);

	nav_open_list[nav_open_list_cursor] = start_point;
	nav_open_list_cursor++;

	for(i = 0; i < nav_waypoint_count; i++)
	{
		nav_waypoints[i].flags &= ~(WAYPOINT_FLAG_OPEN | WAYPOINT_FLAG_CLOSED);
		nav_waypoints[i].parent = NULL;
	}

	while(nav_open_list_cursor)
	{
		lowest_f = FLT_MAX;

		/* find the best waypoint in the open list... */
		for(i = 0; i < nav_open_list_cursor; i++)
		{
			f = nav_open_list[i]->route_cost + nav_open_list[i]->h_cost;

			if(f < lowest_f)
			{
				lowest_f = f;
				current_index = i;
			}
		}

		current = nav_open_list[current_index];
		/* ... drop it from the open list... */
		if(current_index < nav_open_list_cursor - 1)
		{
			nav_open_list[current_index] = nav_open_list[nav_open_list_cursor - 1];
		}
		nav_open_list_cursor--;

		/* ... and add it to the closed list... */
		nav_closed_list[nav_closed_list_cursor] = current;
		nav_closed_list_cursor++;

		current->flags |= WAYPOINT_FLAG_CLOSED;
		current->flags &= ~WAYPOINT_FLAG_OPEN;


		if(current == end_point)
		{

			/* if the start and end point are not coincidental, use the
			parent pointers to build the actual route and store it in
			the closed list... */
			if(start_point != end_point)
			{
				nav_closed_list_cursor = 0;
				current = end_point;

				/*if(current->position.x != from.x || current->position.y != from.y || current->position.z != from.z)
				{
					nav_temp_start_waypoint.position = from;
					nav_closed_list[nav_closed_list_cursor] = &nav_temp_start_waypoint;
					nav_closed_list_cursor++;
				}*/

				while(current != start_point)
				{
					nav_closed_list[nav_closed_list_cursor] = current;
					nav_closed_list_cursor++;
					current = current->parent;
				}

				nav_closed_list[nav_closed_list_cursor] = current;
				nav_closed_list_cursor++;

				if(current->position.x != to.x || current->position.y != to.y || current->position.z != to.z)
				{
					nav_temp_end_waypoint.position = to;
					nav_closed_list[nav_closed_list_cursor] = &nav_temp_end_waypoint;
					nav_closed_list_cursor++;
				}
			}

			*waypoint_count = nav_closed_list_cursor;
			return nav_closed_list;
		}

		links = current->links;
		link_count = current->links_count;

		for(i = 0; i < link_count; i++)
		{
			neighbor = nav_waypoints + links[i].waypoint_index;

			/* the total cost of this route if we are to move to this neighbor waypoint... */
			cost = current->route_cost + links[i].sqrd_cost;

			/* if the total cost of this route is smaller then the total cost of
			another route going through this same neighbor, drop it from the open
			list as the current route is better... */
			if((neighbor->flags & WAYPOINT_FLAG_OPEN) && cost < neighbor->route_cost)
			{
				if(neighbor->open_list_index < nav_open_list_cursor - 1)
				{
					nav_open_list[neighbor->open_list_index] = nav_open_list[nav_open_list_cursor - 1];
					nav_open_list[neighbor->open_list_index]->open_list_index = neighbor->open_list_index;
				}

				nav_open_list_cursor--;

				neighbor->flags &= ~WAYPOINT_FLAG_OPEN;
			}

			/* this neighbor either never got added or was just dropped from the open list... */
			if(!(neighbor->flags & (WAYPOINT_FLAG_OPEN | WAYPOINT_FLAG_CLOSED)))
			{
				nav_open_list[nav_open_list_cursor] = neighbor;
				neighbor->open_list_index = nav_open_list_cursor;

				nav_open_list_cursor++;

				v.x = neighbor->position.x - from.x;
				v.y = neighbor->position.y - from.y;
				v.z = neighbor->position.z - from.z;

				/* save into the neighbor the cost of this route... */
				neighbor->route_cost = cost;
				neighbor->h_cost = dot3(v, v);
				neighbor->flags |= WAYPOINT_FLAG_OPEN;
				neighbor->parent = current;
			}
		}
	}


	return NULL;
}


#ifdef __cplusplus
}
#endif











