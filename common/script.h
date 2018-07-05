#ifndef SCRIPT_H
#define SCRIPT_H

#include "scr_common.h"
#include "scr_types.h"


#ifdef __cplusplus
extern "C"
{
#endif


int script_Init();

void script_Finish();

void script_RegisterTypesAndFunctions();

/*
=========================================================
=========================================================
=========================================================
*/

void *script_PopScriptContext();

void script_PushScriptContext(void *script_context);

/*
=========================================================
=========================================================
=========================================================
*/

struct script_t *script_CreateScript(char *file_name, char *script_name, int script_type_size, int (*get_data_callback)(struct script_t *script), void *(*setup_data_callback)(struct script_t *script, void *data));

struct script_t *script_LoadScript(char *file_name, char *script_name, int script_type_size, int (*get_data_callback)(struct script_t *script), void *(*setup_data_callback)(struct script_t *script, void *data));

char *script_LoadScriptSource(char *file_name);

int script_CompileScriptSource(char *source, struct script_t *script);


void script_ReloadScripts();

void script_ExecuteScriptImediate(struct script_t *script, void *data);



void *script_GetGlobalVarAddress(char *var, struct script_t *script);

void *script_GetFunctionAddress(char *function, struct script_t *script);



#ifdef __cplusplus
}
#endif



#endif
