#include <stdlib.h>
#include <string.h>

#include "gui_widget_bar.h"
#include "gui_dropdown.h"


widget_bar_t *gui_AddWidgetBar(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags)
{
	widget_bar_t *bar;
	
	bar = malloc(sizeof(widget_bar_t));
	
	if(w < WIDGET_MIN_SIZE) w = WIDGET_MIN_SIZE;
	if(h < WIDGET_MIN_SIZE) h = WIDGET_MIN_SIZE;
	
	bar->widget.last_nestled = NULL;
	bar->widget.nestled = NULL;
	bar->widget.next = NULL;
	bar->widget.prev = NULL;
	bar->widget.x = x;
	bar->widget.y = y;
	bar->widget.w = w / 2;
	bar->widget.h = h / 2;
	bar->widget.bm_flags = 0;
	bar->widget.type = WIDGET_BAR;
	bar->widget.parent = widget;
	bar->widget.widget_callback = NULL;
	bar->widget.name = strdup(name);
	bar->type = WIDGET_NONE;
	bar->process_fn = NULL;
	bar->bm_flags = bm_flags;
	bar->active_widget = NULL;
	
	if(!widget->nestled)
	{
		widget->nestled = (widget_t *)bar;
	}
	else
	{
		widget->last_nestled->nestled = (widget_t *)bar;
	}
	
	widget->last_nestled = (widget_t *)bar;
		
	return bar;
}

void gui_AddWidgetToBar(widget_t *widget, widget_bar_t *bar)
{
	if(widget)
	{
		if(bar)
		{
			/* first widget added defines the type of this bar... */
			if(bar->type == WIDGET_NONE)
			{
				bar->type = widget->type;
				bar->widget.nestled = widget;
				
				/* set proper process function pointer here... */
				switch(bar->type)
				{
					case WIDGET_BUTTON:
					
					break;
					
					case WIDGET_CHECKBOX:
					
					break;
					
					case WIDGET_DROPDOWN:
						bar->process_fn = gui_UpdateDropdownBar;
					break;
				}
				
			}
			else
			{
				/* do not add the widget if it has
				a different type from the bar... */
				if(widget->type != bar->type)
					return;
					
				bar->widget.last_nestled->next = widget;
				widget->prev = bar->widget.last_nestled;
			}
			
			widget->parent = (widget_t *) bar;
			
			bar->widget.last_nestled = widget;
			bar->bm_flags |= WIDGET_BAR_ADJUST_WIDGETS;
			
		}
	}
}

void gui_AdjustBar(widget_t *widget)
{
	widget_bar_t *bar = (widget_bar_t *)widget;
	widget_t *r;
	int height = 0;
	int width = 0;
	int next_x;
	//int height;
	
	r = widget->nestled;
	
	if(!r)
		return;
	
	while(r)
	{
		if(r->h > height) height = r->h;
		
		width += r->w;
		
		r = r->next;
	}
	
	bar->widget.h = height;
	
	if(!(bar->bm_flags & WIDGET_BAR_FIXED_SIZE))	
		bar->widget.w = width;
	
	r = widget->nestled;
	
	next_x = -bar->widget.w + r->w;
		
	while(r)
	{
		r->x = next_x;
		next_x += r->w;
		r = r->next;
			
		if(r) next_x += r->w;
	}
			
	bar->bm_flags &= ~WIDGET_BAR_ADJUST_WIDGETS;
	
}


void gui_UpdateWidgetBar(widget_t *widget)
{
	widget_bar_t *bar;
	widget_t *w;
	widget_t *active;
	
	bar = (widget_bar_t *)widget;
	
	if(bar->bm_flags & WIDGET_BAR_ADJUST_WIDGETS)
	{
		gui_AdjustBar(widget);
	}
}


void gui_PostUpdateWidgetBar(widget_t *widget)
{
	widget_bar_t *bar = (widget_bar_t *)widget;
	if(bar->process_fn)
	{
		bar->process_fn(widget);
	}
}







