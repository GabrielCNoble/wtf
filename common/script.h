#ifndef SCRIPT_H
#define SCRIPT_H

#include "scr_common.h"
#include "script_types/scr_array.h"
#include "script_types/scr_string.h"

#include "angelscript.h"


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

struct script_context_t *script_PopScriptContext();

void script_PushScriptContext();

struct script_context_t *script_GetCurrentContext();

void script_QueueEntryPoint(void *entry_point);

void script_PushArg(void *arg, int arg_type);

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



void script_ExecuteScript(struct script_t *script, void *data);

void script_ExecuteScriptImediate(struct script_t *script, void *data);



void script_RegisterGlobalFunction(char *decl, void *function);

void script_RegisterObjectType(char *decl, int size, int flags);

void script_RegisterObjectBehaviour(char *type, int behaviour, char *decl, void *function);

void script_RegisterObjectProperty(char *type, char *decl, int offset);

void script_RegisterEnum(char *type);

void script_RegisterEnumValue(char *type, char *name, int value);




void *script_GetGlobalVarAddress(char *var, struct script_t *script);

int script_GetFunctionCount(struct script_t *script);

void *script_GetFunctionAddressByIndex(int index, struct script_t *script);

void *script_GetFunctionAddress(char *function, struct script_t *script);

char *script_GetFunctionName(void *function);

void *script_GetTypeInfo(char *type);


int script_GetScriptTypeSize(int type);

int script_GetTypeSize(void *type_info);

int script_GetContextStackTop();

void script_SetCurrentContextData(void *data, int size);

void *script_GetCurrentContextDataPointer();

void script_GetCurrentContextData(void *data, int size);

char *script_GetCurrentFunctionName();

void *script_GetCurrentFunction();


void script_LineCallback();

void script_BeginDebugSection();

void script_EndDebugSection();

//void script_BreakPoint();


#ifdef __cplusplus
}
#endif



#endif
