#ifndef GUI_IMGUI
#define GUI_IMGUI

//#define GUI_MODULE_HACK

#ifdef GUI_MODULE_HACK

struct ImVec2
{
	float x, y;
};

struct ImVec4
{
	float x, y, z, w;
};

#endif /* GUI_MODULE_HACK */


#include "imgui.h"
#include "vector.h"

#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

//void gui_ImGuiInit();

//void gui_ImGuiFinish();

void gui_ImGuiNewFrame();

void gui_ImGuiRender();

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiBegin(const char *name, char *open, int flags);

void gui_ImGuiEnd();

int gui_ImGuiBeginChild(const char *str_id, vec2_t size, int border, int flags);

void gui_ImGuiEndChild();

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiIsWindowAppearing();

int gui_ImGuiIsWindowCollapsed();

int gui_ImGuiIsWindowFocused(int flags);

int gui_ImGuiIsWindowHovered(int flags);

vec2_t gui_ImGuiGetWindowPos();

vec2_t gui_ImGuiGetWindowSize();

float gui_ImGuiGetWindowWidth();

float gui_ImGuiGetWindowHeight();

vec2_t gui_ImGuiGetContentRegionMax();

vec2_t gui_ImGuiGetContentRegionAvail();

/*
===========================================================
===========================================================
===========================================================
*/


void gui_ImGuiSetNextWindowPos(vec2_t pos, int cond, vec2_t pivot);

void gui_ImGuiSetNextWindowSize(vec2_t size, int cond);

void gui_ImGuiSetNextWindowBgAlpha(float alpha);

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiPushStyleColor(int gui_color, vec4_t color);

void gui_ImGuiPopStyleColor();

void gui_ImGuiPushStyleVarf(int var, float val);

void gui_ImGuiPushStyleVar2f(int var, vec2_t val);

void gui_ImGuiPopStyleVar();

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiPushItemWidth(float item_width);

void gui_ImGuiPopItemWidth();

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiSeparator();

void gui_ImGuiSameLine(float pos_x, float spacing_w);

void gui_ImGuiNewLine();

void gui_ImGuiSpacing();

void gui_ImGuiIndent(float indent_w);

void gui_ImGuiUnindent(float indent_w);

vec2_t gui_ImGuiGetCursorPos();

void gui_ImGuiSetCursorPos(vec2_t local_pos);

vec2_t gui_ImGuiGetCursorScreenPos();

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiPushID(const char *str_id);

void gui_ImGuiPopID();

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiTextUnformatted(const char *text, const char *text_end);

void gui_ImGuiText(const char *fmt, ...);

void gui_ImGuiTextV(const char *fmt, va_list args);

void gui_ImGuiTextColored(vec4_t color, const char *fmt, ...);

void gui_ImGuiTextColoredV(vec4_t color, const char *fmt, va_list args);

void gui_ImGuiTextDisabled(const char *fmt, ...);

void gui_ImGuiTextDisabledV(const char *fmt, va_list args);

void gui_ImGuiTextWrapped(const char *fmt, ...);

void gui_ImGuiTextWrappedV(const char *fmt, va_list args);

void gui_ImGuiLabelText(const char *label, const char *fmt, ...);

void gui_ImGuiLabelTextV(const char *label, const char *fmt, va_list args);

void gui_ImGuiBulletText(const char *fmt, ...);

void gui_ImGuiBulletTextV(const char *fmt, va_list args);

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiButton(const char *label, vec2_t size);

int gui_ImGuiSmallButton(const char *label);

int gui_ImGuiInvisibleButton(const char *str_id, vec2_t size);

int gui_ImGuiArrowButton(const char *str_id, int dir);

void gui_ImGuiImage(int texture_id, vec2_t size, vec2_t uv0, vec2_t uv1, vec4_t tint_color, vec4_t border_color);

int gui_ImGuiImageButton(int texture_id, vec2_t size, vec2_t uv0, vec2_t uv1, int frame_padding, vec4_t bg_col, vec4_t tint_col);

int gui_ImGuiCheckbox(const char *label, char *v);

int gui_ImGuiCheckboxFlags(const char *label, unsigned int *flags, int flags_value);

int gui_ImGuiRadioButton(const char *label, char active);

void gui_ImGuiProgressBar(float fraction, vec2_t size_arg, const char *overlay);

void gui_ImGuiBullet();

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiBeginCombo(const char *label, const char *preview_value, int flags);

void gui_ImGuiEndCombo();

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiDragFloat(const char *label, float *v, float v_speed, float v_min, float v_max, const char *format, float power);

int gui_ImGuiDragFloat2(const char *label, float *v, float v_speed, float v_min, float v_max, const char *format, float power);

int gui_ImGuiDragFloat3(const char *label, float *v, float v_speed, float v_min, float v_max, const char *format, float power);

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiInputText(const char *label, char *buf, int buf_size, int flags);

/*
===========================================================
===========================================================
===========================================================
*/


int gui_ImGuiSliderFloat(const char *label, float *v, float v_min, float v_max, const char *format, float power);

int gui_ImGuiSliderFloat3(const char *label, float *v, float v_min, float v_max, const char *format, float power);

int gui_ImGuiSliderAngle(const char *label, float *v_rad, float v_degrees_min, float v_degrees_max);

int gui_ImGuiSliderInt(const char *label, int *v, int v_min, int v_max, const char *format);

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiTreeNode(const char *str_id, const char *fmt, ...);

int gui_ImGuiTreeNodeEx(const char *str_id, int flags, const char *fmt, ...);

void gui_ImGuiTreePush(const char *str_id);

void gui_ImGuiTreePop();

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiSelectable(const char *label, int flags, vec2_t size);

int gui_ImGuiListBoxHeader(const char *label, vec2_t size);

void gui_ImGuiListBoxFooter();

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiBeginMainMenuBar();

void gui_ImGuiEndMainMenuBar();

int gui_ImGuiBeginMenuBar();

void gui_ImGuiEndMenuBar();

int gui_ImGuiBeginMenu(const char *label);

void gui_ImGuiEndMenu();

int gui_ImGuiMenuItem(const char *label, const char *shortcut, char *selected, int enabled);

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiOpenPopup(const char *str_id);

int gui_ImGuiBeginPopup(const char *str_id, int flags);

void gui_ImGuiEndPopup();

int gui_ImGuiIsPopupOpen(const char *str_id);

void gui_ImGuiCloseCurrentPopup();

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiColumns(int count, const char *id, int border);

void gui_ImGuiNextColumn();

float gui_ImGuiGetColumnWidth(int column_index);

float gui_ImGuiGetColumnOffset(int column_index);

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiIsItemHovered(int flags);

int gui_ImGuiIsItemActive();

int gui_ImGuiIsItemFocused();

int gui_ImGuiIsItemClicked(int mouse_button);

int gui_ImGuiIsItemVisible();

int gui_ImGuiIsItemDeactivated();


/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiIsMouseDown(int button);

int gui_ImGuiIsMouseClicked(int button, int repeat);

int gui_ImGuiIsMouseDoubleClicked(int button);

vec2_t gui_ImGuiGetMouseDragDelta(int button);

/*
===========================================================
===========================================================
===========================================================
*/

#ifdef __cplusplus
}
#endif


#endif






