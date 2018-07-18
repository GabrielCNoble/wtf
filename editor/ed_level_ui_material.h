#ifndef ED_UI_MATERIAL
#define ED_UI_MATERIAL


void editor_InitMaterialUI();

void editor_OpenMaterialWindow();

void editor_CloseMaterialWindow();

void editor_ToggleMaterialWindow();

void editor_EnumerateMaterials();

void editor_OpenEditMaterialWindow(int material_index);

void editor_CloseEditMaterialWindow();

void editor_OpenEditMaterialWindowTextureExplorer();

void editor_CloseEditMaterialWindowTextureExplorer();

void editor_OpenEditMaterialWindowDiffuseTextureExplorer();

void editor_OpenEditMaterialWindowNormalTextureExplorer();

void editor_UpdateMaterialUI();



#endif
