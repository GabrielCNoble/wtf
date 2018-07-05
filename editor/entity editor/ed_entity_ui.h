#ifndef ED_ENTITY_UI_H
#define ED_ENTITY_UI_H



void editor_EntityEditorInitUI();

void editor_EntityEditorFinishUI();


void editor_EntityEditorOpenAddComponentMenu(int x, int y);

void editor_EntityEditorCloseAddComponentMenu();

 
void editor_EntityEditorOpenAddColliderPrimitiveMenu(int x, int y);

void editor_EntityEditorCloseAddColliderPrimitiveMenu();

void editor_EntityEditorOpenDestroySelectionMenu(int x, int y);

void editor_EntityEditorCloseDestroySelectionMenu();

#endif
