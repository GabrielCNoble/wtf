#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL2\SDL_ttf.h"
#include "SDL2\SDL_syswm.h"
#include "GL/glew.h"

//#include "imgui.h"
//#include "imgui_impl_opengl3.h"

#include "gui_imgui.h"

#include "matrix.h"
#include "gui.h"
#include "font.h"
//#include "renderer.h"
#include "r_main.h"
#include "r_imediate.h"
#include "r_shader.h"
#include "r_gl.h"
#include "input.h"
#include "c_memory.h"
#include "texture.h"
#include "r_imediate.h"
#include "log.h"


//#include "gui_dropdown.h"


/* from r_main.h */
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;
extern int r_imediate_color_shader;
extern int r_gui_shader;
extern SDL_Window *window;

/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern int bm_mouse;
extern int mouse_x;
extern int mouse_y;

widget_t *widgets;
widget_t *last_widget;
widget_t *top_widget;



mat4_t gui_projection_matrix;

gui_var_t *gui_vars = NULL;
gui_var_t *last_gui_var = NULL;

char formated_str[8192];
extern font_t *gui_font;

int gui_widget_unique_index = 0;

widget_t *freed_widgets[WIDGET_LAST];


/*
#ifdef __cplusplus
extern "C++"
{
#endif*/

#include "imgui.h"
struct ImGuiContext *gui_context;


/*
#ifdef __cplusplus
}
#endif*/

/*int gui_draw_cmd_count = 0;
ImDrawCmd *gui_draw_cmds = NULL;
ImDrawIdx *gui_index_buffer = NULL;
ImDrawVert *gui_vertex_buffer = NULL;
int gui_draw_flags = 0;*/


#ifdef __cplusplus
extern "C"
{
#endif


void gui_ImGuiInit()
{
	unsigned char *pixels;
	int width;
	int height;
	unsigned int font_texture;

	gui_context = ImGui::CreateContext(NULL);
	ImGui::SetCurrentContext(gui_context);

	ImGuiIO &io = ImGui::GetIO();

	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	font_texture = renderer_GenGLTexture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0, 1);

	glBindTexture(GL_TEXTURE_2D, font_texture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	io.Fonts->SetTexID((void *)font_texture);

    io.FontDefault = NULL;

	//io.Fonts->TexID = (void *)font_texture;

	ImGui::StyleColorsDark();

	SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;

	io.MousePos.x = 0.0;
	io.MousePos.y = 0.0;

	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
    io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
    io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
    io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
    io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
    io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

	io.DeltaTime = 	1.0 / 60.0;
	io.MouseDoubleClickTime = 0.5;
}

void gui_ImGuiFinish()
{
	ImGui::DestroyContext(gui_context);

	//ImGui_ImplOpenGL3_Shutdown();
}

int gui_Init()
{
	int i;
	widgets = NULL;

	//gui_InitImGui();
	gui_ImGuiInit();

	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	return 1;
}

void gui_Finish()
{
	int i;

	/*while(widgets)
	{
		gui_DestroyWidget(widgets);
	}

	while(gui_vars)
	{
		last_gui_var = gui_vars->next;
		gui_DeleteVar(gui_vars);
		gui_vars = last_gui_var;
	}*/

	gui_ImGuiFinish();

	//gui_FinishImGui();

}


void gui_OpenGuiFrame()
{
	ImGuiIO &io = ImGui::GetIO();

	static float f;
	int i;
	int *text_buffer;
	char *keys;

	static bool p_open;

//	ImVec2 mouse_pos;

	io.DisplaySize.x = r_window_width;
	io.DisplaySize.y = r_window_height;

	io.DisplayFramebufferScale.x = 1.0;
	io.DisplayFramebufferScale.y = 1.0;

	io.MouseDown[0] = false;
	io.MouseDown[1] = false;
	io.MouseDown[2] = false;

	bm_mouse &= ~MOUSE_OVER_WIDGET;

	if(io.WantCaptureMouse)
	{
		bm_mouse |= MOUSE_OVER_WIDGET;

		if(input_GetMouseButton(MOUSE_BUTTON_WHEEL) & MOUSE_WHEEL_UP)
		{
			io.MouseWheel += 1;
		}
		else if(input_GetMouseButton(MOUSE_BUTTON_WHEEL) & MOUSE_WHEEL_DOWN)
		{
			io.MouseWheel -= 1;
		}

		if(input_GetMouseButton(MOUSE_BUTTON_LEFT) & MOUSE_LEFT_BUTTON_CLICKED)
		{
			io.MouseDown[0] = true;
		}

		if(input_GetMouseButton(MOUSE_BUTTON_RIGHT) & MOUSE_RIGHT_BUTTON_CLICKED)
		{
			io.MouseDown[1] = true;
		}

		if(input_GetMouseButton(MOUSE_BUTTON_MIDDLE) & MOUSE_MIDDLE_BUTTON_CLICKED)
		{
			io.MouseDown[2] = true;
		}
	}

	if(io.WantTextInput)
	{
		input_EnableTextInput(1);
		i = 0;

		text_buffer = input_GetTextBuffer();

	//	io.ClearInputCharacters();

		while(text_buffer[i])
		{
			io.AddInputCharacter(text_buffer[i]);
			i++;
		}

		input_ClearTextBuffer();
	}
	else
	{
		io.ClearInputCharacters();
		input_EnableTextInput(0);
	}

	io.MousePos.x = mouse_x;
	io.MousePos.y = r_window_height - mouse_y;

	keys = input_GetKeyArray();
	for(i = SDL_SCANCODE_A; i < SDL_NUM_SCANCODES; i++)
	{
		io.KeysDown[i] = keys[i];
	}

	gui_ImGuiNewFrame();
}

void gui_CloseGuiFrame()
{
	gui_ImGuiRender();
}

void gui_DrawGUI()
{
	ImDrawData *draw_data;
	ImDrawList *draw_list;
	ImDrawCmd *draw_cmd;

	ImDrawVert *vertices;
	ImDrawIdx *indices;


	int indice_offset;

	//draw_data = ImGui::GetDrawData();

	//ImGui_ImplOpenGL3_RenderDrawData(draw_data);



	#if 1

	int i;
	int draw_list_count;
	int j;
	int draw_cmds_count;

	int index;

	int k;

	vec4_t color;

	mat4_t scale = mat4_t_id();

	//CreateOrthographicMatrix(&gui_projection_matrix, -(r_window_width >> 1), r_window_width >> 1, r_window_height >> 1, -(r_window_height >> 1), -10.0, 10.0, NULL);
	CreateOrthographicMatrix(&gui_projection_matrix, 0, r_window_width, 0, r_window_height, -10.0, 10.0, NULL);


	//scale.floats[1][1] = -1.0;
	//scale.floats[3][0] = -r_window_width * 0.5;
	//scale.floats[3][1] = r_window_height * 0.5;

    gui_ImGuiRender();

    //return ;

	renderer_EnableImediateDrawing();
	renderer_SetShader(r_gui_shader);
	renderer_SetProjectionMatrix(&gui_projection_matrix);
	//renderer_SetProjectionMatrix(NULL);
	renderer_SetViewMatrix(&scale);
	//renderer_SetViewMatrix(NULL);
	renderer_SetModelMatrix(NULL);

	glEnable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0);

	draw_data = ImGui::GetDrawData();

	//draw_data->ScaleClipRects(ImVec2(2.0, 2.0));

	draw_list_count = draw_data->CmdListsCount;

	for(i = 0; i < draw_list_count; i++)
	{
		draw_list = draw_data->CmdLists[i];

		draw_cmds_count = draw_list->CmdBuffer.Size;
		vertices = (ImDrawVert *)draw_list->VtxBuffer.Data;
		indices = (ImDrawIdx *)draw_list->IdxBuffer.Data;

		indice_offset = 0;

		for(j = 0; j < draw_cmds_count; j++)
		{
			draw_cmd = &draw_list->CmdBuffer[j];
			renderer_BindTextureTexUnit(GL_TEXTURE0, GL_TEXTURE_2D, (int)draw_cmd->TextureId);
			renderer_SetDefaultUniform1i(UNIFORM_texture_sampler0, 0);

			if(draw_cmd->UserCallback)
			{
				printf("nope\n");
			}
			else
			{
				renderer_Begin(GL_TRIANGLES);
				for(k = 0; k < draw_cmd->ElemCount; k++)
				{
					index = indices[k + indice_offset];

					color.r = (float)(vertices[index].col & 0xff) / 255.0;
					color.g = (float)((vertices[index].col >> 8) & 0xff) / 255.0;
					color.b = (float)((vertices[index].col >> 16) & 0xff) / 255.0;
					color.a = (float)((vertices[index].col >> 24) & 0xff) / 255.0;

					renderer_Color4f(color.r, color.g, color.b, color.a);
					//renderer_Color4f(1.0, 1.0, 1.0, 1.0);
					renderer_TexCoord2f(vertices[index].uv.x, vertices[index].uv.y);
					renderer_Vertex3f(vertices[index].pos.x, vertices[index].pos.y, 0.0);
				}
				renderer_End();

				indice_offset += draw_cmd->ElemCount;
			}

		}

	}

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	renderer_DisableImediateDrawing();

	#endif
}


#ifdef __cplusplus
}
#endif





