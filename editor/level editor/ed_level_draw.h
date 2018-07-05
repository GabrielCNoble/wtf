#ifndef ED_LEVEL_DRAW_H
#define ED_LEVEL_DRAW_H

#include "..\brush.h"

void editor_LevelEditorPreDraw();

void editor_LevelEditorPostDraw();


void editor_LevelEditorDrawBrushes();

void editor_LevelEditorDrawGrid();
 
void editor_LevelEditorDrawSelected();

void editor_LevelEditorDrawCursors();

void editor_LevelEditorDrawLights();

void editor_LevelEditorDrawSpawnPoints();

void editor_LevelEditorDrawLeaves();

void editor_LevelEditorDrawWorldPolygons();

void editor_LevelEditorDrawClippedPolygons();

void editor_LevelEditorDrawEntitiesAabbs();


void editor_LevelEditorDrawBrush(brush_t *brush);


#endif
