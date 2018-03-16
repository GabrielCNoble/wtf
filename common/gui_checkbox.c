#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "gui_checkbox.h"

extern int gui_widget_unique_index;

checkbox_t *gui_AddCheckbox(widget_t *widget, short x, short y, short w, short h, short bm_flags, void (*checkbox_callback)(widget_t *widget))
{
	checkbox_t *checkbox = NULL;
	widget_t *wdgt;
	
	if(widget)
	{
		/*checkbox = malloc(sizeof(checkbox_t));
	 
		if(w < CHECKBOX_MIN_SIZE) w = CHECKBOX_MIN_SIZE;
		if(h < CHECKBOX_MIN_SIZE) h = CHECKBOX_MIN_SIZE;
		
		checkbox->widget.last_nestled = NULL;
		checkbox->widget.nestled = NULL;
		checkbox->widget.next = NULL;
		checkbox->widget.prev = NULL;
		checkbox->widget.x = x;
		checkbox->widget.y = y;
		checkbox->widget.w = w / 2;
		checkbox->widget.h = h / 2;
		checkbox->widget.bm_flags = WIDGET_RENDER_TEXT;
		checkbox->widget.type = WIDGET_CHECKBOX;
		checkbox->widget.var = NULL;
		checkbox->widget.parent = widget;
		checkbox->widget.widget_callback = checkbox_callback;
		checkbox->widget.rendered_name = NULL;	
		checkbox->bm_checkbox_flags = bm_flags;
		checkbox->widget.unique_index = gui_widget_unique_index++;
		checkbox->widget.process_callback = NULL;*/
		
		checkbox = (checkbox_t *)gui_CreateWidget(NULL, x, y, w, h, WIDGET_CHECKBOX);
		checkbox->widget.widget_callback = checkbox_callback;
		checkbox->bm_checkbox_flags = bm_flags;
		checkbox->widget.parent = widget;
		
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)checkbox;
			widget->last_nestled = (widget_t *)checkbox;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)checkbox;
			checkbox->widget.prev = widget->last_nestled;
			widget->last_nestled = (widget_t *)checkbox;
		}
	}
	
	return checkbox;
}

void gui_UpdateCheckbox(widget_t *widget)
{
	checkbox_t *checkbox = (checkbox_t *)widget;
	
	gui_var_t *var;
	int *iptr;
				
	if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
	{
		if(checkbox->bm_checkbox_flags & CHECKBOX_CHECKED)
		{
			checkbox->bm_checkbox_flags &= ~CHECKBOX_CHECKED;
		}
		else
		{
			checkbox->bm_checkbox_flags |= CHECKBOX_CHECKED;
		}
							
		if(widget->widget_callback)
		{
			widget->widget_callback(widget);
		}		
	}
	
	if(widget->bm_flags & WIDGET_TRACK_VAR)
	{
		var = widget->var;
		
		switch(var->type)
		{
			case GUI_VAR_INT:
				
				if(!var->addr)
					break;
					
				iptr = (int *)var->addr;
				
				//printf("%d : %d\n", (*iptr) & checkbox->bm_check_flags, checkbox->bm_checkbox_flags & CHECKBOX_CHECKED);
				
										
				if(var->bm_flags & GUI_VAR_VALUE_HAS_CHANGED)
				{
					//printf("var changed!\n");
					if((*iptr) & checkbox->bm_check_flags)
					{
						checkbox->bm_checkbox_flags |= CHECKBOX_CHECKED;
					}
					else
					{
						checkbox->bm_checkbox_flags &= ~CHECKBOX_CHECKED;
					}
				}
				else if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
				{
					if(checkbox->bm_checkbox_flags & CHECKBOX_CHECKED)
					{
						*iptr |= checkbox->bm_check_flags;
					}
					else
					{
						*iptr &= ~checkbox->bm_check_flags;
					}
				}
				
					
					
				var->bm_flags &= ~GUI_VAR_VALUE_HAS_CHANGED;
					
			
			break;
		}
	}

}







