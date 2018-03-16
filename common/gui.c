#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL2\SDL_ttf.h"
#include "GL/glew.h"

#include "matrix.h"
#include "gui.h"
#include "font.h"
#include "r_main.h"
#include "input.h"


//#include "gui_dropdown.h"


/* from r_main.h */
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;

/* from input.c */
extern float normalized_mouse_x;
extern float normalized_mouse_y;
extern int bm_mouse;
 
widget_t *widgets;
widget_t *last_widget;
widget_t *top_widget;
mat4_t gui_projection_matrix;

gui_var_t *gui_vars = NULL;
gui_var_t *last_gui_var = NULL;

char formated_str[8192];
extern font_t *gui_font;

int gui_widget_unique_index = 0;


int gui_Init()
{
	widgets = NULL;
	gui_UpdateGUIProjectionMatrix();
	return 1;
}

void gui_Finish()
{
	/*while(widgets)
	{
		gui_DestroyWidget(widgets);
	}*/
}

widget_t *gui_CreateWidget(char *name, short x, short y, short w, short h, int type)
{
	widget_t *widget;
	int name_len;
	int size;
	
	
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
		
		default:
			return NULL;
	}
	
	
	//widget = malloc(sizeof(widget_t));
	widget = malloc(size);
	
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
	widget->type = type;
	widget->parent = NULL;
	
	widget->left_edge_of = NULL;
	widget->bottom_edge_of = NULL;
	//widget->first_parent = NULL;
	widget->widget_callback = NULL;
	
	//widget->name = strdup(name);
	
	widget->name = malloc(WIDGET_NAME_MAX_LEN);
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
	widget->process_callback = NULL;
	
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
	
	wsurface_t *surface;
	
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
				widgets->prev = NULL;
		}
		else
		{
			parent = widget->parent;
			
			if(widget == parent->nestled)
			{
				parent->nestled = widget->next;
				
				if(parent->nestled)
					parent->nestled->prev = NULL;
			}
			else
			{
				widget->prev->next = widget->next;	
			}
			
			if(widget->next)
			{
				widget->next->prev = widget->prev;
			}
		}
	}
	
	free(widget->name);
	
	if(widget->rendered_name)
		SDL_FreeSurface(widget->rendered_name);
		
	
	switch(widget->type)
	{
		case WIDGET_SURFACE:
			surface = (wsurface_t *)widget;
			glDeleteTextures(1, &surface->color_texture);
			glDeleteTextures(1, &surface->depth_texture);
			glDeleteFramebuffers(1, &surface->framebuffer_id);
		break;
	}
		
	
	free(widget);	
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
			
			/* no business setting a bar's widgets as top, 
			as it messes up their order whenever the bar
			readjusts everything... */
			//if(widget->parent->type == WIDGET_BAR)
			//	return;
			
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
		and it will be either in the middle or the last... */
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
				
				option->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, option->widget.w * 2.0);
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
						
				if(field->rendered_text)
					SDL_FreeSurface(field->rendered_text);
						
				sprintf(formated_str, field->text);
				field->rendered_text = TTF_RenderUTF8_Blended_Wrapped(gui_font->font, formated_str, foreground, field->widget.w * 2.0);		
			}
			
		break;
		
		case WIDGET_BUTTON:
			
			button = (button_t *)widget;
			
			if(button->rendered_text)
				SDL_FreeSurface(button->rendered_text);
			
			
			sprintf(formated_str, button->widget.name);
			
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
			var = malloc(sizeof(gui_var_t));
			var->name = strdup(name);
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
			break;
		
		r = r->next;
	}
	
	if(r)
	{
		r->next = var->next;
		
		free(var->name);
		
		if(var->type == GUI_VAR_STRING)
			if(var->prev_var_value.str_var)
				free(var->prev_var_value.str_var);
				
		
		free(var);
	}
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
						free(v->prev_var_value.str_var);
					}
					
					v->prev_var_value.str_var = strdup(*((char **)v->addr));
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
	
	int widget_stack_top = -1;
	widget_t *widget_stack[WIDGET_STACK_SIZE];
	
	float screen_mouse_x = (r_window_width * 0.5) * normalized_mouse_x;
	float screen_mouse_y = (r_window_height * 0.5) * normalized_mouse_y;

	float relative_screen_mouse_x;
	float relative_screen_mouse_y;
	
	float relative_mouse_x;
	float relative_mouse_y;
	
	int b_do_rest = 0;
	int x = 0;
	int y = 0;
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
	
	_do_rest:
	
	//bm_mouse &= ~MOUSE_OVER_WIDGET;
		
	while(w)
	{
		
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
		
		
		
		relative_screen_mouse_x = screen_mouse_x - ((float)w->x + (float)x);
		relative_screen_mouse_y = screen_mouse_y - ((float)w->y + (float)y);
		
		w->relative_mouse_x = relative_screen_mouse_x / (float)w->w;
		w->relative_mouse_y = relative_screen_mouse_y / (float)w->h;
		
		w->bm_flags &= ~(WIDGET_MOUSE_OVER |
						 WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | 
						 WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON | 
						 WIDGET_OUT_OF_BOUNDS | 
						 WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK |
						 WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP |
						 WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN);
		
		/* ignore invisible widgets... */
		if(w->bm_flags & WIDGET_INVISIBLE)
			goto _advance_widget;
			
		//w->bm_flags &= ~WIDGET_OUT_OF_BOUNDS;	
		
		
				
		if(w == top)
		{	
			/* this enables manipulating widgets like sliders. This
			only clears the WIDGET_HAS_LEFT_MOUSE_BUTTON flag if the left
			mouse button is not pressed, given that the top widget will
			conserve the flag even if the mouse is not over it... */
			if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
			{
				w->bm_flags &= ~WIDGET_HAS_LEFT_MOUSE_BUTTON;
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
					
					w->bm_flags |= WIDGET_MOUSE_OVER;
					
					if(w->w - abs(w->w * w->relative_mouse_x) <= WIDGET_BORDER_WIDTH)
					{
						if(w->relative_mouse_x < 0.0)
						{
							w->edge_flags |= WIDGET_MOUSE_OVER_LEFT_EDGE;
						}
						else
						{
							w->edge_flags |= WIDGET_MOUSE_OVER_RIGHT_EDGE;
						}
					}
					
					
					if(w->h - abs(w->h * w->relative_mouse_y) <= WIDGET_BORDER_WIDTH)
					{
						if(w->relative_mouse_y < 0.0)
						{
							w->edge_flags |= WIDGET_MOUSE_OVER_BOTTOM_EDGE;
						}
						else
						{
							w->edge_flags |= WIDGET_MOUSE_OVER_TOP_EDGE;
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
					
					if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
					{
						w->bm_flags |= WIDGET_HAS_LEFT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON;
						
						r = w->parent;
						
						while(r)
						{
							r->bm_flags |= WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON;
							r = r->parent;
						}
						
						//if(w->type != WIDGET_OPTION)
							gui_SetAsTop(w);
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
		
		//_skip_mouse_over:
		
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
					//printf("%s\n", w->name);
					//printf("%s\n", w->nestled->name);
					x += w->x;
					y += w->y;
					widget_stack_top++;		
					widget_stack[widget_stack_top] = w;
					w = w->nestled;
					//top = &(*top)->nestled;
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









