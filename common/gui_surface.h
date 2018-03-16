#ifndef GUI_SURFACE_H
#define GUI_SURFACE_H

#include "gui_common.h"


wsurface_t *gui_CreateSurface(char *name, short x, short y, short w, short h, short bm_flags, void (*surface_callback)(widget_t *));

wsurface_t *gui_AddSurface(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*surface_callback)(widget_t *));

void gui_SaveCurrentStates(widget_t *widget);

void gui_SetUpStates(widget_t *widget);

void gui_RestorePrevStates(widget_t *widget);

void gui_ClearSurface(widget_t *widget);

void gui_UpdateSurface(widget_t *widget);

void gui_PostUpdateSurface(widget_t *widget);

void gui_EnablePicking(widget_t *widget);

void gui_DisablePicking(widget_t *widget);




#endif
