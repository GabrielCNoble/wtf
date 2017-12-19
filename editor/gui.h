#ifndef GUI_H
#define GUI_H

#define WIDGET_MIN_SIZE 32
#define WIDGET_BORDER_WIDTH 10


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
};


enum WIDGET_TYPES
{
	WIDGET_NONE = 0,
	WIDGET_BUTTON,
	WIDGET_CHECKBOX,
	WIDGET_SLIDER,
	WIDGET_DROPDOWN,
};

enum BUTTON_FLAGS
{
	BUTTON_PRESSED = 1,
	BUTTON_TOGGLE = 1 << 1
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
	int bm_flags;
	int type;
	char *name;
}widget_t;


typedef struct
{
	widget_t widget;
	char *button_text;
	short bm_button_flags;
}button_t;

typedef struct
{
	widget_t widget;
	int check_box_state;
}checkbox_t;

typedef struct
{
	widget_t widget;
	int dropdown_state;
	int option_count;
	int selected_option;
}dropdown_t;

void gui_Init();

void gui_Finish();

widget_t *gui_CreateWidget(char *name, short x, short y, short w, short h);

button_t *gui_AddButtonToWidget(widget_t *widget, char *name, short x, short y, short w, short h, short bm_flags);

//void gui_SetTopWidget(widget_t *widget);

void gui_ProcessGUI();

void gui_UpdateGUIProjectionMatrix();

#endif








