#ifndef EVENT_H
#define EVENT_H


#include "script.h"


typedef struct
{
	struct script_t *script;
	char *name;
}event_t;



int event_Init();

void event_Finish();




void event_ExecuteEvent(int event_index);





#endif
