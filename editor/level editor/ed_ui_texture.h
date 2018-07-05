#ifndef ED_UI_TEXTURE_H
#define ED_UI_TEXTURE_H



void editor_InitTextureUI();

void editor_OpenTextureWindow();

void editor_CloseTextureWindow();

void editor_ToggleTextureWindow();

void editor_EnumerateTextures();

void editor_OpenTexturePropertiesWindow(int texture_index);

void editor_CloseTexturePropertiesWindow();

void editor_OpenTextureExplorer();

void editor_CloseTextureExplorer();

void editor_TextureWindowExplorerFileClickCallback();

void editor_UpdateTextureUI();


#endif
