#ifndef GUI_COMMON_H
#define GUI_COMMON_H

#include "matrix.h"
#include "vector.h"

#include "SDL2/SDL_surface.h"

#define WIDGET_MIN_SIZE 32
#define WIDGET_BORDER_WIDTH 10

#define DROPDOWN_HEIGHT 20
#define OPTION_HEIGHT 20
#define CHECKBOX_MIN_SIZE 8

#define TEXT_FIELD_CURSOR_BLINK_TIME 180
#define TEXT_FIELD_BUFFER_SIZE 128


enum WIDGET_FLAGS
{
	WIDGET_MOUSE_OVER = 										1,
	WIDGET_HAS_LEFT_MOUSE_BUTTON = 								1 << 1,
	WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON = 					1 << 2,			/* active for a single frame... */
	WIDGET_HAS_RIGHT_MOUSE_BUTTON = 							1 << 3,
	WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON = 					1 << 4,		/* active for a single frame... */	
	WIDGET_MOUSE_OVER_RIGHT_BORDER = 							1 << 5,
	WIDGET_MOUSE_OVER_LEFT_BORDER = 							1 << 6,
	WIDGET_MOUSE_OVER_TOP_BORDER = 								1 << 7,
	WIDGET_MOUSE_OVER_BOTTOM_BORDER = 							1 << 8,
	WIDGET_INVISIBLE = 											1 << 9,		/* invisible widgets are not processed nor drawn... */
	WIDGET_RENDER_TEXT = 										1 << 10,
	WIDGET_TRACK_VAR = 											1 << 11,
	WIDGET_IGNORE_EDGE_CLIPPING = 								1 << 12,	/* allow a widget to detect mouse over events even if it's outside it's parent widget... */
};


enum WIDGET_TYPES
{
	WIDGET_NONE = 0,
	WIDGET_BUTTON,
	WIDGET_CHECKBOX,
	WIDGET_SLIDER,
	WIDGET_DROPDOWN,
	WIDGET_OPTION_LIST,
	WIDGET_OPTION,
	WIDGET_BAR,
	WIDGET_TEXT_FIELD,
};

enum BUTTON_FLAGS
{
	BUTTON_PRESSED = 1,
	BUTTON_TOGGLE = 1 << 1
};

enum CHECKBOX_FLAGS
{
	CHECKBOX_CHECKED = 1,
	CHECKBOX_DISPLAY_TEXT = 1 << 1,
};

enum DROPDOWN_FLAGS
{
	DROPDOWN_DROPPED = 1,
	DROPDOWN_JUST_DROPPED = 1 << 1,
};

enum OPTION_LIST_FLAGS
{
	OPTION_LIST_UPDATE_EXTENTS = 1,
	OPTION_LIST_SCROLLER = 1 << 1,
	OPTION_LIST_NO_OPTION_DIVISIONS = 1 << 2, 
};

enum WIDGET_BAR_FLAGS
{
	WIDGET_BAR_JUSTIFY_LEFT = 1,
	WIDGET_BAR_JUSTIFY_RIGHT = 1 << 1,
	WIDGET_BAR_ADJUST_WIDGETS = 1 << 2,
	WIDGET_BAR_FIXED_SIZE = 1 << 3,
};

enum TEXT_FIELD_FLAGS
{
	TEXT_FIELD_READ_ONLY = 1,
	TEXT_FIELD_NO_WRITE = 1 << 1,
	TEXT_FIELD_RECEIVING_TEXT =  1 << 2,
	TEXT_FIELD_CURSOR_VISIBLE = 1 << 3,
};

enum GUI_VAR_TYPES
{
	GUI_VAR_CHAR,
	GUI_VAR_SHORT,
	GUI_VAR_INT,
	GUI_VAR_FLOAT,
	GUI_VAR_DOUBLE,
	GUI_VAR_VEC2_T,
	GUI_VAR_VEC3_T,
	GUI_VAR_VEC4_T,
	GUI_VAR_MAT3_T,
	GUI_VAR_MAT4_T,
	GUI_VAR_STRING,
};


enum GUI_VAR_FLAGS
{
	GUI_VAR_VALUE_HAS_CHANGED = 1,
};

typedef struct gui_var_t
{
	union
	{
		mat4_t mat4_t_var;
		mat3_t mat3_t_var;
		vec4_t vec4_t_var;
		vec3_t vec3_t_var;
		vec2_t vec2_t_var;
		double double_var;
		float float_var;
		int int_var;
		short short_var;
		char char_var;
		char *str_var;
	}prev_var_value;
	
	char *name;
	void *addr;
	
	short type;
	short bm_flags;
	
	struct gui_var_t *next;
	
}gui_var_t;


typedef struct widget_t
{
	short x;
	short y;
	short w;
	short h;
	float relative_mouse_x;
	float relative_mouse_y;
	gui_var_t *var;
	struct widget_t *next;
	struct widget_t *prev;
	struct widget_t *nestled;
	struct widget_t *last_nestled;
	struct widget_t *top;
	//struct widget_t *nestled_top;
	struct widget_t *parent;
	//struct widget_t *first_parent;
	void (*widget_callback)(struct widget_t *widget);
	short bm_flags;
	short type;
	char *name;
}widget_t;


typedef struct
{
	widget_t widget;
	char *button_text;
	SDL_Surface *rendered_text;
	short bm_button_flags;
}button_t;

typedef struct
{
	widget_t widget;
	int bm_checkbox_flags;
	char *checkbox_text;
	SDL_Surface *rendered_text;					/* text to display alongside the checkbox... */
}checkbox_t;

typedef struct
{
	widget_t widget;
	struct option_t *active_option;
	short option_count;
	short active_option_index;
	short bm_option_list_flags;
	short align;
}option_list_t;

typedef struct
{
	widget_t widget;
	int index;
	int unique_index;
	char *option_text;
	SDL_Surface *rendered_text;								/* rendered only when it changes... */
}option_t;

typedef struct
{
	widget_t widget;
	unsigned char bm_dropdown_flags;
	unsigned char align0;
	unsigned char align1;
	unsigned char align2;
	char *dropdown_text;
	SDL_Surface *rendered_text;
}dropdown_t;

typedef struct
{
	widget_t widget;
	widget_t *active_widget;
	short type;
	short bm_flags;
	void (*process_fn)(widget_t *);
}widget_bar_t;

typedef struct
{
	widget_t *widget;
	gui_var_t *var;
	gui_var_t min_value;
	gui_var_t max_value;
	float slider_position;
}slider_t;



#endif
