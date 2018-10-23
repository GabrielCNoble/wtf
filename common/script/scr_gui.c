#include "scr_gui.h"
#include "gui_imgui.h"

#include "vector.h"


extern int r_window_width;
extern int r_window_height;

#ifdef __cplusplus
extern "C"
{
#endif


void gui_ScriptTextWall(struct script_string_t *text, float alpha)
{
    vec2_t text_size;
    vec2_t cursor_pos;

    char *wall_text = script_string_GetRawString(text);

    gui_ImGuiPushFont(gui_ImGuiGetFontIndex(1));
    gui_ImGuiPushStyleVarf(ImGuiStyleVar_Alpha, alpha);
    gui_ImGuiPushStyleColor(ImGuiCol_WindowBg, vec4(0.0, 0.0, 0.0, 1.0));

    gui_ImGuiSetNextWindowSize(vec2(r_window_width, r_window_height), 0);
    gui_ImGuiSetNextWindowPos(vec2(0.0, 0.0), 0, vec2(0.0, 0.0));

    gui_ImGuiBegin("Text Wall", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    text_size = gui_ImGuiCalcTexSize(wall_text, NULL, 0, -1);
    cursor_pos.x = r_window_width / 2 - text_size.x / 2;
    cursor_pos.y = r_window_height / 2 - text_size.y / 2;

    if(cursor_pos.x < 0.0)
    {
        cursor_pos.x = 0.0;
    }

    if(cursor_pos.y < 0.0)
    {
        cursor_pos.y = 0.0;
    }

    //gui_ImGuiSetNextWindowPos(vec2(r_window_width / 2 - text_size.x / 2, r_window_height / 2 - text_size.y / 2), 0, vec2(0.0, 0.0));
    gui_ImGuiSetCursorPos(cursor_pos);
    gui_ImGuiTextWrapped(wall_text);

    gui_ImGuiEnd();
    gui_ImGuiPopStyleVar();
    gui_ImGuiPopStyleColor();
    gui_ImGuiPopFont();
}

#ifdef __cplusplus
}
#endif
