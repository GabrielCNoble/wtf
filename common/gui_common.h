#ifndef GUI_COMMON_H
#define GUI_COMMON_H

#include "matrix.h"
#include "vector.h"

#include "SDL2/SDL_surface.h"

#define WIDGET_MIN_SIZE 4
#define WIDGET_BORDER_WIDTH 10

#define DROPDOWN_HEIGHT 20
#define OPTION_HEIGHT 20
#define SLIDER_HEIGHT 20
#define SLIDER_HANDLE_SIZE 16.0
#define CHECKBOX_MIN_SIZE 8
#define OPTION_LIST_MINIMUM_MAXIMUM_VISIBLE_OPTIONS 4 

#define TEXT_FIELD_CURSOR_BLINK_TIME 180
#define TEXT_FIELD_BUFFER_SIZE 512

#define WIDGET_NAME_MAX_LEN 64


enum WIDGET_FLAGS
{
	WIDGET_MOUSE_OVER = 										1,
	WIDGET_HAS_LEFT_MOUSE_BUTTON = 								1 << 1,
	WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON = 					1 << 2,		/* active for a single frame... */
	WIDGET_HAS_RIGHT_MOUSE_BUTTON = 							1 << 3,
	WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON = 					1 << 4,		/* active for a single frame... */	
	//WIDGET_MOUSE_OVER_RIGHT_BORDER = 							1 << 5,
	//WIDGET_MOUSE_OVER_LEFT_BORDER = 							1 << 6,
	//WIDGET_MOUSE_OVER_TOP_BORDER = 								1 << 7,
	//WIDGET_MOUSE_OVER_BOTTOM_BORDER = 							1 << 8,
	WIDGET_INVISIBLE = 											1 << 5,		/* invisible widgets are not processed nor drawn... */
	WIDGET_RENDER_TEXT = 										1 << 6,
	WIDGET_TRACK_VAR = 											1 << 7,	/* this widget is currently tracking a gui_var_t... */
	WIDGET_IGNORE_EDGE_CLIPPING = 								1 << 8,	/* allow a widget to detect mouse over even if it's outside it's parent widget... */
	WIDGET_JUST_CREATED =										1 << 9,	/* active for a single frame, set after a widget gets creates (becomes visible)... */
	WIDGET_SHOW_NAME = 											1 << 10,
	WIDGET_RENDER_NAME =										1 << 11,
	WIDGET_OUT_OF_BOUNDS = 										1 << 12, 	/* active when a widget is completely outside it's parent bounds (used for faster clipping)... */
	WIDGET_JUST_RECEIVED_LEFT_MOUSE_DOUBLE_CLICK = 				1 << 13,
	WIDGET_JUST_RECEIVED_MOUSE_WHEEL_UP =						1 << 14,
	WIDGET_JUST_RECEIVED_MOUSE_WHEEL_DOWN =						1 << 15,
	WIDGET_HAS_MIDDLE_MOUSE_BUTTON = 							1 << 16,
	WIDGET_NOT_AS_TOP = 										1 << 17,	/* avoids a widget becoming the top widget when clicked upon... */
	WIDGET_NO_HIGHLIGHT = 										1 << 18,
	WIDGET_DRAW_OUTLINE = 										1 << 19,
	WIDGET_DONT_RECEIVE_MOUSE = 								1 << 20,	/* makes a widget (and everything nestled within it) to ignore the mouse... */
};

enum WIDGET_EDGE_FLAGS
{
	WIDGET_MOUSE_OVER_RIGHT_EDGE = 1,
	WIDGET_MOUSE_OVER_LEFT_EDGE = 1 << 1,
	WIDGET_MOUSE_OVER_TOP_EDGE = 1 << 2,
	WIDGET_MOUSE_OVER_BOTTOM_EDGE = 1 << 3,
	
	WIDGET_RIGHT_EDGE_GRABBED = 1 << 4,
	WIDGET_LEFT_EDGE_GRABBED = 1 << 5,
	WIDGET_TOP_EDGE_GRABBED = 1 << 6,
	WIDGET_BOTTOM_EDGE_GRABBED = 1 << 7,
	
	WIDGET_LEFT_EDGE_ENABLED = 1 << 8,
	WIDGET_RIGHT_EDGE_ENABLED = 1 << 9,
	WIDGET_TOP_EDGE_ENABLED = 1 << 10,
	WIDGET_BOTTOM_EDGE_ENABLED = 1 << 11,
	
	WIDGET_HEADER = 1 << 12,
};

enum WIDGET_JUSTIFICATION_FLAGS
{
	WIDGET_JUSTIFY_LEFT = 1,
	WIDGET_JUSTIFY_RIGHT = 1 << 1,
	WIDGET_JUSTIFY_TOP = 1 << 2,
	WIDGET_JUSTIFY_BOTTOM = 1 << 3,
};


enum WIDGET_TYPES
{
	WIDGET_NONE = 0,
	WIDGET_BASE,
	WIDGET_BUTTON,
	WIDGET_CHECKBOX,
	WIDGET_SLIDER,
	WIDGET_DROPDOWN,
	WIDGET_OPTION_LIST,
	WIDGET_OPTION,
	WIDGET_BAR,
	WIDGET_TEXT_FIELD,
	WIDGET_SURFACE,
	WIDGET_ITEM_LIST,
	WIDGET_ITEM_TREE,
	WIDGET_LAST,
};

enum BUTTON_FLAGS
{
	BUTTON_PRESSED = 1,
	BUTTON_TOGGLE = 1 << 1,
	BUTTON_DRAW_TEST = 1 << 2,
	BUTTON_DONT_RECEIVE_CLICK = 1 << 3
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
	DROPDOWN_DISPLAY_SELECTED_OPTION = 1 << 2,
};

enum OPTION_LIST_FLAGS
{
	OPTION_LIST_UPDATE_EXTENTS = 1,
	OPTION_LIST_SCROLLER = 1 << 1,
	OPTION_LIST_NO_OPTION_DIVISIONS = 1 << 2, 
	OPTION_LIST_DOUBLE_CLICK_SELECTION = 1 << 3,
	OPTION_LIST_DONT_TRANSLATE = 1 << 4,
	OPTION_LIST_DONT_RECEIVE_MOUSE = 1 << 5,
};

enum OPTION_FLAGS
{
	OPTION_INVALID = 1, 
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
	TEXT_FIELD_DRAW_TEXT_SELECTED = 1 << 4,
	TEXT_FIELD_UPDATED = 1 << 5,
};

enum SLIDER_FLAGS
{
	SLIDER_SHOW_VALUE = 1,
	SLIDER_CLAMP_VAR = 1 << 1,						
};

enum ITEM_LIST_FLAGS
{
	ITEM_LIST_UPDATE = 1,
	ITEM_LIST_HORIZONTAL_ORDER = 1 << 1,
	ITEM_LIST_SCROLLER = 1 << 2,
	ITEM_LIST_SHOW_ITEM_NAME = 1 << 3,
	ITEM_LIST_NAME_INSIDE_ITEM = 1 << 4,
	ITEM_LIST_NAME_JUSTIFY_LEFT = 1 << 5,
	ITEM_LIST_NAME_JUSTIFY_RIGHT = 1 << 6,
	ITEM_LIST_NAME_JUSTIFY_TOP = 1 << 7,
	ITEM_LIST_DOUBLE_CLICK_SELECTION = 1 << 8,
};







enum GUI_VAR_TYPES
{
	GUI_VAR_CHAR,
	GUI_VAR_POINTER_TO_CHAR,
	GUI_VAR_UNSIGNED_CHAR,
	GUI_VAR_POINTER_TO_UNSIGNED_CHAR,
	GUI_VAR_SHORT,
	GUI_VAR_UNSIGNED_SHORT,
	GUI_VAR_POINTER_TO_UNSIGNED_SHORT,
	GUI_VAR_POINTER_TO_SHORT,
	GUI_VAR_INT,
	GUI_VAR_POINTER_TO_INT,
	GUI_VAR_FLOAT,
	GUI_VAR_POINTER_TO_FLOAT,
	GUI_VAR_DOUBLE,
	GUI_VAR_POINTER_TO_DOUBLE,
	GUI_VAR_VEC2_T,
	GUI_VAR_VEC3_T,
	GUI_VAR_VEC4_T,
	GUI_VAR_MAT3_T,
	GUI_VAR_MAT4_T,
	GUI_VAR_ALLOCD_STRING,
	GUI_VAR_STRING,
};


enum GUI_VAR_FLAGS
{
	GUI_VAR_VALUE_HAS_CHANGED = 1,
	GUI_VAR_POINTER_TO_TYPE = 1 << 1
};

typedef struct gui_var_t
{
	union
	{
		//mat4_t mat4_t_var;
		//mat3_t mat3_t_var;
		vec4_t vec4_t_var;
		vec3_t vec3_t_var;
		vec2_t vec2_t_var;
		double double_var;
		float float_var;
		int int_var;
		short short_var;
		unsigned short unsigned_short_var;
		char char_var;
		unsigned char unsigned_char_var;
		char *str_var;
		char **str_ptr_var;
	}prev_var_value;
	
	void *base;
	int offset;
	
	/* used to test whether the pointer
	which this var points to has changed... */
	//void *prev_ptr;
	
	char *name;
	void *addr;
	
	short type;
	short bm_flags;
	struct gui_var_t *next;
	
}gui_var_t;

/*typedef struct linked_edge_t
{
	struct linked_edge_t *next;
	
}linked_edge_t;*/

typedef struct linked_edge_t linked_edge_t;

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
	struct widget_t *parent;
	
	//struct widget_t *bottom_edge_of;					/* which widget uses this widget's top edge as it's bottom edge... */
	//struct widget_t *left_edge_of;						/* which widget uses this widget's right edge as it's left edge...  */
	
	//linked_edge_t *bottom_edge_of;
	//linked_edge_t *left_edge_of;
	linked_edge_t *linked_edges;
	
	//struct widget_t *first_parent;
	void (*widget_callback)(struct widget_t *widget);
	void (*process_callback)(struct widget_t *widget);
	unsigned int bm_flags;
	unsigned int edge_flags;
	unsigned int jusitication_flags;
	unsigned int gl_tex_handle;							/* texture to be used as this widget's color... */
	short type;
	int unique_index;
	void *data;											/* general purpouse field... */
	char *name;
	SDL_Surface *rendered_name;
}widget_t;

enum WIDGET_EDGES
{
	WIDGET_LEFT_EDGE = 1,
	WIDGET_RIGHT_EDGE = 1 << 1,
	WIDGET_TOP_EDGE = 1 << 2,
	WIDGET_BOTTOM_EDGE = 1 << 3,
};

struct linked_edge_t
{
	linked_edge_t *next;
	widget_t *widget;
	int linked_widget_edge;				
	int this_widget_edge;
	short offset;
};


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
	int bm_check_flags;							/* bits to toggle from a bitmask when used with a gui_var_t */
	char *checkbox_text;
	SDL_Surface *rendered_text;					/* text to display alongside the checkbox... */
}checkbox_t;

typedef struct
{
	widget_t widget;
	struct option_t *active_option;
	struct option_t *selected_option;
	widget_t *first_unused;
	short option_count;
	short active_option_index;
	short selected_option_index;
	short bm_option_list_flags;
	short first_x;
	short first_y;
	unsigned short y_offset;								/* modified by the scroller... */
	short max_visible_options;
}option_list_t;

typedef struct option_t
{
	widget_t widget;
	int index;
	int unique_index;
	int bm_option_flags;
	char *option_text;
	SDL_Surface *rendered_text;								/* rendered only when it changes... */
	int rendered_string;
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
	widget_t *first_widget;
	short type;
	short bm_flags;
	void (*process_fn)(widget_t *);
}widget_bar_t;

typedef struct
{
	widget_t widget;
	//gui_var_t *var;
	gui_var_t min_value;
	gui_var_t max_value;
	float slider_position;
	int bm_slider_flags;
}slider_t;

typedef struct
{
	widget_t widget;
	unsigned int framebuffer_id;
	unsigned int color_texture;
	unsigned int depth_texture;
		
	int prev_viewport[4];
	vec4_t clear_color;
	vec4_t prev_clear_color;
	unsigned int prev_draw_framebuffer;
	unsigned int prev_read_framebuffer;
	unsigned int prev_texture;
	unsigned int prev_draw_buffer;
}wsurface_t;

typedef struct
{
	widget_t *widget;
	char *text;
	SDL_Surface *rendered_text;
}tag_t;

typedef struct
{
	widget_t widget;
	
	widget_t *active_item;
	widget_t *selected_item;
	widget_t *first_unused;
	
	unsigned short type;
	unsigned short item_count;
	unsigned int flags;
	
	unsigned short active_item_index;
	unsigned short selected_item_index;
	
	unsigned short item_w;
	unsigned short item_h;
	
	short x_offset;
	short y_offset;
	
}item_list_t;

typedef struct
{
	widget_t widget;
	
	int flags;
	
	short x_offset;
	short y_offset;
	
}item_tree_t;


#endif






