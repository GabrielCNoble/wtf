#include "scr_world.h"
#include "world.h"
//#include "angelscript.h"


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






