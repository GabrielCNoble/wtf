#ifndef ED_UI_ENTITY_H
#define ED_UI_ENTITY_H



void editor_InitEntityUI();

void editor_OpenEntityDefViewerWindow();

void editor_CloseEntityDefViewerWindow();

void editor_ToggleEntityDefViewerWindow();

void editor_EnumerateEntityDefs();

void editor_UpdateThumbnail(widget_t *thumbnail);



void editor_OpenEntityDefPropertiesWindow();

void editor_CloseEntityDefPropertiesWindow();


void editor_OpenModelExplorer();

void editor_CloseModelExplorer();

void editor_UpdateEntityUI();


void editor_SpawnEntity(int entity_def_index);





#endif
