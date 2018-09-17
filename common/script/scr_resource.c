#include "scr_resource.h"
#include "resource.h"



void resource_ScriptLoadResource(struct script_string_t *name)
{
	char *res_name = script_string_GetRawString(name);
    resource_LoadResource(res_name);
}


