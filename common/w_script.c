#include "script.h"
#include "w_script.h"
#include "world.h"

#include "ent_common.h"
#include "containers/stack_list.h"
#include "script_types/scr_array.h"

#include "c_memory.h"

//#include "angelscript.h"



extern struct stack_list_t ent_entities[2];


struct script_array_t array_return;

#ifdef __cplusplus
extern "C"
{
#endif

void world_ScriptAddWorldVar(struct script_string_t *name, void *type)
{
    int var_size;
    char *var_name;

    var_size = script_GetTypeSize(type);
    var_name = script_string_GetRawString(name);

    world_AddWorldVar(var_name, var_size);
}

void world_ScriptAddWorldArrayVar(struct script_string_t *name, int max_elements, void *type)
{
    int var_size;
    char *var_name;


    var_size = script_GetTypeSize(type);
    var_name = script_string_GetRawString(name);

    world_AddWorldArrayVar(var_name, var_size, max_elements);
}

void world_ScriptRemoveWorldVar(struct script_string_t *name)
{
    char *var_name;
    var_name = script_string_GetRawString(name);
    world_RemoveWorldVar(var_name);
}

int world_ScriptGetWorldVarSize(struct script_string_t *name)
{
	return 0;
}

int world_ScriptGetWorldArrayVarLength(struct script_string_t *name)
{
	char *var_name;
	struct world_var_t *world_var;

	var_name = script_string_GetRawString(name);
	world_var = world_GetWorldVarPointer(var_name);

    if(world_var)
	{
        return world_var->element_count;
	}
}







void world_ScriptSetWorldVarValue(struct script_string_t *name, void *value)
{
    char *var_name;

    var_name = script_string_GetRawString(name);

    world_SetWorldVarValue(var_name, value);
}

void world_ScriptGetWorldVarValue(struct script_string_t *name, void *value)
{
    char *var_name;

    var_name = script_string_GetRawString(name);

    world_GetWorldVarValue(var_name, value);
}

void world_ScriptSetWorldArrayVarValue(struct script_string_t *name, int index, void *value)
{
    char *var_name;

    var_name = script_string_GetRawString(name);

    world_SetWorldArrayVarValue(var_name, value, index);
}

void world_ScriptGetWorldArrayVarValue(struct script_string_t *name, int index, void *value)
{
    char *var_name;

    var_name = script_string_GetRawString(name);

    world_GetWorldArrayVarValue(var_name, value, index);
}

void world_ScriptAppendWorldArrayVarValue(struct script_string_t *name, void *value)
{
    char *var_name;

    var_name = script_string_GetRawString(name);

    world_AppendWorldArrayVarValue(var_name, value);
}

void world_ScriptClearWorldArrayVar(struct script_string_t *name)
{
	char *var_name;

	var_name = script_string_GetRawString(name);

	world_ClearWorldArrayVar(var_name);
}


void *world_ScriptGetEntities()
{
	int i;

	struct entity_handle_t *entity_handles;
	struct entity_t *entities;

    if(!array_return.buffer)
	{
        array_return.buffer = memory_Calloc(512, sizeof(struct entity_handle_t));
	}

	array_return.element_count = 0;
	entities = (struct entity_t *)ent_entities[0].elements;
	entity_handles = (struct entity_handle_t *)array_return.buffer;

	for(i = 0; i < ent_entities[0].element_count; i++)
	{
        if(entities[i].flags & ENTITY_FLAG_INVALID)
		{
			continue;
		}

		entity_handles[array_return.element_count].def = 0;
		entity_handles[array_return.element_count].entity_index = i;

		array_return.element_count++;
	}

	array_return.element_size = sizeof(struct entity_handle_t);
	array_return.extern_buffer = 1;
	array_return.extern_array = 1;

	return &array_return;
}


void world_ScriptCallEvent(struct script_string_t *event_name)
{
	char *name = script_string_GetRawString(event_name);

	world_CallEvent(name);
}


void world_ScriptStopCurrentEvent()
{
    char *event_name;
    struct world_script_t *world_script;
    void *current_function;
    int i;

    world_script = world_GetWorldScript();

    if(world_script)
	{
		current_function = script_GetCurrentFunction();

		for(i = 0; i < world_script->event_count; i++)
		{
            if(current_function == world_script->events[i].event_function)
			{
                world_StopEventIndex(i);
                break;
			}
		}
	}
}


void world_ScriptStopAllEvents()
{
	world_StopAllEvents();
}



void world_ScriptClearWorld()
{
    world_Clear(WORLD_CLEAR_FLAG_ALL);
}


#ifdef __cplusplus
}
#endif



