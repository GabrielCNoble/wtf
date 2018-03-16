#include "gui_slider.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int gui_widget_unique_index;

slider_t *gui_AddSlider(widget_t *widget, char *name, short x, short y, short w, short bm_flags, void (*slider_callback)(widget_t *), gui_var_t *var, gui_var_t max, gui_var_t min)
{
	slider_t *slider = NULL;
	
	/* sliders can't live on their own... */
	if(widget)
	{
		slider = malloc(sizeof(slider_t ));
		 
		slider->widget.x = x;
		slider->widget.y = y;
		slider->widget.w = w * 0.5;
		slider->widget.h = SLIDER_HEIGHT * 0.5;
		slider->widget.name = strdup(name);
		slider->widget.next = NULL;
		slider->widget.prev = NULL;
		slider->widget.nestled = NULL;
		slider->widget.last_nestled = NULL;
		slider->widget.type = WIDGET_SLIDER;
		slider->widget.var = var;
		slider->widget.widget_callback = slider_callback;
		slider->widget.bm_flags = WIDGET_TRACK_VAR;
		slider->widget.rendered_name = NULL;
		slider->widget.unique_index = gui_widget_unique_index++;
		slider->widget.process_callback = NULL;
		
		
		slider->max_value = max;
		slider->min_value = min;
		slider->slider_position = 0.0;
		slider->bm_slider_flags = bm_flags;	
		
		slider->widget.parent = widget;
		
		if(!widget->nestled)
		{
			widget->nestled = (widget_t *)slider;
			widget->last_nestled = (widget_t *)slider;
		}
		else
		{
			widget->last_nestled->next = (widget_t *)slider;
			slider->widget.prev = widget->last_nestled;
			widget->last_nestled = (widget_t *)slider;
		}
		
	}
	
	return slider;
	
}


void gui_UpdateSlider(widget_t *widget)
{
	slider_t *slider = (slider_t *)widget;
	gui_var_t *var;
	float d;
	float t;
	float f;
	float max;
	float min;
	float *fvar;
	unsigned char *cvar;
	unsigned short *svar;
	
	
	
	if(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON)
	{
		t = 1.0 - (slider->widget.relative_mouse_x * 0.5 + 0.5);
		
		if(t > 1.0) t = 1.0;
		else if(t < 0.0) t = 0.0;
		
		slider->slider_position = t;
	}
	
	if(!slider->widget.var)
		return;
		
	var = slider->widget.var;	
	
	switch(var->type)
	{
		case GUI_VAR_FLOAT:
			
			max = slider->max_value.prev_var_value.float_var;
			min = slider->min_value.prev_var_value.float_var;
			fvar = (float *)var->addr;
			d = max - min;
			
			if(!(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON))
			{
				t = (*fvar - min) / d;
				/* var got modified externally, 
				so update the slider value, making
				sure the var value is properly clamped... */
				if(t > 1.0) t = 1.0;	
				else if(t < 0.0) t = 0.0;
				
				slider->slider_position = t;
			}
						
			*fvar = min + d * slider->slider_position;										
		break;
		
		case GUI_VAR_INT:
		
		break;
		
		case GUI_VAR_UNSIGNED_CHAR:
			max = (float)slider->max_value.prev_var_value.unsigned_char_var;
			min = (float)slider->min_value.prev_var_value.unsigned_char_var;
			cvar = (unsigned char *)var->addr;
			
			if(!cvar)
				break;	
			
			f = (float)(*cvar);
			
			d = max - min;
			
			if(!(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON))
			{
				t = (f - min) / d;
				/* var got modified externally, 
				so update the slider value, making
				sure the var value is properly clamped... */
				if(t > 1.0) t = 1.0;	
				else if(t < 0.0) t = 0.0;
				
				slider->slider_position = t;
			}		
			*cvar = (unsigned char)min + (unsigned char)(d * slider->slider_position);
		break;
		
		case GUI_VAR_POINTER_TO_UNSIGNED_CHAR:
			max = (float)slider->max_value.prev_var_value.unsigned_char_var;
			min = (float)slider->min_value.prev_var_value.unsigned_char_var;
			cvar = *(unsigned char **)var->addr;
			
			if(!cvar)
				break;
			
			f = (float)(*cvar);
			
			d = max - min;
			
			if(!(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON))
			{
				t = (f - min) / d;
				/* var got modified externally, 
				so update the slider value, making
				sure the var value is properly clamped... */
				if(t > 1.0) t = 1.0;	
				else if(t < 0.0) t = 0.0;
				
				slider->slider_position = t;
			}		
			*cvar = (unsigned char)min + (unsigned char)(d * slider->slider_position);	
		break;
		
		case GUI_VAR_UNSIGNED_SHORT:
			max = (float)slider->max_value.prev_var_value.unsigned_short_var;
			min = (float)slider->min_value.prev_var_value.unsigned_short_var;
			svar = (unsigned short *)var->addr;
			
			if(!svar)
				break;	
			
			f = (float)(*svar);
			
			d = max - min;
			
			if(!(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON))
			{
				t = (f - min) / d;
				/* var got modified externally, 
				so update the slider value, making
				sure the var value is properly clamped... */
				if(t > 1.0) t = 1.0;	
				else if(t < 0.0) t = 0.0;
				
				slider->slider_position = t;
			}		
			*svar = (unsigned short)min + (unsigned short)(d * slider->slider_position);	
		break;
		
		case GUI_VAR_POINTER_TO_UNSIGNED_SHORT:
			max = (float)slider->max_value.prev_var_value.unsigned_short_var;
			min = (float)slider->min_value.prev_var_value.unsigned_short_var;
			svar = *(unsigned short **)var->addr;
			
			if(!svar)
				break;
			
			f = (float)(*svar);
			
			d = max - min;
			
			if(!(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON))
			{
				t = (f - min) / d;
				/* var got modified externally, 
				so update the slider value, making
				sure the var value is properly clamped... */
				if(t > 1.0) t = 1.0;	
				else if(t < 0.0) t = 0.0;
				
				slider->slider_position = t;
			}		
			*svar = (unsigned short)min + (unsigned short)(d * slider->slider_position);	
		break;
	}
}

void gui_PostUpdateSlider(widget_t *widget)
{
	
}

















