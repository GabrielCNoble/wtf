#ifndef ED_LEVEL_UI_H
#define ED_LEVEL_UI_H

#include "brush.h"


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

void editor_LevelEditorMenuWindow();

void editor_LevelEditorMapCompilerWindow();

void editor_LevelEditorMaterialsWindow();

void editor_LevelEditorLightOptionsMenu();

void editor_LevelEditorBrushOptionsMenu();

void editor_LevelEditorBrushUVMenu();

void editor_LevelEditorEntityOptionsMenu();

void editor_LevelEditorWaypointWindow(int waypoint_index);

void editor_LevelEditorWorldMenu();

void editor_LevelEditorSnapValueMenu();

void editor_LevelEditorSnap3dCursorMenu();

void editor_LevelEditorAddToWorldMenu();

void editor_LevelEditorDeleteSelectionsMenu();

void editor_LevelEditorWaypointOptionsMenu();



void editor_LevelEditorOpenAddToWorldMenu(int x, int y);

void editor_LevelEditorCloseAddToWorldMenu();

void editor_LevelEditorOpenDeleteSelectionsMenu(int x, int y);

void editor_LevelEditorCloseDeleteSelectionsMenu();


void editor_LevelEditorOpenWaypointOptionMenu(int x, int y);

void editor_LevelEditorCloseWaypointOptionMenu();



void editor_LevelEditorOpenSnap3dCursorMenu(int x, int y);

void editor_LevelEditorToggleMaterialsWindow();

void editor_LevelEditorToggleMapCompilerWindow();





#endif
