#ifndef ED_LEVEL_UI_H
#define ED_LEVEL_UI_H


void editor_LevelEditorUIInit();

void editor_LevelEditorUIFinish();

void editor_LevelEditorUISetup();

void editor_LevelEditorUIShutdown();

void editor_LevelEditorUpdateUI();


/*
====================================================================
====================================================================
====================================================================
*/

void editor_LevelEditorCloseAllMenus();

void editor_LevelEditorLightOptionsMenu();

void editor_LevelEditorWorldMenu();

void editor_LevelEditorAddToWorldMenu();

void editor_LevelEditorDeleteSelectionsMenu();

 

void editor_LevelEditorOpenAddToWorldMenu(int x, int y);

void editor_LevelEditorCloseAddToWorldMenu();

void editor_LevelEditorOpenDeleteSelectionsMenu(int x, int y);

void editor_LevelEditorCloseDeleteSelectionsMenu();


void editor_LevelEditorOpenWaypointOptionMenu(int x, int y);

void editor_LevelEditorCloseWaypointOptionMenu();



#endif
