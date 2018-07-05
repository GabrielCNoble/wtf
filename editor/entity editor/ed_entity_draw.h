#ifndef ED_ENTITY_DRAW_H
#define ED_ENTITY_DRAW_H



void editor_EntityEditorPreDraw();

void editor_EntityEditorPostDraw();

/*
====================================================================
====================================================================
====================================================================
*/ 

void editor_EntityEditorDrawEntityDef();

void editor_EntityEditorDrawColliderDef(int pick);

void editor_EntityEditorDrawGrid();

void editor_EntityEditorDrawCursors();



#endif
