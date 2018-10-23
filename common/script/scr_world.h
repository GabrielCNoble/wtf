#ifndef SCR_WORLD_H
#define SCR_WORLD_H

#include "script_types/scr_string.h"


#ifdef __cplusplus
extern "C"
{
#endif

void world_ScriptAddWorldVar(struct script_string_t *name, void *type);

void world_ScriptAddWorldArrayVar(struct script_string_t *name, int max_elements, void *type);

void world_ScriptRemoveWorldVar(struct script_string_t *name);


int world_ScriptGetWorldVarSize(struct script_string_t *name);

int world_ScriptGetWorldArrayVarLength(struct script_string_t *name);


void world_ScriptSetWorldVarValue(struct script_string_t *name, void *value);

void world_ScriptGetWorldVarValue(struct script_string_t *name, void *value);

void world_ScriptSetWorldArrayVarValue(struct script_string_t *name, int index, void *value);

void world_ScriptGetWorldArrayVarValue(struct script_string_t *name, int index, void *value);

void world_ScriptAppendWorldArrayVarValue(struct script_string_t *name, void *value);

void world_ScriptClearWorldArrayVar(struct script_string_t *name);


void *world_ScriptGetEntities();

void world_ScriptCallEvent(struct script_string_t *event_name);

void world_ScriptStopCurrentEvent();

void world_ScriptStopAllEvents();



void world_ScriptClearWorld();



#ifdef __cplusplus
}
#endif

#endif // SCR_WORLD_H





