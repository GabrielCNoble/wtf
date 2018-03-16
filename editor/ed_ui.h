#ifndef ED_UI_H
#define ED_UI_H




#define MENU_BAR_HEIGHT 20
#define FILE_DROPDOWN_WIDTH 120
#define WORLD_DROPDOWN_WIDTH 220
#define WOW_DROPDOWN_WIDTH 120
#define MISC_DROPDOWN_WIDTH 200

#define FPS_DISPLAY_WIDTH 70
#define HANDLE_3D_MODE_DISPLAY_WIDTH 120


#define LIGHT_PROPERTIES_WINDOW_WIDTH	200
#define LIGHT_PROPERTIES_WINDOW_HEIGHT	350

#define MATERIAL_WINDOW_WIDTH 400
#define MATERIAL_WINDOW_HEIGHT 300

#define BRUSH_FACE_PROPERTIES_WINDOW_WIDTH 200
#define BRUSH_FACE_PROPERTIES_WINDOW_HEIGHT 150

#define UV_WINDOW_WIDTH 500
#define UV_WINDOW_HEIGHT 500

#define EDIT_MATERIAL_WINDOW_RETURN_BUTTON_WIDTH 50
#define EDIT_MATERIAL_WINDOW_RETURN_BUTTON_HEIGHT 30
#define EDIT_MATERIAL_WINDOW_DELETE_MATERIAL_BUTTON_HEIGHT 30


#define TEXTURE_WINDOW_WIDTH 350
#define TEXTURE_WINDOW_HEIGHT 250
#define TEXTURE_PROPERTIES_WINDOW_WIDTH 350
#define TEXTURE_PROPERTIES_WINDOW_HEIGHT 250


#define ENTITY_DEF_VIEWER_WINDOW_WIDTH 600
#define ENTITY_DEF_VIEWER_WINDOW_HEIGHT 400


#define SNAP_VALUE_DROPDOWN_WIDTH 100


void editor_InitUI();

void editor_FinishUI();

void editor_ProcessUI();

void editor_HideUI();

void editor_ShowUI();

void editor_OpenAddToWorldMenu(int x, int y);

void editor_CloseAddToWorldMenu();

void editor_OpenDeleteSelectionMenu(int x, int y);

void editor_OpenSaveProjectWindow();

void editor_CloseSaveProjectWindow();

void editor_OpenOpenProjectWindow();


void editor_OpenLightPropertiesWindow(int light_index);

void editor_CloseLightPropertiesWindow();


/*void editor_OpenMaterialWindow();

void editor_CloseMaterialWindow();

void editor_ToggleMaterialWindow();

void editor_EnumerateMaterials();

void editor_OpenEditMaterialWindow(int material_index);

void editor_CloseEditMaterialWindow();*/

void editor_UIWindowResizeCallback();



#endif
