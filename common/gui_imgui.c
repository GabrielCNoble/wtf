#include "gui_imgui.h"
#include <stdio.h>
#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include "texture.h"

#include "r_gl.h"

#include "path.h"


#ifdef __cplusplus
extern "C++"
{
#endif // __cplusplus

static ImGuiContext *gui_context;

#ifdef __cplusplus
}
#endif // __cplusplus





#ifdef __cplusplus
extern "C"
{
#endif

ImGuiContext *gui_CreateContext()
{
	return ImGui::CreateContext(NULL);
}
#if 0
void gui_ImGuiInit()
{
	unsigned char *pixels;
	int width;
	int height;
	unsigned int font_texture;

	gui_context = gui_CreateContext();
	ImGui::SetCurrentContext(gui_context);

	ImGuiIO &io = ImGui::GetIO();

	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	font_texture = texture_GenEmptyGLTexture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0);

	glBindTexture(GL_TEXTURE_2D, font_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	io.Fonts->TexID = (void *)font_texture;

	ImGui::StyleColorsDark();

	SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;

	io.MousePos.x = 0.0;
	io.MousePos.y = 0.0;
}

void gui_ImGuiFinish()
{
	ImGui::DestroyContext(gui_context);
}

#endif

void gui_ImGuiNewFrame()
{
	ImGui::NewFrame();
	ImGuiIO &io = ImGui::GetIO();
}

void gui_ImGuiRender()
{
	ImGui::Render();
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiAddFontFromFileTTF(const char *file_name, float size_pixels)
{
	unsigned int font_texture;

	unsigned char *font_tex_pixels;

	const char *file_path;

	int font_tex_width;
	int font_tex_height;

	ImGuiIO &io = ImGui::GetIO();

	file_path = (const char *)path_GetPathToFile((char *)file_name);

	if(file_path)
	{
		//io.Fonts->Clear();

		io.Fonts->AddFontFromFileTTF(file_path, size_pixels);
		font_texture = (unsigned int )io.Fonts->TexID;

		glDeleteTextures(1, &font_texture);
		font_texture = renderer_GenGLTexture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0);

		io.Fonts->GetTexDataAsRGBA32(&font_tex_pixels, &font_tex_width, &font_tex_height);

		glBindTexture(GL_TEXTURE_2D, font_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, font_tex_width, font_tex_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font_tex_pixels);
		glBindTexture(GL_TEXTURE_2D, 0);

		io.Fonts->TexID = (void *)font_texture;

//		io.FontDefault = NULL;
	}
}


int gui_ImGuiBegin(const char *name, char *open, int flags)
{
	return ImGui::Begin(name, (bool *)open, flags);
}

void gui_ImGuiEnd()
{
	ImGui::End();
}

int gui_ImGuiBeginChild(const char *str_id, vec2_t size, int border, int flags)
{
	return ImGui::BeginChild(str_id, ImVec2(size.x, size.y), border, flags);
}

int gui_ImGuiBeginChildIId(int id, vec2_t size, int border, int flags)
{
	return ImGui::BeginChild(id, ImVec2(size.x, size.y), border, flags);
}

void gui_ImGuiEndChild()
{
	ImGui::EndChild();
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiIsWindowAppearing()
{
	return ImGui::IsWindowAppearing();
}

int gui_ImGuiIsWindowCollapsed()
{
	return ImGui::IsWindowCollapsed();
}

int gui_ImGuiIsWindowFocused(int flags)
{
	return ImGui::IsWindowFocused(flags);
}

int gui_ImGuiIsWindowHovered(int flags)
{
	return ImGui::IsWindowHovered(flags);
}

vec2_t gui_ImGuiGetWindowPos()
{
	ImVec2 pos = ImGui::GetWindowPos();
	return (vec2_t){pos.x, pos.y};
}

vec2_t gui_ImGuiGetWindowSize()
{
	ImVec2 size = ImGui::GetWindowPos();
	return (vec2_t){size.x, size.y};
}

float gui_ImGuiGetWindowWidth()
{
	return ImGui::GetWindowWidth();
}

float gui_ImGuiGetWindowHeight()
{
	return ImGui::GetWindowHeight();
}

vec2_t gui_ImGuiGetContentRegionMax()
{
	ImVec2 size = ImGui::GetContentRegionMax();
	return (vec2_t){size.x, size.y};
}

vec2_t gui_ImGuiGetContentRegionAvail()
{
	ImVec2 size = ImGui::GetContentRegionAvail();
	return (vec2_t){size.x, size.y};
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiSetNextWindowPos(vec2_t pos, int cond, vec2_t pivot)
{
	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), cond, ImVec2(pivot.x, pivot.y));
}

void gui_ImGuiSetNextWindowSize(vec2_t size, int cond)
{
	ImGui::SetNextWindowSize(ImVec2(size.x, size.y), cond);
}

void gui_ImGuiSetNextWindowBgAlpha(float alpha)
{
	ImGui::SetNextWindowBgAlpha(alpha);
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiPushFont(void *font)
{
	ImGui::PushFont((ImFont *)font);
}

void gui_ImGuiPopFont()
{
	ImGui::PopFont();
}

void *gui_ImGuiGetFontIndex(int font_index)
{
    ImGuiIO &io = ImGui::GetIO();

    if(font_index >= 0 && font_index < io.Fonts->Fonts.Size)
	{
		return io.Fonts->Fonts[font_index];
	}

    return NULL;
}

void gui_ImGuiPushStyleColor(int gui_color, vec4_t color)
{
	ImGui::PushStyleColor(gui_color, ImVec4(color.r, color.g, color.b, color.a));
}

void gui_ImGuiPopStyleColor()
{
	ImGui::PopStyleColor();
}

void gui_ImGuiPushStyleVarf(int var, float val)
{
	ImGui::PushStyleVar(var, val);
}

void gui_ImGuiPushStyleVar2f(int var, vec2_t val)
{
	ImGui::PushStyleVar(var, ImVec2(val.x, val.y));
}

void gui_ImGuiPopStyleVar()
{
	ImGui::PopStyleVar();
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiPushItemWidth(float item_width)
{
	ImGui::PushItemWidth(item_width);
}

void gui_ImGuiPopItemWidth()
{
	ImGui::PopItemWidth();
}

/*
===========================================================
===========================================================
===========================================================
*/


void gui_ImGuiSeparator()
{
	ImGui::Separator();
}

void gui_ImGuiSameLine(float pos_x, float spacing_w)
{
	ImGui::SameLine(pos_x, spacing_w);
}

void gui_ImGuiNewLine()
{
	ImGui::NewLine();
}

void gui_ImGuiSpacing()
{
	ImGui::Spacing();
}

void gui_ImGuiIndent(float indent_w)
{
	ImGui::Indent(indent_w);
}

void gui_ImGuiUnindent(float indent_w)
{
	ImGui::Unindent(indent_w);
}

void gui_ImGuiBeginGroup()
{
	ImGui::BeginGroup();
}

void gui_ImGuiEndGroup()
{
	ImGui::EndGroup();
}

vec2_t gui_ImGuiGetCursorPos()
{
	ImVec2 pos = ImGui::GetCursorPos();
	return (vec2_t){pos.x, pos.y};
}

void gui_ImGuiSetCursorPos(vec2_t local_pos)
{
	ImGui::SetCursorPos(ImVec2(local_pos.x, local_pos.y));
}

vec2_t gui_ImGuiGetCursorScreenPos()
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	return (vec2_t){pos.x, pos.y};
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiPushID(const char *str_id)
{
	ImGui::PushID(str_id);
}

void gui_ImGuiPushIDi(int int_id)
{
	ImGui::PushID(int_id);
}

void gui_ImGuiPopID()
{
	ImGui::PopID();
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiTextUnformatted(const char *text, const char *text_end)
{
	ImGui::TextUnformatted(text, text_end);
}

void gui_ImGuiText(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextV(fmt, args);
}

void gui_ImGuiTextV(const char *fmt, va_list args)
{
	ImGui::TextV(fmt, args);
}

void gui_ImGuiTextColored(vec4_t color, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	ImGui::TextColoredV(ImVec4(color.r, color.g, color.b, color.a), fmt, args);
}

void gui_ImGuiTextColoredV(vec4_t color, const char *fmt, va_list args)
{
	ImGui::TextColoredV(ImVec4(color.r, color.g, color.b, color.a), fmt, args);
}

void gui_ImGuiTextDisabled(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextDisabledV(fmt, args);
}

void gui_ImGuiTextDisabledV(const char *fmt, va_list args)
{
	ImGui::TextDisabledV(fmt, args);
}

void gui_ImGuiTextWrapped(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextWrappedV(fmt, args);
}

void gui_ImGuiTextWrappedV(const char *fmt, va_list args)
{
	ImGui::TextWrappedV(fmt, args);
}

void gui_ImGuiLabelText(const char *label, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::LabelTextV(label, fmt, args);
}

void gui_ImGuiLabelTextV(const char *label, const char *fmt, va_list args)
{
	ImGui::LabelTextV(label, fmt, args);
}

void gui_ImGuiBulletText(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::BulletTextV(fmt, args);
}

void gui_ImGuiBulletTextV(const char *fmt, va_list args)
{
	ImGui::BulletTextV(fmt, args);
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiButton(const char *label, vec2_t size)
{
	return ImGui::Button(label, ImVec2(size.x, size.y));
}

int gui_ImGuiSmallButton(const char *label)
{
	return ImGui::SmallButton(label);
}

int gui_ImGuiInvisibleButton(const char *str_id, vec2_t size)
{
	return ImGui::InvisibleButton(str_id, ImVec2(size.x, size.y));
}

int gui_ImGuiArrowButton(const char *str_id, int dir)
{
	return ImGui::ArrowButton(str_id, dir);
}

void gui_ImGuiImage(int texture_id, vec2_t size, vec2_t uv0, vec2_t uv1, vec4_t tint_color, vec4_t border_color)
{
	ImGui::Image((ImTextureID)texture_id, ImVec2(size.x, size.y), ImVec2(uv0.x, uv0.y), ImVec2(uv1.x, uv1.y), ImVec4(tint_color.r, tint_color.g, tint_color.b, tint_color.a), ImVec4(border_color.r, border_color.g, border_color.b, border_color.a));
}

int gui_ImGuiImageButton(int texture_id, vec2_t size, vec2_t uv0, vec2_t uv1, int frame_padding, vec4_t bg_col, vec4_t tint_col)
{
	return ImGui::ImageButton((ImTextureID)texture_id, ImVec2(size.x, size.y), ImVec2(uv0.x, uv0.y), ImVec2(uv1.x, uv1.y), frame_padding, ImVec4(bg_col.r, bg_col.g, bg_col.b, bg_col.a), ImVec4(tint_col.r, tint_col.g, tint_col.b, tint_col.a));
}

int gui_ImGuiCheckbox(const char *label, char *v)
{
	return ImGui::Checkbox(label, (bool *)v);
}

int gui_ImGuiCheckboxFlags(const char *label, unsigned int *flags, int flags_value)
{
	return ImGui::CheckboxFlags(label, flags, flags_value);
}

int gui_ImGuiRadioButton(const char *label, char active)
{
	return ImGui::RadioButton(label, active);
}

void gui_ImGuiProgressBar(float fraction, vec2_t size_arg, const char *overlay)
{
	ImGui::ProgressBar(fraction, ImVec2(size_arg.x, size_arg.y), overlay);
}

void gui_ImGuiBullet()
{
	ImGui::Bullet();
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiBeginCombo(const char *label, const char *preview_value, int flags)
{
	return ImGui::BeginCombo(label, preview_value, flags);
}

void gui_ImGuiEndCombo()
{
	ImGui::EndCombo();
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiDragFloat(const char *label, float *v, float v_speed, float v_min, float v_max, const char *format, float power)
{
	return ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, power);
}

int gui_ImGuiDragFloat2(const char *label, float *v, float v_speed, float v_min, float v_max, const char *format, float power)
{
	return ImGui::DragFloat2(label, v, v_speed, v_min, v_max, format, power);
}

int gui_ImGuiDragFloat3(const char *label, float *v, float v_speed, float v_min, float v_max, const char *format, float power)
{
	return ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format, power);
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiInputText(const char *label, char *buf, int buf_size, int flags)
{
	return ImGui::InputText(label, buf, buf_size, flags, NULL, NULL);
}


/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiSliderFloat(const char *label, float *v, float v_min, float v_max, const char *format, float power)
{
	return ImGui::SliderFloat(label, v, v_min, v_max, format, power);
}

int gui_ImGuiSliderFloat3(const char *label, float *v, float v_min, float v_max, const char *format, float power)
{
	return ImGui::SliderFloat3(label, v, v_min, v_max, format, power);
}

int gui_ImGuiSliderAngle(const char *label, float *v_rad, float v_degrees_min, float v_degrees_max)
{
	return ImGui::SliderAngle(label, v_rad, v_degrees_min, v_degrees_max);
}

int gui_ImGuiSliderInt(const char *label, int *v, int v_min, int v_max, const char *format)
{
	return ImGui::SliderInt(label, v, v_min, v_max, format);
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiTreeNode(const char *str_id, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	return ImGui::TreeNodeV(str_id, fmt, args);
}

int gui_ImGuiTreeNodeEx(const char *str_id, int flags, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	return ImGui::TreeNodeExV(str_id, flags, fmt, args);
}

void gui_ImGuiTreePush(const char *str_id)
{
	ImGui::TreePush(str_id);
}

void gui_ImGuiTreePop()
{
	ImGui::TreePop();
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiSelectable(const char *label, int flags, vec2_t size)
{
	return ImGui::Selectable(label, false, flags, ImVec2(size.x, size.y));
}

int gui_ImGuiListBoxHeader(const char *label, vec2_t size)
{
	return ImGui::ListBoxHeader(label, ImVec2(size.x, size.y));
}

void gui_ImGuiListBoxFooter()
{
	ImGui::ListBoxFooter();
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiBeginMainMenuBar()
{
	return ImGui::BeginMainMenuBar();
}

void gui_ImGuiEndMainMenuBar()
{
	ImGui::EndMainMenuBar();
}

int gui_ImGuiBeginMenuBar()
{
	return ImGui::BeginMenuBar();
}

void gui_ImGuiEndMenuBar()
{
	ImGui::EndMenuBar();
}

int gui_ImGuiBeginMenu(const char *label)
{
	return ImGui::BeginMenu(label);
}

void gui_ImGuiEndMenu()
{
	ImGui::EndMenu();
}

int gui_ImGuiMenuItem(const char *label, const char *shortcut, char *selected, int enabled)
{
	return ImGui::MenuItem(label, shortcut, (bool *)selected, enabled);
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiOpenPopup(const char *str_id)
{
	ImGui::OpenPopup(str_id);
}

int gui_ImGuiBeginPopup(const char *str_id, int flags)
{
	return ImGui::BeginPopup(str_id, flags);
}

void gui_ImGuiEndPopup()
{
	ImGui::EndPopup();
}

int gui_ImGuiIsPopupOpen(const char *str_id)
{
	return ImGui::IsPopupOpen(str_id);
}

void gui_ImGuiCloseCurrentPopup()
{
	ImGui::CloseCurrentPopup();
}

/*
===========================================================
===========================================================
===========================================================
*/

void gui_ImGuiColumns(int count, const char *id, int border)
{
	ImGui::Columns(count, id, border);
}

void gui_ImGuiNextColumn()
{
	ImGui::NextColumn();
}

float gui_ImGuiGetColumnWidth(int column_index)
{
	return ImGui::GetColumnWidth(column_index);
}

float gui_ImGuiGetColumnOffset(int column_index)
{
	return ImGui::GetColumnOffset(column_index);
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiIsItemHovered(int flags)
{
	return ImGui::IsItemHovered(flags);
}

int gui_ImGuiIsItemActive()
{
	return ImGui::IsItemActive();
}

int gui_ImGuiIsItemFocused()
{
	return ImGui::IsItemFocused();
}

int gui_ImGuiIsItemClicked(int mouse_button)
{
	return ImGui::IsItemClicked(mouse_button);
}

int gui_ImGuiIsItemVisible()
{
	return ImGui::IsItemVisible();
}

int gui_ImGuiIsItemDeactivated()
{
	return ImGui::IsItemDeactivated();
}

int gui_ImGuiIsAnyItemActive()
{
	return ImGui::IsAnyItemActive();
}

/*
===========================================================
===========================================================
===========================================================
*/

int gui_ImGuiIsMouseDown(int button)
{
	return ImGui::IsMouseDown(button);
}

int gui_ImGuiIsMouseClicked(int button, int repeat)
{
	return ImGui::IsMouseClicked(button, repeat);
}

int gui_ImGuiIsMouseDoubleClicked(int button)
{
	return ImGui::IsMouseDoubleClicked(button);
}

vec2_t gui_ImGuiGetMouseDragDelta(int button)
{
	ImVec2 drag = ImGui::GetMouseDragDelta(button);
	return (vec2_t){drag.x, drag.y};
}


/*
===========================================================
===========================================================
===========================================================
*/

#ifdef __cplusplus
}
#endif






