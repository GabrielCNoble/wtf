#include "navigation.h"
#include "physics.h"
#include "c_memory.h"
#include "containers/list.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>

//int nav_waypoint_count = 0;
//int nav_max_waypoints = 0;
//int nav_waypoint_stack_top = -1;
//int *nav_waypoint_stack = NULL;
struct waypoint_t nav_temp_end_waypoint;
struct waypoint_t nav_temp_start_waypoint;
//struct waypoint_t *nav_waypoints = NULL;

struct stack_list_t nav_waypoints;

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

	//nav_waypoint_stack = memory_Malloc(sizeof(int) * MAX_WAYPOINTS);
	//nav_waypoints = memory_Malloc(sizeof(struct waypoint_t) * MAX_WAYPOINTS);

	nav_waypoints = stack_list_create(sizeof(struct waypoint_t), MAX_WAYPOINTS, NULL);


	nav_closed_list = memory_Malloc(sizeof(struct waypoint_t *) * MAX_WAYPOINTS);
	nav_open_list = memory_Malloc(sizeof(struct waypoint_t *) * MAX_WAYPOINTS);

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	return 1;
}

void navigation_Finish()
{
	int i;

	/*for(i = 0; i < nav_waypoint_count; i++)
	{
		if(nav_waypoints[i].flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

		memory_Free(nav_waypoints[i].links);
	}*/

	//memory_Free(nav_waypoint_stack);
	//memory_Free(nav_waypoints);

	stack_list_destroy(&nav_waypoints);

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

	/*if(nav_waypoint_stack_top >= 0)
	{
		waypoint_index = nav_waypoint_stack[nav_waypoint_stack_top];
		nav_waypoint_stack_top--;
	}
	else
	{
		waypoint_index = nav_waypoint_count;
		nav_waypoint_count++;
	}*/

	waypoint_index = stack_list_add(&nav_waypoints, NULL);
	waypoint = stack_list_get(&nav_waypoints, waypoint_index);

	//waypoint = nav_waypoints + waypoint_index;


	waypoint->max_links = 16;
	waypoint->links_count = 0;
	waypoint->links = memory_Malloc(sizeof(struct waypoint_link_t) * waypoint->max_links);
	waypoint->position = position;
	waypoint->flags = 0;
	waypoint->parent = NULL;

	//if(!waypoint->name)
	//{

	//}

	//sprintf(name, "waypoint.%d", waypoint_index);
	//waypoint->name = memory_Strdup(name);

	return waypoint_index;
}

void navigation_DestroyWaypoint(int waypoint_index)
{
	int i;
	struct waypoint_t *waypoint;

	waypoint = stack_list_get(&nav_waypoints, waypoint_index);

	if(waypoint)
	{
		if(!(waypoint->flags & WAYPOINT_FLAG_INVALID))
		{
			waypoint->flags |= WAYPOINT_FLAG_INVALID;

			for(i = 0; waypoint->links_count; i++)
			{
				navigation_UnlinkWaypoints(waypoint_index, waypoint->links[i].waypoint_index);
			}

			stack_list_remove(&nav_waypoints, waypoint_index);
		}
	}

}

void navigation_DestroyAllWaypoints()
{
    int i;
    int c;
    struct waypoint_t *waypoints;
    struct waypoint_t *waypoint;


    waypoints = (struct waypoint_t *)nav_waypoints.elements;
    c = nav_waypoints.element_count;

    for(i = 0; i < c; i++)
	{
        waypoint = waypoints + i;

		if(waypoint->flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

		navigation_DestroyWaypoint(i);
	}
}

struct waypoint_t *navigation_GetWaypointPointer(int waypoint_index)
{
	/*if(waypoint_index >= 0 && waypoint_index < nav_waypoint_count)
	{
		if(!(nav_waypoints[waypoint_index].flags & WAYPOINT_FLAG_INVALID))
		{
			return nav_waypoints + waypoint_index;
		}
	}

	return NULL;*/
	return stack_list_get(&nav_waypoints, waypoint_index);
}

void navigation_LinkWaypoints(int waypoint_a, int waypoint_b)
{
	struct waypoint_t *a;
	struct waypoint_t *b;
	vec3_t v;
	float sqrd_cost;

	int i;

	struct waypoint_link_t *link;

	//a = nav_waypoints + waypoint_a;
	//b = nav_waypoints + waypoint_b;

	a = stack_list_get(&nav_waypoints, waypoint_a);
	b = stack_list_get(&nav_waypoints, waypoint_b);

	for(i = 0; i < a->links_count; i++)
	{
		if(a->links[i].waypoint_index == waypoint_b)
		{
			return;
		}
	}

	if(a->links_count >= a->max_links)
	{
		link = memory_Malloc(sizeof(struct waypoint_link_t) * (a->max_links + 16));
		memcpy(link, a->links, sizeof(struct waypoint_link_t) * a->max_links);
		memory_Free(a->links);
		a->links = link;
		a->max_links += 16;
	}

	if(b->links_count >= b->max_links)
	{
		link = memory_Malloc(sizeof(struct waypoint_link_t) * (b->max_links + 16));
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

//	a = nav_waypoints + waypoint_a;
//	b = nav_waypoints + waypoint_b;

	a = stack_list_get(&nav_waypoints, waypoint_a);
	b = stack_list_get(&nav_waypoints, waypoint_b);

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
	/*int i;
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

	}*/

}

void navigation_UpdateWaypoint(int waypoint_index)
{
	int i;
	int j;

	struct waypoint_t *waypoint;
	struct waypoint_t *waypoints;
	struct waypoint_t *linked_waypoint;

	vec3_t v;
	float sqrd_cost;

	waypoint = stack_list_get(&nav_waypoints, waypoint_index);

	if(waypoint)
	{
        if(waypoint->flags & WAYPOINT_FLAG_INVALID)
		{
			return;
		}

		waypoints = (struct waypoint_t *)nav_waypoints.elements;

		for(i = 0; i < waypoint->links_count; i++)
		{
			linked_waypoint = waypoints + waypoint->links[i].waypoint_index;

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

	waypoint = stack_list_get(&nav_waypoints, waypoint_index);

	if(waypoint)
	{
		if(waypoint->flags & WAYPOINT_FLAG_INVALID)
		{
			return;
		}

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

struct waypoint_t *navigation_GetClosestWaypoint(vec3_t position, int ignore_world)
{
	float lowest_d = FLT_MAX;
	struct waypoint_t *closest = NULL;
	struct waypoint_t *waypoints;
	float d;
	vec3_t v;
	int i;
	int c;

	waypoints = (struct waypoint_t *)nav_waypoints.elements;
	c = nav_waypoints.element_count;

	for(i = 0; i < c; i++)
	{
		if(waypoints[i].flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

		v.x = position.x - waypoints[i].position.x;
		v.y = position.y - waypoints[i].position.y;
		v.z = position.z - waypoints[i].position.z;

		d = v.x * v.x + v.y * v.y + v.z * v.z;


		if(d < lowest_d)
		{
			if(!physics_Raycast(waypoints[i].position, position, NULL, NULL, !ignore_world))
			{
				lowest_d = d;
				closest = waypoints + i;
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

	struct waypoint_t *waypoints;


	float cost;
	float new_cost;

	float lowest_f;
	float f;

	float d;
	vec3_t v;

	int i;
	int c;
	int current_index;
	int lowest_index;
	int link_count;

	/* find the path backwards, so the list can
	be constructed in the right direction by
	following the parent pointers... */
	start_point = navigation_GetClosestWaypoint(to, 0);
	end_point = navigation_GetClosestWaypoint(from, 0);

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
	start_point->open_list_index = 0;
	nav_open_list_cursor++;

	waypoints = (struct waypoint_t *)nav_waypoints.elements;
	c = nav_waypoints.element_count;

	for(i = 0; i < c; i++)
	{
		waypoints[i].flags &= ~(WAYPOINT_FLAG_OPEN | WAYPOINT_FLAG_CLOSED);
		waypoints[i].parent = NULL;
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
			nav_open_list[current_index]->open_list_index = current_index;
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

				//if(current->position.x != to.x || current->position.y != to.y || current->position.z != to.z)
				//{
				//	nav_temp_end_waypoint.position = to;
				//	nav_closed_list[nav_closed_list_cursor] = &nav_temp_end_waypoint;
				//	nav_closed_list_cursor++;
				//}
			}

			*waypoint_count = nav_closed_list_cursor;
			return nav_closed_list;
		}

		links = current->links;
		link_count = current->links_count;

		for(i = 0; i < link_count; i++)
		{
			neighbor = waypoints + links[i].waypoint_index;

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

/*
=================================================================
=================================================================
=================================================================
*/



char waypoint_section_start_tag[] = "[waypoint section start]";

struct waypoint_section_start_t
{
	char tag[(sizeof(waypoint_section_start_tag) + 3) & (~3)];
	int waypoint_count;
};

char waypoint_section_end_tag[] = "[waypoint section end]";

struct waypoint_section_end_t
{
    char tag[(sizeof(waypoint_section_end_tag) + 3) & (~3)];
};

char waypoint_record_tag[] = "[waypoint]";

struct waypoint_record_t
{
	char tag[(sizeof(waypoint_record_tag) + 3) & (~3)];
	int  waypoint_index;
    vec3_t position;
};

char waypoint_link_record_tag[] = "[waypoint links]";

struct waypoint_link_record_t
{
    char tag[(sizeof(waypoint_link_record_tag) + 3) & (~3)];
    int link_count;
    int next_offset;
};



void navigation_SerializeWaypoints(void **buffer, int *buffer_size)
{
	struct waypoint_section_start_t *section_start;
	struct waypoint_section_end_t *section_end;
	struct waypoint_record_t *waypoint_record;
	struct waypoint_link_record_t *link_record;

	struct waypoint_record_t *first_waypoint_record;
	struct waypoint_link_record_t *first_waypoint_link_record;

	int i;
	int j;
	int c;

	int out_buffer_size = 0;
	char *out_buffer = NULL;
	int *links;

    struct waypoint_t *waypoints;
    struct waypoint_t *waypoint;
    struct waypoint_t *linked_waypoint;

    waypoints = (struct waypoint_t *) nav_waypoints.elements;
    c = nav_waypoints.element_count;

	out_buffer_size += sizeof(struct waypoint_section_start_t) + sizeof(struct waypoint_section_end_t);

    for(i = 0; i < c; i++)
	{
		waypoint = waypoints + i;

		if(waypoint->flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

		out_buffer_size += sizeof(struct waypoint_record_t);
		out_buffer_size += sizeof(struct waypoint_link_record_t) * waypoint->links_count;
	}

	out_buffer = memory_Calloc(out_buffer_size, 1);

	*buffer = out_buffer;
	*buffer_size = out_buffer_size;


	section_start = (struct waypoint_section_start_t *)out_buffer;
	out_buffer += sizeof(struct waypoint_section_start_t);

	strcpy(section_start->tag, waypoint_section_start_tag);

	first_waypoint_record = (struct waypoint_record_t *)out_buffer;

	for(i = 0; i < c; i++)
	{
        waypoint = waypoints + i;

		if(waypoint->flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

		waypoint_record = (struct waypoint_record_t *)out_buffer;
		out_buffer += sizeof(struct waypoint_record_t);

		strcpy(waypoint_record->tag, waypoint_record_tag);

		waypoint_record->position = waypoint->position;

		/* store the index of the record belonging to this
		waypoint as it will be necessary when generating the
		link records... */
		waypoint->open_list_index = section_start->waypoint_count;

		section_start->waypoint_count++;
	}

	first_waypoint_link_record = (struct waypoint_link_record_t *)out_buffer;

	for(i = 0; i < c; i++)
	{
		waypoint = waypoints + i;

		if(waypoint->flags & WAYPOINT_FLAG_INVALID)
		{
			continue;
		}

        link_record = (struct waypoint_link_record_t *)out_buffer;
        out_buffer += sizeof(struct waypoint_link_record_t);

        strcpy(link_record->tag, waypoint_link_record_tag);

		link_record->next_offset = sizeof(struct waypoint_link_record_t) + sizeof(int) * waypoint->links_count;
		link_record->link_count = waypoint->links_count;

		links = (int *)out_buffer;
		out_buffer += sizeof(int) * waypoint->links_count;

		for(j = 0; j < waypoint->links_count; j++)
		{
			linked_waypoint = waypoints + waypoint->links[j].waypoint_index;
			/* store the waypoint record index as a link... */
			links[j] = linked_waypoint->open_list_index;

			//*(int *)out_buffer = linked_waypoint->open_list_index;
			//out_buffer += sizeof(int);
		}
	}

	section_end = (struct waypoint_section_end_t *)out_buffer;
	out_buffer += sizeof(struct waypoint_section_end_t);

    strcpy(section_end->tag, waypoint_section_end_tag);
}

void navigation_DeserializeWaypoints(void **buffer)
{
	struct waypoint_section_start_t *header;
	struct waypoint_record_t *waypoint_record;
	struct waypoint_record_t *linked_waypoint_record;
	struct waypoint_link_record_t *link_record;

	struct waypoint_record_t *first_waypoint_record = NULL;
	struct waypoint_link_record_t * first_link_record = NULL;
	int *links;

	int header_found = 0;
	int records_found = 0;
	int links_found = 0;

	int i;
	int j;

	char *in;

	in = *buffer;

	while(1)
	{
		if(header_found && records_found && links_found)
		{
			for(i = 0; i < header->waypoint_count; i++)
			{
				/* first create the waypoints, and store their
				indexes into the waypoint records, so they can
				be properly linked...*/
				waypoint_record = first_waypoint_record + i;
				waypoint_record->waypoint_index = navigation_CreateWaypoint(waypoint_record->position);
			}

			for(i = 0; i < header->waypoint_count; i++)
			{
				waypoint_record = first_waypoint_record + i;

				link_record = (struct waypoint_link_record_t *)in;
				in += sizeof(struct waypoint_link_record_t);

				links = (int *)in;
				in += sizeof(int) * link_record->link_count;

				for(j = 0; j < link_record->link_count; j++)
				{
					/* go over the links inside this link record, and
					use the waypoint record it references to get the real
					waypoint index back... */
					linked_waypoint_record = first_waypoint_record + links[j];
					navigation_LinkWaypoints(waypoint_record->waypoint_index, linked_waypoint_record->waypoint_index);
				}
			}

			header_found = 0;
			records_found = 0;
			links_found = 0;
		}
        else if(!strcmp(in, waypoint_section_start_tag))
		{
			header = (struct waypoint_section_start_t *)in;
			in += sizeof(struct waypoint_section_start_t);
			header_found = 1;
		}
		else if(!strcmp(in, waypoint_record_tag))
		{
			first_waypoint_record = (struct waypoint_record_t *)in;
			in += sizeof(struct waypoint_record_t) * header->waypoint_count;
			records_found = 1;
		}
		else if(!strcmp(in, waypoint_link_record_tag))
		{
            first_link_record = (struct waypoint_link_record_t *)in;
            links_found = 1;
		}
		else if(!strcmp(in, waypoint_section_end_tag))
		{
            in += sizeof(struct waypoint_section_end_t);
			break;
		}
		else
		{
			in++;
		}
	}

	*buffer = in;
}


#ifdef __cplusplus
}
#endif











