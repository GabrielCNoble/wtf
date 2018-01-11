#ifndef GUI_H
#define GUI_H

#include "SDL2/SDL_surface.h"

#define WIDGET_MIN_SIZE 32
#define WIDGET_BORDER_WIDTH 10

#define DROPDOWN_HEIGHT 20
#define OPTION_HEIGHT 20
#define CHECKBOX_MIN_SIZE 8


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
};

enum OPTION_LIST_FLAGS
{
	OPTION_LIST_UPDATE_EXTENTS = 1,
};

typedef struct widget_t
{
	short x;
	short y;
	short w;
	short h;
	float relative_mouse_x;
	float relative_mouse_y;
	struct widget_t *next;
	struct widget_t *prev;
	struct widget_t *nestled;
	struct widget_t *last_nestled;
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

void gui_Init();

void gui_Finish();

widget_t *gui_CreateWidget(char *name, short x, short y, short w, short h);

button_t *gui_AddButton(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags, void (*button_callback)(widget_t *widget));

checkbox_t *gui_AddCheckBox(widget_t *widget, short x, short y, short w, short h, short bm_flags, void (*checkbox_callback)(widget_t *widget));

dropdown_t *gui_AddDropDown(widget_t *widget, char *name, char *text, short x, short y, short w, short bm_flags, void (*dropdown_callback)(widget_t *widget));

void gui_AddOption(dropdown_t *dropdown, char *name, char *text);

void gui_NestleOption(option_list_t *option_list, int option_index, char *name, char *text);

void gui_SetAsTop(widget_t *widget);

void gui_RenderText(widget_t *widget);

void gui_ProcessGUI();

void gui_UpdateGUIProjectionMatrix();

#endif








