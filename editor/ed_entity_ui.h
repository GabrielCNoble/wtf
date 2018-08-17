#ifndef ED_ENTITY_UI_H
#define ED_ENTITY_UI_H

#include "ent_common.h"

void editor_EntityEditorInitUI();

void editor_EntityEditorFinishUI();


void editor_EntityEditorUpdateUI();



void editor_EntityEditorCloseAllMenus();

void editor_EntityEditorAddComponentMenu();

void editor_EntityEditorPropMenu();

void editor_EntityEditorSetComponentValueMenu();

void editor_EntityEditorDefsMenu();

void editor_EntityEditorDefTree();

/*
====================================================
====================================================
====================================================
*/





void editor_EntityEditorOpenAddComponentMenu(int x, int y, struct entity_handle_t entity, struct component_handle_t transform);

void editor_EntityEditorOpenAddPropMenu(int x, int y, struct entity_handle_t entity);

void editor_EntityEditorOpenSetComponentValueMenu(int x, int y, struct entity_handle_t entity, int component_type);

void editor_EntityEditorToggleDefsMenu();


//void editor_EntityEditorCloseAddComponentMenu();


//void editor_EntityEditorOpenAddColliderPrimitiveMenu(int x, int y);

//void editor_EntityEditorCloseAddColliderPrimitiveMenu();

//void editor_EntityEditorOpenDestroySelectionMenu(int x, int y);

//void editor_EntityEditorCloseDestroySelectionMenu();

#endif
