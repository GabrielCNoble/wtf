#ifndef SCRIPT_H
#define SCRIPT_H

#include "scr_common.h"
#include "scr_array.h"
#include "scr_string.h"


#ifdef __cplusplus
extern "C"
{
#endif


int script_Init();

void script_Finish();

void script_RegisterTypesAndFunctions();

void script_ExecuteScripts(double delta_time);

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

struct script_t *script_GetScript(char *script_name);


void script_ReloadScripts();

void script_QueueEntryPoint(void *entry_point);

void script_PushArg(void *arg, int arg_type);

void script_ExecuteScript(struct script_t *script, void *data);

void script_ExecuteScriptImediate(struct script_t *script, void *data);



void script_RegisterGlobalFunction(char *decl, void *function);


void *script_GetGlobalVarAddress(char *var, struct script_t *script);

void *script_GetFunctionAddress(char *function, struct script_t *script);


int script_GetScriptTypeSize(int type);

int script_GetTypeSize(void *type_info);


#ifdef __cplusplus
}
#endif



#endif
