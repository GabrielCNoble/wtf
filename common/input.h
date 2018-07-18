#ifndef INPUT_H
#define INPUT_H

//#include "conf.h"
//#include "includes.h"
#include "SDL2/SDL_scancode.h"
#include "SDL2/SDL_mouse.h"
#include "SDL2/SDL_keyboard.h"

#include "vector_types.h"

#define TEXT_BUFFER_SIZE 64

enum MOUSE_FLAGS
{
	MOUSE_LEFT_BUTTON_CLICKED = 1,
	MOUSE_LEFT_BUTTON_JUST_CLICKED = 1 << 1,
	MOUSE_LEFT_BUTTON_JUST_RELEASED = 1 << 2,
	MOUSE_LEFT_BUTTON_DOUBLE_CLICKED = 1 << 3,
	
	MOUSE_RIGHT_BUTTON_CLICKED = 1 << 4,
	MOUSE_RIGHT_BUTTON_JUST_CLICKED = 1 << 5,
	MOUSE_RIGHT_BUTTON_JUST_RELEASED = 1 << 6,
	MOUSE_RIGHT_BUTTON_DOUBLE_CLICKED = 1 << 7,
	
	MOUSE_MIDDLE_BUTTON_CLICKED = 1 << 8,
	MOUSE_MIDDLE_BUTTON_JUST_CLICKED = 1 << 9,
	MOUSE_MIDDLE_BUTTON_JUST_RELEASED = 1 << 10,
	
	MOUSE_WHEEL_UP = 1 << 11,
	MOUSE_WHEEL_DOWN = 1 << 12,
	
	MOUSE_OVER_WIDGET = 1 << 13,
};

enum CURSOR
{
	CURSOR_ARROW,
	CURSOR_HORIZONTAL_ARROWS,
	CURSOR_VERTICAL_ARROWS,
	CURSOR_I_BEAM,
	CURSOR_LEFT_DIAGONAL_ARROWS,
	CURSOR_RIGHT_DIAGONAL_ARROWS
};

enum KEYBOARD_FLAGS
{
	KEY_PRESSED = 1,
	KEY_JUST_PRESSED = 1 << 1,
	KEY_JUST_RELEASED = 1 << 2,
};

enum MOUSE_BUTTON
{
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_WHEEL,
};

typedef struct
{
	short key;
	short bm_flags;
}key_t;
/*
typedef struct
{
	Uint8 *kb_keys;
	SDL_Event *kb_event;
	int max_registered_keys;
	int registered_keys_count;
	key_t *registered_keys;
	int bm_mouse;
	int mouse_x;
	int mouse_y;
	float normalized_mouse_x;
	float normalized_mouse_y;
	float mouse_dx;
	float mouse_dy;
}input_cache;*/

#ifdef __cplusplus
extern "C"
{
#endif

int input_Init();

void input_Finish();

void input_GetInput(double delta_time);

void input_GetMouseDelta(float *dx, float *dy);

void input_SetCursor(int cursor);

void input_EnableTextInput(int enable);

void input_BufferTextInput();

int *input_GetTextBuffer(); 

void input_ClearTextBuffer();

char *input_GetKeyArray();

int input_GetKeyPressed(int key);

int input_GetKeyStatus(int key);

int input_GetMouseButton(int button);

int input_RegisterKey(int key);

int input_UnregisterKey(int key);

void input_GetEsc();

int input_ParseFloat(float *value, char *str);

int input_ParseInt(int *value, char *str);

int input_ParseShort(short *value, char *str);

int input_ParseVec3(vec3_t *value, char *str);




/*PEWAPI void input_ProcessConsoleInput();*/

#ifdef __cplusplus
}
#endif



#endif /* INPUT_H */
