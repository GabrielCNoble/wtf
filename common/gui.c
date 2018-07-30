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
#include "r_shader.h"
#include "r_gl.h"
#include "input.h"
#include "memory.h"
#include "texture.h"
#include "r_imediate.h"


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



#ifdef __cplusplus
extern "C++"
{
#endif

#include "imgui.h"
extern ImGuiContext *gui_context;

#ifdef __cplusplus
}
#endif
	
int gui_draw_cmd_count = 0;
ImDrawCmd *gui_draw_cmds = NULL;
ImDrawIdx *gui_index_buffer = NULL;
ImDrawVert *gui_vertex_buffer = NULL;
int gui_draw_flags = 0;	


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
	
	font_texture = texture_GenGLTexture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, GL_REPEAT, 0, 0);

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
	return 1;
}

void gui_Finish()
{
	int i;
	
	while(widgets)
	{
		gui_DestroyWidget(widgets);
	}
	
	while(gui_vars)
	{
		last_gui_var = gui_vars->next;
		gui_DeleteVar(gui_vars);
		gui_vars = last_gui_var;
	}
	
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
	//struct ImDrawList *draw_list;
	
	//ImGui::EndFrame();
//	ImGui::Render();

	gui_ImGuiRender();
	
	//draw_list = ImGui::GetDrawData();
	
	//gui_draw_cmd_count = draw_list->CmdBuffer.Size;
	//gui_draw_cmds = (struct ImDrawCmd *)draw_list->CmdBuffer.Data;
	//gui_index_buffer = (struct ImDrawIdx *)draw_list->IdxBuffer.Data;
	//gui_vertex_buffer = (struct ImDrawVert *)draw_list->VtxBuffer.Data;
}


widget_t *gui_CreateWidget(char *name, short x, short y, short w, short h, int type)
{
	widget_t *widget;
	int name_len;
	unsigned int size;
	
	
	switch(type)
	{
		case WIDGET_BASE:
			size = sizeof(widget_t);
		break;
		
		case WIDGET_BUTTON:
			size = sizeof(button_t);
		break;
		
		case WIDGET_CHECKBOX:
			size = sizeof(checkbox_t);	
		break;
			
		case WIDGET_SLIDER:
			size = sizeof(slider_t);	
		break;
			
		case WIDGET_DROPDOWN:
			size = sizeof(dropdown_t);	
		break;
		
		case WIDGET_OPTION_LIST:
			size = sizeof(option_list_t);	
		break;
			
		case WIDGET_OPTION:
			size = sizeof(option_t);	
		break;
		
		case WIDGET_BAR:
			size = sizeof(widget_bar_t);	
		break;
			
		case WIDGET_TEXT_FIELD:
			size = sizeof(text_field_t);	
		break;
			
		case WIDGET_SURFACE:
			size = sizeof(wsurface_t);
		break;
			
		case WIDGET_ITEM_LIST:
			size = sizeof(item_list_t);
		break;
		
		case WIDGET_ITEM_TREE:
			size = sizeof(item_tree_t);
		break;
		
		default:
			return NULL;
	}
	
	
	//widget = malloc(sizeof(widget_t));
	//widget = malloc(size);
	widget = (widget_t *)memory_Malloc(size, "gui_CreateWidget");
	
	if(w < WIDGET_MIN_SIZE) w = WIDGET_MIN_SIZE;
	if(h < WIDGET_MIN_SIZE) h = WIDGET_MIN_SIZE;
	
	widget->last_nestled = NULL;
	widget->nestled = NULL;
	widget->next = NULL;
	widget->prev = NULL;
	widget->x = x;
	widget->y = y;
	widget->w = w / 2;
	widget->h = h / 2;
	widget->bm_flags = 0;
	widget->edge_flags = 0;
	widget->jusitication_flags = 0;
	widget->type = type;
	widget->parent = NULL;
	
	//widget->left_edge_of = NULL;
	//widget->bottom_edge_of = NULL;
	widget->linked_edges = NULL;
	//widget->first_parent = NULL;
	widget->widget_callback = NULL;
	widget->process_callback = NULL;
	widget->gl_tex_handle = 0;
	
	//widget->name = strdup(name);
	
	//widget->name = malloc(WIDGET_NAME_MAX_LEN);
	widget->name = (char *)memory_Malloc(WIDGET_NAME_MAX_LEN, "gui_CreateWidget");
	widget->name[0] = '\0';
	
	if(name)
	{
		name_len = strlen(name) + 1;
		
		if(name_len > WIDGET_NAME_MAX_LEN)
		{
			name_len = WIDGET_NAME_MAX_LEN;
		}
		
		memcpy(widget->name, name, name_len);
	}

	widget->rendered_name = NULL;
	widget->unique_index = gui_widget_unique_index++;
	
	return widget;
}

widget_t *gui_AddWidget(widget_t *parent, char *name, short x, short y, short w, short h)
{
	widget_t *widget;
	
	
	widget = gui_CreateWidget(name, x, y, w, h, WIDGET_BASE);
	
	
	if(parent)
	{
		if(!parent->nestled)
		{
			parent->nestled = widget;
		}	
		else
		{
			parent->last_nestled->next = widget;
			widget->prev = parent->last_nestled;
		}
		
		parent->last_nestled = widget;
		widget->parent = parent;
	}
	else
	{
		if(!widgets)
		{
			widgets = widget;
		}	
		else
		{
			last_widget->next = widget;
			widget->prev = last_widget;
		}
		
		last_widget = widget;
	}
	
	return widget;
	
}

void gui_NestleWidget(widget_t *parent, widget_t *widget)
{
	widget_t **first;
	widget_t **last;
	
	
	if(!parent)
	{
		first = &widgets;
		last = &last_widget;
	}
	else
	{
		first = &parent->nestled;
		last = &parent->last_nestled;
	}
	
	
	if(!(*first))
	{
		if(!parent)
		{
			top_widget = widget;
		}
		*first = widget;
	}
	else
	{
		(*last)->next = widget;
		widget->prev = *last;
	}
	
	
	*last = widget;
	
}

void gui_DestroyWidget(widget_t *widget)
{
	widget_t *w;
	widget_t *r;
	widget_t *parent;
	text_field_t *field;
	wsurface_t *surface;
	dropdown_t *dropdown;
	option_list_t *option_list;
	option_t *option;
	linked_edge_t *linked_edges;
	linked_edge_t *next_linked_edge;
	
	w = widget->nestled;
	
	while(w)
	{
		r = w->next;
		gui_DestroyWidget(w);
		w = r;
	}
		
	if(!widget->parent)
	{
		if(widget == widgets)
		{
			widgets = widget->next;
			
			if(widgets)
			{
				widgets->prev = NULL;
			}	
		}
		else
		{
			widget->prev->next = widget->next;
			
			if(widget->next)
			{
				widget->next->prev = widget->prev;
			}
			else
			{
				last_widget = last_widget->prev;
			}
		}
	}
	else
	{
		parent = widget->parent;
			
		if(widget == parent->nestled)
		{
			parent->nestled = widget->next;
			
			if(parent->nestled)
			{
				parent->nestled->prev = NULL;
			}
					
		}
		else
		{
			widget->prev->next = widget->next;	
		}
			
		if(widget->next)
		{
			widget->next->prev = widget->prev;
		}
		else
		{
			parent->last_nestled = widget->prev;
		}
	}
	
	_free_widget:
	
	if(widget->name)
	{
		memory_Free(widget->name);
	}
	
	if(widget->rendered_name)
	{
		SDL_FreeSurface(widget->rendered_name);
	}
	
	linked_edges = widget->linked_edges;
	while(linked_edges)
	{
		next_linked_edge = linked_edges->next;
		memory_Free(linked_edges);
		linked_edges = next_linked_edge;
	}
		
	switch(widget->type)
	{
		case WIDGET_SURFACE:
			surface = (wsurface_t *)widget;
			glDeleteTextures(1, &surface->color_texture);
			glDeleteTextures(1, &surface->depth_texture);
			glDeleteFramebuffers(1, &surface->framebuffer_id);
		break;
		
		case WIDGET_TEXT_FIELD:
			field = (text_field_t *)widget;
			if(field->text)
			{
				memory_Free(field->text);
			}
			
			if(field->rendered_text)
			{
				SDL_FreeSurface(field->rendered_text);
			}
		break;
		
		case WIDGET_DROPDOWN:
			dropdown = (dropdown_t *)widget;
			
			if(dropdown->dropdown_text)
			{
				memory_Free(dropdown->dropdown_text);
			}
			
			if(dropdown->rendered_text)
			{
				SDL_FreeSurface(dropdown->rendered_text);
			}
		break;
		
		case WIDGET_OPTION_LIST:
			option_list = (option_list_t *)widget;
			w = option_list->first_unused;
			
			while(w)
			{
				r = w->next;
				gui_DestroyWidget(w);
				w = r;
			}
			
		break;
		
		case WIDGET_OPTION:
			option = (option_t *)widget;
			
			w = option->widget.nestled;
			
			while(w)
			{
				r = w->next;
				gui_DestroyWidget(w);
				w = r;
			}
			
			/* this is causing the engine to
			crash when shutting down. Probable
			memory corruption when freeing stuff
			before this. FIX IT! */
			//memory_Free(option->option_text);
		break;
	}
		
	
	memory_Free(widget);	
}

void gui_GetAbsolutePosition(widget_t *widget, short *x, short *y)
{
	widget_t *w;
	
	*x = widget->x;
	*y = widget->y;
	
	w = widget->parent;
	
	while(w)
	{
		*x += w->x;
		*y += w->y;
		
		w = w->parent;
	}
}

void gui_SetAsTop(widget_t *widget)
{
	widget_t **first;
	widget_t **last;
	
	if(widget)
	{
		if(widget->bm_flags & WIDGET_NOT_AS_TOP)
			return;
		
		if(widget->parent)
		{			
			first = &widget->parent->nestled;
			last = &widget->parent->last_nestled;			
		}
		else
		{
			first = &widgets;
			last = &last_widget;
		}
		
		/* widget is already first in the list (or the only one in the list)... */
		if(!widget->prev)
		{
			return;
		}
			
		/* by here there will be at least two widgets in the list, 
		and this widget will be either in the middle or in the end... */
		widget->prev->next = widget->next;
			
		if(widget->next)
		{
			/* middle... */
			widget->next->prev = widget->prev;
		}
		else
		{
			/* last... */
			*last = widget->prev;
		}
			
		widget->prev = NULL;
		widget->next = *first;
		(*first)->prev = widget;
		*first = widget;		
	}
}


int stack_top = -1;
widget_t *stack[512];


void gui_SetVisible(widget_t *widget)
{
	widget->bm_flags &= ~WIDGET_INVISIBLE;
	
	widget = widget->nestled;
	
	
	/* propagate the flag downwards... */
	while(widget)
	{
		widget->bm_flags |= WIDGET_JUST_CREATED;
		
		if(widget->nestled)
		{
			stack_top++;
			stack[stack_top] = widget;
			
			widget = widget->nestled;
			continue;
		}
		
		_advance_widget:
		
		widget = widget->next;
		
		if(!widget)
		{
			if(stack_top >= 0)
			{
				widget = stack[stack_top]; 
				stack_top--;
				goto _advance_widget;
			}
		}
			
	}
}

void gui_SetInvisible(widget_t *widget)
{
	widget->bm_flags |= WIDGET_INVISIBLE;
	
	widget->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | 
						 WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON | WIDGET_OUT_OF_BOUNDS | 
						 WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK | WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP | 
						 WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN);
}

void gui_SetIgnoreMouse(widget_t *widget)
{
	widget_t *w;
	if(widget)
	{
		widget->bm_flags |= WIDGET_DONT_RECEIVE_MOUSE;
		
		w = widget->nestled;
		while(w)
		{
			gui_SetIgnoreMouse(w);
			w = w->next;
		}
	}
}

void gui_SetReceiveMouse(widget_t *widget)
{
	widget_t *w;
	if(widget)
	{
		widget->bm_flags &= ~WIDGET_DONT_RECEIVE_MOUSE;
		
		w = widget->nestled;
		while(w)
		{
			gui_SetReceiveMouse(w);
			w = w->next;
		}
	}
}


void gui_RenderText(widget_t *widget)
{
	option_t *option;
	dropdown_t *dropdown;
	text_field_t *field;
	button_t *button;
	SDL_Color foreground = {255, 255, 255, 255};
	SDL_Color background = {0, 0, 0, 0};
	
	char *format;
	
	if(!gui_font)
		return;
		
			
	switch(widget->type)
	{
		case WIDGET_OPTION:
			option = (option_t *)widget;
			
			if(!option->option_text)
				return;
			
			if(widget->bm_flags & WIDGET_RENDER_TEXT)
			{
				if(option->rendered_text)
				{
					SDL_FreeSurface(option->rendered_text);
				}
				
				if(option->bm_option_flags & OPTION_INVALID)
				{
					foreground.g = 0;
					foreground.b = 0;
				}
				
				sprintf(formated_str, option->option_text);
				
				//option->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, option->widget.w * 2.0);
				option->rendered_text = TTF_RenderUTF8_Blended(gui_font->font, formated_str, foreground);
				option->rendered_string = font_RenderString(gui_font, formated_str, 0, vec4(1.0, 1.0, 1.0, 1.0), 1);
			}
			
			if(widget->bm_flags & WIDGET_RENDER_NAME)
			{
				
			}
			
		break;
		
		case WIDGET_DROPDOWN:
			dropdown = (dropdown_t *)widget;
			if(!dropdown->dropdown_text)
				return;
				
			if(dropdown->rendered_text)
				SDL_FreeSurface(dropdown->rendered_text);
			
			sprintf(formated_str, dropdown->dropdown_text);
			
			dropdown->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, dropdown->widget.w * 2.0);	
		break;
		
		case WIDGET_TEXT_FIELD:
			field = (text_field_t *)widget;
			
			if(field->text)
			{
					
				if(field->bm_text_field_flags & TEXT_FIELD_DRAW_TEXT_SELECTED)
				{
					foreground.r = 0;
					foreground.g = 0;
					foreground.b = 0;
				}
						
				//if(field->rendered_text)
				//	SDL_FreeSurface(field->rendered_text);
						
				sprintf(formated_str, field->text);
				field->rendered_string = font_RenderString(gui_font, formated_str, 0, vec4(1.0, 1.0, 1.0, 1.0), 0);
				//field->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, field->widget.w * 2.0);
						
			}
			
		break;
		
		case WIDGET_BUTTON:
			
			button = (button_t *)widget;
			
			if(button->rendered_text)
				SDL_FreeSurface(button->rendered_text);
				
			
			if(button->bm_button_flags & BUTTON_DONT_RECEIVE_CLICK)
			{
				/*foreground.r = 255;
				foreground.g = 200;
				foreground.b = 200;
				foreground.a = 255;*/
			}	
			
			
			sprintf(formated_str, button->button_text);
			
			button->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, button->widget.w * 2.0);	
				
		break;
	}
	
	if(widget->bm_flags & WIDGET_RENDER_NAME)
	{
		foreground.r = 255;
		foreground.g = 255;
		foreground.b = 255;
			
		if(widget->rendered_name)
			SDL_FreeSurface(widget->rendered_name);
			
		widget->rendered_name = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, widget->name, foreground, widget->w * 2.0);		
	}
	
	
	widget->bm_flags &= ~(WIDGET_RENDER_TEXT | WIDGET_RENDER_NAME);
}


gui_var_t *gui_CreateVar(char *name, short type, void *addr, void *refresh_base, int offset)
{
	gui_var_t *var = NULL;
	
	switch(type)
	{
		case GUI_VAR_MAT4_T:
		case GUI_VAR_MAT3_T:
		case GUI_VAR_VEC4_T:
		case GUI_VAR_VEC3_T:
		case GUI_VAR_VEC2_T:
		case GUI_VAR_DOUBLE:
		case GUI_VAR_FLOAT:
		case GUI_VAR_POINTER_TO_FLOAT:
		case GUI_VAR_INT:
		case GUI_VAR_SHORT:
		case GUI_VAR_UNSIGNED_SHORT:
		case GUI_VAR_POINTER_TO_UNSIGNED_SHORT:
		case GUI_VAR_CHAR:
		case GUI_VAR_UNSIGNED_CHAR:
		case GUI_VAR_POINTER_TO_UNSIGNED_CHAR:
		case GUI_VAR_ALLOCD_STRING:
		case GUI_VAR_STRING:
			//var = malloc(sizeof(gui_var_t));
			var = (gui_var_t *)memory_Malloc(sizeof(gui_var_t), "gui_CreateVar");
			//var->name = strdup(name);
			var->name = memory_Strdup(name, "gui_CreateVar");
			var->type = type;
			var->addr = addr;
			var->base = refresh_base;
			var->offset = offset;
			//var->prev_ptr = NULL;
			var->next = NULL;
			var->bm_flags = GUI_VAR_VALUE_HAS_CHANGED;
			var->prev_var_value.str_var = NULL;
			/*if(type == GUI_VAR_STRING)
			{
				var->prev_var_value.str_var = strdup(*(char **)addr);
			}*/
			
			
			if(!gui_vars)
			{
				gui_vars = var;
			}
			else
			{
				last_gui_var->next = var;
			}
			
			last_gui_var = var;
		break;
	}

	return var;
}

void gui_TrackVar(gui_var_t *var, widget_t *widget)
{
	text_field_t *field;
	char *format;
	if(widget)
	{
		widget->var = var;
		widget->bm_flags |= WIDGET_TRACK_VAR;		
	}
}

/* this will cause problems if there's a widget tracking
this var... */
void gui_DeleteVar(gui_var_t *var)
{
	gui_var_t *r = gui_vars;
	
	while(r)
	{
		if(r->next == var)
		{
			r->next = var->next;
			break;
		}		
		r = r->next;
	}
	
	memory_Free(var->name);
		
	if(var->type == GUI_VAR_STRING)
		if(var->prev_var_value.str_var)
			memory_Free(var->prev_var_value.str_var);
			
	memory_Free(var);
}


#define WIDGET_STACK_SIZE 128

#define push_widget(w) if(widget_stack_top+1>=WIDGET_STACK_SIZE)		\\
					   {												\\
							printf("cannot push widget!\n");			\\
					   }												\\
					   else												\\
					   {												\\
					   		x += w->x;									\\
							y += w->y;									\\
							widget_stack_top++;							\\
							widget_stack[widget_stack_top] = w;			\\
							w = w->nestled;								\\
					   }


void gui_UpdateVars()
{
	gui_var_t *v = gui_vars;
	
	
	while(v)
	{
		if(!v->addr)
		{
			v = v->next;
			continue;	
		}	
		switch(v->type)
		{
			case GUI_VAR_INT:
				if(v->prev_var_value.int_var != *((int *)v->addr))
				{
					v->prev_var_value.int_var = *((int *)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			break;
			
			case GUI_VAR_POINTER_TO_FLOAT:
				if(!(*(float **)v->addr))
					break;
				
				if(v->prev_var_value.float_var != **((float **)v->addr))
				{
					v->prev_var_value.float_var = **((float **)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			
			break;
				
			case GUI_VAR_FLOAT:
				if(v->prev_var_value.float_var != *((float *)v->addr))
				{
					v->prev_var_value.float_var = *((float *)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			break;
			
			case GUI_VAR_ALLOCD_STRING:
			case GUI_VAR_STRING:
				
				if(!(*(char **)v->addr))
					break;
				
				if(!v->prev_var_value.str_var || strcmp(v->prev_var_value.str_var, *((char **)v->addr)))
				{
					if(v->prev_var_value.str_var)
					{
						//memory_Free(v->prev_var_value.str_var);
					}
					
					v->prev_var_value.str_var = memory_Strdup(*((char **)v->addr), "gui_UpdateVars");
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			break;
			
			case GUI_VAR_UNSIGNED_CHAR:
				if(v->prev_var_value.unsigned_char_var != *((unsigned char *)v->addr))
				{
					v->prev_var_value.unsigned_char_var = *((unsigned char *)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			break;
			
			case GUI_VAR_POINTER_TO_UNSIGNED_CHAR:				
				if(!(*(unsigned char **)v->addr))
					break;
					
				if(v->prev_var_value.unsigned_char_var != **((unsigned char **)v->addr))
				{
					v->prev_var_value.unsigned_char_var = **((unsigned char **)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			break;
			
			case GUI_VAR_UNSIGNED_SHORT:
				if(v->prev_var_value.unsigned_short_var != *((unsigned short *)v->addr))
				{
					v->prev_var_value.unsigned_short_var = *((unsigned short *)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			break;
			
			case GUI_VAR_SHORT:
				if(v->prev_var_value.short_var != *((short *)v->addr))
				{
					v->prev_var_value.short_var = *((short *)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
			break;
			
			case GUI_VAR_POINTER_TO_UNSIGNED_SHORT:
				if(!(*(unsigned short **)v->addr))
					break;
					
				if(v->prev_var_value.unsigned_short_var != **((unsigned short **)v->addr))
				{
					v->prev_var_value.unsigned_short_var = **((unsigned short **)v->addr);
					v->bm_flags |= GUI_VAR_VALUE_HAS_CHANGED;
				}
					
			break;
		}
		
		v = v->next;
	}
}


void gui_UpdateWidgetRelativeMouse(widget_t *widget, short parent_x, short parent_y)
{
	float screen_mouse_x = (r_window_width * 0.5) * normalized_mouse_x;
	float screen_mouse_y = (r_window_height * 0.5) * normalized_mouse_y;

	float relative_screen_mouse_x;
	float relative_screen_mouse_y;
	
	float relative_mouse_x;
	float relative_mouse_y;
	
	relative_screen_mouse_x = screen_mouse_x - ((float)widget->x + (float)parent_x);
	relative_screen_mouse_y = screen_mouse_y - ((float)widget->y + (float)parent_y);
		
	widget->relative_mouse_x = relative_screen_mouse_x / (float)widget->w;
	widget->relative_mouse_y = relative_screen_mouse_y / (float)widget->h;
}

void gui_UpdateWidgetMouseEvents(widget_t *widget)
{
	
	widget_t *top;
	widget_t *r;
	widget_t *w = widget;
	
	
	w->bm_flags &= ~(WIDGET_MOUSE_OVER | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | 
				     WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON | WIDGET_OUT_OF_BOUNDS | 
					 WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK | WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP |
					 WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN);
	
	
	if(w->parent)
	{
		top = w->parent->nestled;
			
		if(!top)
		{
			printf("%s %s\n", w->name, w->parent->name);
		}
	}
	else
	{
		top = widgets;
	}
	
	
	if(w == top)
	{	
		/* this enables manipulating widgets like sliders. This
		only clears the WIDGET_HAS_LEFT_MOUSE_BUTTON flag if the left
		mouse button is not pressed, given that the top widget will
		conserve the flag even if the mouse is not over it... */
		if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
		{
			w->bm_flags &= ~WIDGET_HAS_LEFT_MOUSE_BUTTON;
			
			w->edge_flags &= ~(WIDGET_LEFT_EDGE_GRABBED | 
			 				   WIDGET_RIGHT_EDGE_GRABBED | 
							   WIDGET_TOP_EDGE_GRABBED | 
							   WIDGET_BOTTOM_EDGE_GRABBED);
		}
		
		if(!(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED))
		{
			w->bm_flags &= ~WIDGET_HAS_MIDDLE_MOUSE_BUTTON;
		}
			
	}
	else
	{
		w->bm_flags &= ~(WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_HAS_RIGHT_MOUSE_BUTTON | WIDGET_HAS_MIDDLE_MOUSE_BUTTON);
	}
		
	w->edge_flags &= ~(WIDGET_MOUSE_OVER_LEFT_EDGE | 
					   WIDGET_MOUSE_OVER_RIGHT_EDGE | 
					   WIDGET_MOUSE_OVER_TOP_EDGE | 
					   WIDGET_MOUSE_OVER_BOTTOM_EDGE);
		
	
	if(w->edge_flags & WIDGET_LEFT_EDGE_GRABBED)
	{
		w->edge_flags |= WIDGET_MOUSE_OVER_LEFT_EDGE;
	}
	else if(w->edge_flags & WIDGET_RIGHT_EDGE_GRABBED)
	{
		w->edge_flags |= WIDGET_MOUSE_OVER_RIGHT_EDGE;
	}
					
	if(w->edge_flags & WIDGET_BOTTOM_EDGE_GRABBED)
	{
		w->edge_flags |= WIDGET_MOUSE_OVER_BOTTOM_EDGE;
	}		
	else if(w->edge_flags & WIDGET_TOP_EDGE_GRABBED)
	{
		w->edge_flags |= WIDGET_MOUSE_OVER_TOP_EDGE;
	}
		
			
	if(w->relative_mouse_x > -1.0 && w->relative_mouse_x <= 1.0 && w->relative_mouse_y > -1.0 && w->relative_mouse_y <= 1.0)
	{
		if(!(top->bm_flags & WIDGET_MOUSE_OVER))
		{				
			//if(!(w->bm_flags & WIDGET_DONT_RECEIVE_MOUSE))
			{
				if(w->bm_flags & WIDGET_IGNORE_EDGE_CLIPPING)
				{
					_do_flag_business:
					
					r = top->parent;
							
					/* propagate the flag upwards... */
					while(r)
					{
						r->bm_flags |= WIDGET_MOUSE_OVER;
						r = r->parent;
					}
					
					/* it's important to have the widget detect when the mouse is over
					it even when it's set to ignore it. Not doing so essentially allows
					one to click 'through' it... */	
					w->bm_flags |= WIDGET_MOUSE_OVER;
					
					if(!(w->bm_flags & WIDGET_DONT_RECEIVE_MOUSE))
					{
						if(w->w - abs(w->w * w->relative_mouse_x) <= WIDGET_BORDER_WIDTH)
						{
							if(w->relative_mouse_x < 0.0)
							{
								if(w->edge_flags & WIDGET_LEFT_EDGE_ENABLED)
								{
									w->edge_flags |= WIDGET_MOUSE_OVER_LEFT_EDGE;
								}
									
							}
							else
							{
								if(w->edge_flags & WIDGET_RIGHT_EDGE_ENABLED)
								{
									w->edge_flags |= WIDGET_MOUSE_OVER_RIGHT_EDGE;
								}
							}
						}
							
						
						if(w->h - abs(w->h * w->relative_mouse_y) <= WIDGET_BORDER_WIDTH)
						{
							if(w->relative_mouse_y < 0.0)
							{
								if(w->edge_flags & WIDGET_BOTTOM_EDGE_ENABLED)
								{
									w->edge_flags |= WIDGET_MOUSE_OVER_BOTTOM_EDGE;
								}
							}
							else
							{
								if(w->edge_flags & WIDGET_TOP_EDGE_ENABLED)
								{
									w->edge_flags |= WIDGET_MOUSE_OVER_TOP_EDGE;
								}
							}
						}
					}
				}
				else
				{
					if(w->parent)
					{
						if(w->parent->bm_flags & WIDGET_MOUSE_OVER)
						{
							goto _do_flag_business;
						}
					}
					else
					{
						goto _do_flag_business;
					}
				}
				
				
				
				
				if(w->bm_flags & WIDGET_MOUSE_OVER)
				{
					bm_mouse |= MOUSE_OVER_WIDGET;
					
					if(!(w->bm_flags & WIDGET_DONT_RECEIVE_MOUSE))
					{
						if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
						{
							w->bm_flags |= WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON;
							
							r = w->parent;
								
							while(r)
							{
								r->bm_flags |= WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON;
								r = r->parent;
							}
							
							gui_SetAsTop(w);
							
							switch(w->type)
							{
								case WIDGET_BASE:
								case WIDGET_SURFACE:
									
									switch(w->edge_flags & (WIDGET_MOUSE_OVER_LEFT_EDGE | WIDGET_MOUSE_OVER_RIGHT_EDGE))
									{
										case WIDGET_MOUSE_OVER_LEFT_EDGE:
											w->edge_flags |= WIDGET_LEFT_EDGE_GRABBED;
										break;	
											
										case WIDGET_MOUSE_OVER_RIGHT_EDGE:
											w->edge_flags |= WIDGET_RIGHT_EDGE_GRABBED;
										break;
									}
										
									switch(w->edge_flags & (WIDGET_MOUSE_OVER_TOP_EDGE | WIDGET_MOUSE_OVER_BOTTOM_EDGE))
									{
										case WIDGET_MOUSE_OVER_TOP_EDGE:
											w->edge_flags |= WIDGET_TOP_EDGE_GRABBED;
										break;	
											
										case WIDGET_MOUSE_OVER_BOTTOM_EDGE:
											w->edge_flags |= WIDGET_BOTTOM_EDGE_GRABBED;
										break;
									}
										
								break;
							}
								
						}
							
						if(bm_mouse & MOUSE_LEFT_BUTTON_DOUBLE_CLICKED)
						{
							w->bm_flags |= WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK;
							
							r = w->parent;
								
							while(r)
							{
								r->bm_flags |= WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK;
								r = r->parent;
							}
						}
							
						if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
						{
							w->bm_flags |= WIDGET_HAS_RIGHT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON;
							
							r = w->parent;
								
							while(r)
							{
								r->bm_flags |= WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON;
								r = r->parent;
							}
								
							gui_SetAsTop(w);
						}
							
							
						if(bm_mouse & MOUSE_WHEEL_UP)
						{
							w->bm_flags |= WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP;
								
							r = w->parent;
								
							while(r)
							{
								r->bm_flags |= WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP;
								r = r->parent;
							}
						}
						else if(bm_mouse & MOUSE_WHEEL_DOWN)
						{
							w->bm_flags |= WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN;
								
							r = w->parent;
								
							while(r)
							{
								r->bm_flags |= WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN;
								r = r->parent;
							}
						}
							
						if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED)
						{
							w->bm_flags |= WIDGET_HAS_MIDDLE_MOUSE_BUTTON;
								
							r = w->parent;
								
							while(r)
							{
								r->bm_flags |= WIDGET_HAS_MIDDLE_MOUSE_BUTTON;
								r = r->parent;
							}
						}
					}
					
					
						
				}
			}	
			
			
		}
	}
}

void gui_UpdateWidgetEdges(widget_t *widget, short parent_x, short parent_y)
{
	
	widget_t *w;
	short right_edge_x;
	short left_edge_x;
	short top_edge_y;
	short bottom_edge_y;
	short dx;
	short dy;
	short x = parent_x;
	short y = parent_y;
	
	if(w->edge_flags & WIDGET_RIGHT_EDGE_GRABBED)
	{
		right_edge_x = x + w->x + w->w;
		dx = (short)(r_window_width * normalized_mouse_x * 0.5) - right_edge_x;
		
		dx >>= 1;
			
		if(dx + w->w < WIDGET_MIN_SIZE)
		{
			dx = -w->w + WIDGET_MIN_SIZE;
		}
			
		w->x += dx;
		w->w += dx;
	}
		
	if(w->edge_flags & WIDGET_LEFT_EDGE_GRABBED)
	{
		left_edge_x = x + w->x - w->w;
		dx = (short)(r_window_width * normalized_mouse_x * 0.5) - left_edge_x;
		
		dx >>= 1;
			
		if(w->w - dx < WIDGET_MIN_SIZE)
		{
			dx = w->w - WIDGET_MIN_SIZE;
		}
			
		w->x += dx;
		w->w -= dx;
	}
		
	if(w->edge_flags & WIDGET_TOP_EDGE_GRABBED)
	{
		top_edge_y = y + w->y + w->h;
		dy = (short)(r_window_height * normalized_mouse_y * 0.5) - top_edge_y;

		dy >>= 1;
			
		if(dy + w->h < WIDGET_MIN_SIZE)
		{
			dy = -w->h + WIDGET_MIN_SIZE;
		}
			
			
		w->y += dy;
		w->h += dy;
	}
		
	if(w->edge_flags & WIDGET_BOTTOM_EDGE_GRABBED)
	{
		bottom_edge_y = y + w->y - w->h;
		dy = (short)(r_window_height * normalized_mouse_y * 0.5) - bottom_edge_y;

		dy >>= 1;
			
		if(w->h - dy < WIDGET_MIN_SIZE)
		{
			dy = w->h - WIDGET_MIN_SIZE;
		}

			
		w->y += dy;
		w->h -= dy;
	}
}


void gui_ProcessGUI()
{
	widget_t *w;
	widget_t *new_top;
	widget_t *top;
	widget_t *r;
	button_t *button;
	dropdown_t *dropdown;
	checkbox_t *checkbox;
	option_list_t *option_list;
	option_t *option;
	widget_bar_t *bar;
	text_field_t *field;
	
	linked_edge_t *linked;
	short smallest_linked_edge_dist;
	
	int widget_stack_top = -1;
	widget_t *widget_stack[WIDGET_STACK_SIZE];
	
	//input_GetInput(0.0);
	
	float screen_mouse_x = (r_window_width * 0.5) * normalized_mouse_x;
	float screen_mouse_y = (r_window_height * 0.5) * normalized_mouse_y;

	float relative_screen_mouse_x;
	float relative_screen_mouse_y;
	
	float relative_mouse_x;
	float relative_mouse_y;
	
	int b_do_rest = 0;
	int x = 0;
	int y = 0;
	short right_edge_x;
	short bottom_edge_y;
	short left_edge_x;
	short top_edge_y;
	
	short linked_edge_x;
	short linked_edge_y;
	short this_edge_x;
	short this_edge_y;
	short hpos_signal;
	short hdim_signal;
	short vpos_signal;
	short vdim_signal;
	
	short dx;
	short dy;
	int call_callback;
	int mouse_over_top;
	
	short top_y;
	short y_increment;
	short bottom_y;
	
	int b_keep_text_input = 0;
	
	new_top = NULL;
	
	//w = top_widget;
	w = widgets;

	gui_UpdateVars();	

	top = widgets;
	
	//printf("%d\n", (short)(r_window_width * normalized_mouse_x * 0.5));
	
	_do_rest:
	
	//bm_mouse &= ~MOUSE_OVER_WIDGET	
		
	while(w)
	{		
		gui_UpdateWidgetRelativeMouse(w, x, y);
		
		/* ignore invisible widgets... */
		if(w->bm_flags & WIDGET_INVISIBLE)
			goto _advance_widget;
			
		gui_UpdateWidgetMouseEvents(w);
		gui_UpdateWidgetEdges(w, x, y);
		
		if(w->linked_edges)
		{
			linked = w->linked_edges;
			/* first go over the linked edges this widget has... */
			while(linked)
			{
				gui_GetAbsolutePosition(linked->widget, &linked_edge_x, &linked_edge_y);
				
				hpos_signal = 0;
				hdim_signal = 0;
				vpos_signal = 0;
				vdim_signal = 0;
				
				
				switch(linked->linked_widget_edge)
				{
					case WIDGET_LEFT_EDGE:
						linked_edge_x -= linked->widget->w;
					break;
					
					case WIDGET_RIGHT_EDGE:
						linked_edge_x += linked->widget->w;
					break;
					
					case WIDGET_TOP_EDGE:
						linked_edge_y += linked->widget->h;
					break;
					
					case WIDGET_BOTTOM_EDGE:
						linked_edge_y -= linked->widget->h;
					break;
				}
				
				
				
				switch(linked->this_widget_edge)
				{
					case WIDGET_LEFT_EDGE:
						this_edge_x = x + w->x - w->w;
						dx = (this_edge_x - linked_edge_x + linked->offset) >> 1;
						hpos_signal = -1;
						hdim_signal = 1;
					break;
					
					case WIDGET_RIGHT_EDGE:
						this_edge_x = x + w->x + w->w;
						dx = (linked_edge_x - this_edge_x + linked->offset) >> 1;
						hpos_signal = 1;
						hdim_signal = 1;								
					break;
					
					case WIDGET_TOP_EDGE:
						this_edge_y = y + w->y + w->h;
						dy = (linked_edge_y - this_edge_y) >> 1;
						vpos_signal = 1;
						vdim_signal = 1;
					break;
					
					case WIDGET_BOTTOM_EDGE:
						this_edge_y = y + w->y - w->h;
						dy = (this_edge_y - linked_edge_y) >> 1;
						vpos_signal = -1;
						vdim_signal = 1;
					break;
				}
				
				/* ... and make sure we don't turn a widget "inside-out"
				by clamping the edge movement if a widget's size becomes
				smaller than the minimal allowed size... */
				if(linked->this_widget_edge == linked->linked_widget_edge)
				{
					
					if(linked->widget->w - dx < WIDGET_MIN_SIZE)
					{
						dx -= linked->widget->w - WIDGET_MIN_SIZE;						
						w->x += dx * hpos_signal;
						w->w += dx * hdim_signal;
					}
				}
				else
				{
					if(linked->widget->w + dx < WIDGET_MIN_SIZE)
					{
						dx += linked->widget->w - WIDGET_MIN_SIZE;
						w->x += dx * hpos_signal;
						w->w += dx * hdim_signal;
					}
				}
				
				if(linked->this_widget_edge == linked->linked_widget_edge)
				{
					if(linked->widget->h - dy < WIDGET_MIN_SIZE)
					{
						dy -= linked->widget->h - WIDGET_MIN_SIZE;
						w->y += dy * vpos_signal;
						w->h += dy * vdim_signal;
					}
				}
				else
				{
					if(linked->widget->h + dy < WIDGET_MIN_SIZE)
					{
						dy += linked->widget->h - WIDGET_MIN_SIZE;
						w->y += dy * vpos_signal;
						w->h += dy * vdim_signal;
					}
				}
				
				
				linked = linked->next;			
			}
			
			
			linked = w->linked_edges;
			/* now go over all the linked edgess again... */
			while(linked)
			{
				gui_GetAbsolutePosition(linked->widget, &linked_edge_x, &linked_edge_y);
				
				switch(linked->this_widget_edge)
				{
					case WIDGET_LEFT_EDGE:
						this_edge_x = x + w->x - w->w;
					break;
					
					case WIDGET_RIGHT_EDGE:
						this_edge_x = x + w->x + w->w;						
					break;
					
					case WIDGET_TOP_EDGE:
						this_edge_y = y + w->y + w->h;
					break;
					
					case WIDGET_BOTTOM_EDGE:
						this_edge_y = y + w->y - w->h;
					break;
				}
				
				hpos_signal = 0;
				hdim_signal = 0;
				vpos_signal = 0;
				vdim_signal = 0;
				
				switch(linked->linked_widget_edge)
				{
					case WIDGET_LEFT_EDGE:
						linked_edge_x -= linked->widget->w;
						dx = (this_edge_x - linked_edge_x + linked->offset) >> 1;	
						hpos_signal = 1;
						hdim_signal = -1;
					break;
					
					case WIDGET_RIGHT_EDGE:
						linked_edge_x += linked->widget->w;
						dx = (linked_edge_x - this_edge_x + linked->offset) >> 1;
						hpos_signal = -1;
						hdim_signal = -1;						
					break;
					
					case WIDGET_TOP_EDGE:
						linked_edge_y += linked->widget->h;
						dy = (this_edge_y - linked_edge_y) >> 1;
						vpos_signal = 1;
						vdim_signal = 1;
					break;
					
					case WIDGET_BOTTOM_EDGE:
						linked_edge_y -= linked->widget->h;
						dy = (linked_edge_y - this_edge_y) >> 1;
						vpos_signal = -1;
						vdim_signal = 1;
					break;
				}
				
				/* ... and properly update their positions
				so they all match correctly... */
				linked->widget->x += dx * hpos_signal;
				linked->widget->w += dx * hdim_signal;
				linked->widget->y += dy * vpos_signal;
				linked->widget->h += dy * vdim_signal;
				
				
				linked = linked->next;			
			}
			
			
		}
	
		
		call_callback = 0;
		
		
		if(w->bm_flags & WIDGET_RENDER_TEXT || w->bm_flags & WIDGET_RENDER_NAME)
		{
			gui_RenderText(w);
		}
		
	
		
		switch(w->type)
		{
			case WIDGET_BASE:
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					widget_stack_top++;		
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					//top = &(*top)->nestled;
					continue;
				}
			break;
			
			case WIDGET_BUTTON:
				gui_UpdateButton(w);
			break;
			
			case WIDGET_SLIDER:
				gui_UpdateSlider(w);
			break;
			
			case WIDGET_CHECKBOX:
				gui_UpdateCheckbox(w);				
			break;
			
			case WIDGET_DROPDOWN:
				
				gui_UpdateDropdown(w);
				
				dropdown = (dropdown_t *)w;
					
				if(w->nestled)
				{
					//if(!(w->nestled->bm_flags & WIDGET_INVISIBLE))
					if(dropdown->bm_dropdown_flags & DROPDOWN_DROPPED)
					{
						x += w->x;
						y += w->y;
						widget_stack_top++;		
						widget_stack[widget_stack_top] = w;
						w = w->nestled;
						//top = &(*top)->nestled;
						continue;
					}		
				}			
			break;
			
			case WIDGET_OPTION_LIST:
				
				gui_UpdateOptionList(w);
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					widget_stack_top++;		
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					continue;
				}
			break;
			
			case WIDGET_OPTION:
				
				
				option_list = (option_list_t *)w->parent;
				option = (option_t *)w;
						
				if(!option_list)
					break;
					
				
				
				if((option->widget.y + option->widget.h < -option_list->widget.h + OPTION_HEIGHT) || 
				   (option->widget.y - option->widget.h > option_list->widget.h - OPTION_HEIGHT))
				{
					/*printf("%s [%d %d]  [%d %d]\n", option->widget.name, option->widget.y + option->widget.h, -option_list->widget.h,
												 						 option->widget.y - option->widget.h, option_list->widget.h);*/
					option->widget.bm_flags |= WIDGET_OUT_OF_BOUNDS;
					//printf("%s\n", option->widget.name);	
				}
				else
				{
					gui_UpdateOption(w);
				
					if(w->nestled)
					{
						/* only recurse down if this option is the active one... */
						if(option == (option_t *)option_list->active_option)
						{
							w->nestled->bm_flags &= ~WIDGET_INVISIBLE;
							
							x += w->x;
							y += w->y;
							widget_stack_top++;		
							widget_stack[widget_stack_top] = w;
							w = w->nestled;
							//top = &(*top)->nestled;
							continue;
						}
						else
						{
							w->nestled->bm_flags |= WIDGET_INVISIBLE;
						}
					}
				}
				
					
				
				
			break;
			
			case WIDGET_BAR:
				
				gui_UpdateWidgetBar(w);
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					widget_stack_top++;		
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					//top = &(*top)->nestled;
					continue;
				}
			break;
			
			case WIDGET_TEXT_FIELD:
				gui_UpdateTextField(w);
			break;
			
			case WIDGET_SURFACE:
				gui_UpdateSurface(w);
			break;
			
			case WIDGET_ITEM_LIST:
				gui_UpdateItemList((item_list_t *)w);
				
				if(w->nestled)
				{
					x += w->x;
					y += w->y;
					widget_stack_top++;
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					continue;
				}
			break;
			
			case WIDGET_ITEM_TREE:
				//gui_UpdateItemTree(w);
				
				
			break;
		}
		
		if(w->process_callback)
		{
			w->process_callback(w);
		}
			
		_advance_widget:
			
		w->bm_flags &= ~WIDGET_JUST_CREATED;
		
		
		switch(w->type)
		{
			/* we'll get here after recursing through all the
			widgets contained within this bar, and so we have
			all the information needed to properly decide things... */
			case WIDGET_BAR:
				gui_PostUpdateWidgetBar(w);
			break;
			
			case WIDGET_DROPDOWN:
				gui_PostUpdateDropdown(w);				
			break;
			
			case WIDGET_OPTION_LIST:
				gui_PostUpdateOptionList(w);
			break;
			
			case WIDGET_TEXT_FIELD:
				gui_PostUpdateTextField(w);
				
				field = (text_field_t *)w;
				
				if(field->bm_text_field_flags & TEXT_FIELD_RECEIVING_TEXT)
				{
					b_keep_text_input = 1;
				}
			break;
			
			case WIDGET_SURFACE:
				gui_PostUpdateSurface(w);
			break;
			
			case WIDGET_ITEM_LIST:
				gui_PostUpdateItemList((item_list_t *)w);
			break;
		}
		
		w = w->next;
		
		/* this will keep popping from this stack until
		something not null appears to be processed or 
		until the stack is empty. The latter means
		the work is done and we can go home... */
		if(!w)
		{
			if(widget_stack_top >= 0)
			{
				w = widget_stack[widget_stack_top];
				widget_stack_top--;
				
				x -= w->x;
				y -= w->y;
				//top = &(*top)->parent;
				
				goto _advance_widget;
			}
		}
		
	}
	
	if(!b_keep_text_input)
	{
		input_EnableTextInput(0);
	}
		
}

void gui_UpdateGUIProjectionMatrix()
{
	float right = r_window_width / 2;
	float left = -right;
	float top = r_window_height / 2;
	float bottom = -top;
	CreateOrthographicMatrix(&gui_projection_matrix, left, right, top, bottom, -10.0, 10.0, NULL);
}

gui_var_t gui_MakeStringPtrVar(char **value)
{
	gui_var_t var;
	var.prev_var_value.str_ptr_var = value;
	return var;
}

gui_var_t gui_MakeUnsignedCharVar(unsigned char value)
{
	gui_var_t var;
	var.prev_var_value.unsigned_char_var = value;
	return var;
}

gui_var_t gui_MakeUnsignedShortVar(unsigned short value)
{
	gui_var_t var;
	var.prev_var_value.unsigned_short_var = value;
	return var;
}

gui_var_t gui_MakeIntVar(int value)
{
	gui_var_t var;
	var.prev_var_value.int_var = value;
	return var;
}

gui_var_t gui_MakeFloatVar(float value)
{
	gui_var_t var;
	var.prev_var_value.float_var = value;
	return var;
}

gui_var_t gui_MakeDoubleVar(double value)
{
	
}

gui_var_t gui_MakeVec2Var(vec2_t value)
{
	
}

gui_var_t gui_MakeVec3Var(vec3_t value)
{
	
}



void gui_UpdateWidget(widget_t *widget)
{

}

void gui_PostUpdateWidget(widget_t *widget)
{
	
}

void gui_LinkLeftEdge(widget_t *widget, widget_t *left_edge)
{
	linked_edge_t *new_linked;
	linked_edge_t *linked;
	if(widget && left_edge)
	{
		
		//new_linked = malloc(sizeof(linked_edge_t));
		new_linked = (linked_edge_t *)memory_Malloc(sizeof(linked_edge_t), "gui_LinkLeftEdge");
		new_linked->next = NULL;
		new_linked->widget = widget;
		new_linked->this_widget_edge = WIDGET_RIGHT_EDGE;
		new_linked->linked_widget_edge = WIDGET_LEFT_EDGE;
		
		if(!left_edge->linked_edges)
		{
			left_edge->linked_edges = new_linked;
		}
		else
		{
			linked = left_edge->linked_edges;
			while(linked->next)
			{
				linked = linked->next;
			}
			
			linked->next = new_linked;
		}
		
		
		widget->edge_flags &= ~WIDGET_LEFT_EDGE_ENABLED;
	}
}

void gui_LinkBottomEdge(widget_t *widget, widget_t *bottom_edge)
{
	linked_edge_t *new_linked;
	linked_edge_t *linked;
	if(widget && bottom_edge)
	{
		
		new_linked = (linked_edge_t *)memory_Malloc(sizeof(linked_edge_t), "gui_LinkBottomEdge");
		new_linked->next = NULL;
		new_linked->widget = widget;
		new_linked->this_widget_edge = WIDGET_TOP_EDGE;
		new_linked->linked_widget_edge = WIDGET_BOTTOM_EDGE;
		
		if(!bottom_edge->linked_edges)
		{
			bottom_edge->linked_edges = new_linked;
		}
		else
		{
			linked = bottom_edge->linked_edges;
			while(linked->next)
			{
				linked = linked->next;
			}
			
			linked->next = new_linked;
		}
		
		
		widget->edge_flags &= ~WIDGET_BOTTOM_EDGE_ENABLED;
	}
}

linked_edge_t *gui_LinkEdges(widget_t *to_link, widget_t *link_to, int to_link_edge, int link_to_edge)
{
	linked_edge_t *new_linked = NULL;
	linked_edge_t *linked = NULL;
	
	if(to_link && link_to)
	{
		if(to_link_edge >= WIDGET_LEFT_EDGE && to_link_edge <= WIDGET_BOTTOM_EDGE &&
		   link_to_edge >= WIDGET_LEFT_EDGE && link_to_edge <= WIDGET_BOTTOM_EDGE)
		{
			new_linked = (linked_edge_t *)memory_Malloc(sizeof(linked_edge_t), "gui_LinkEdges");
			new_linked->next = NULL;
			new_linked->widget = to_link;
			new_linked->this_widget_edge = link_to_edge;
			new_linked->linked_widget_edge = to_link_edge;
			new_linked->offset = 0;
			
			if(!link_to->linked_edges)
			{
				link_to->linked_edges = new_linked;
			}
			else
			{
				linked = link_to->linked_edges;
				
				while(linked->next)
				{
					linked = linked->next;
				}
				
				linked->next = new_linked;
			}			
		}
	}
	
	return new_linked;
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
}
}
#endif





