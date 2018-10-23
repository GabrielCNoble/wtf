#include "scr_resource.h"
#include "resource.h"

#ifdef __cplusplus
extern "C"
{
#endif

void resource_ScriptLoadResource(struct script_string_t *name)
{
	char *res_name = script_string_GetRawString(name);
    resource_LoadResource(res_name);
}

#ifdef __cplusplus
}
#endif

