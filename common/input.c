#include "input.h"
#include "r_main.h"
#include "engine.h"
//#include "console.h"
//#include "pew.h"
//#include "draw.h"

//input_cache input;
//extern console_t console;
//extern pew_t pew;
//extern renderer_t renderer;
//extern int engine_state;


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


float last_mouse_x = 0.0;
float last_mouse_y = 0.0;

SDL_Cursor *ibeam;
SDL_Cursor *arrow;
SDL_Cursor *h_arrows;
SDL_Cursor *v_arrows;
SDL_Cursor *dl_arrows;
SDL_Cursor *dr_arrows;

extern SDL_Window *window;
extern int r_width;
extern int r_height;
extern int r_window_width;
extern int r_window_height;
extern int engine_state;

short key_pos_map[SDL_NUM_SCANCODES];


/*
=============
input_Init
=============
*/
void input_Init()
{
	int i;
	
	ibeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	h_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	v_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	dl_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	dr_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	
	mouse_dx=0.0;
	mouse_dy=0.0;
	kb_event=(SDL_Event *)calloc(1, sizeof(SDL_Event));
	SDL_WarpMouseInWindow(window, r_window_width / 2, r_window_height / 2);
	bm_mouse = 0;
	registered_keys_count = 0;
	max_registered_keys = 0;
	registered_keys = NULL;
	
	for(i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		key_pos_map[i] = -1;
	}
	
	
	input_GetInput();
	return;
}

/*
=============
input_Finish
=============
*/
void input_Finish()
{
	free(kb_event);
	return;
}

/*
=============
input_GetInput
=============
*/
void input_GetInput()
{
	int bm;
	int i;
	int q;
	//SDL_Cursor *cursor;
	
	bm_mouse &= ~(MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN);
	
	while(SDL_PollEvent(kb_event))
	{		
		switch(kb_event->type)
		{
			case SDL_WINDOWEVENT:
				if(kb_event->window.event == SDL_WINDOWEVENT_CLOSE)
				{
					engine_SetEngineState(ENGINE_QUIT);
				}
			break;
			
			case SDL_MOUSEWHEEL:
				if(kb_event->wheel.y == 1)
				{
					bm_mouse |= MOUSE_WHEEL_UP;
				}
				else if(kb_event->wheel.y == -1)
				{
					bm_mouse |= MOUSE_WHEEL_DOWN;
				}
			break;
		}
	}
	kb_keys = (Uint8 *)SDL_GetKeyboardState(NULL);
	bm = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_y = r_window_height - mouse_y;
	
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_CLICKED)
	{
		bm_mouse &= ~MOUSE_LEFT_BUTTON_JUST_CLICKED;
	}
	
	if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_CLICKED)
	{
		bm_mouse &= ~MOUSE_RIGHT_BUTTON_JUST_CLICKED;
	}
	
	if(bm_mouse & MOUSE_LEFT_BUTTON_JUST_RELEASED)
	{
		bm_mouse &= ~MOUSE_LEFT_BUTTON_JUST_RELEASED;
	}
	
	if(bm_mouse & MOUSE_RIGHT_BUTTON_JUST_RELEASED)
	{
		bm_mouse &= ~MOUSE_RIGHT_BUTTON_JUST_RELEASED;
	}
	
	if(bm_mouse & MOUSE_MIDDLE_BUTTON_JUST_RELEASED)
	{
		bm_mouse &= ~MOUSE_MIDDLE_BUTTON_JUST_RELEASED;
	}
	
	if(bm_mouse & MOUSE_MIDDLE_BUTTON_JUST_RELEASED)
	{
		bm_mouse &= ~MOUSE_MIDDLE_BUTTON_JUST_RELEASED;
	}
	
	if(bm & 1)
	{
		if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
		{
			bm_mouse |= MOUSE_LEFT_BUTTON_JUST_CLICKED;
		}
		
		bm_mouse |= MOUSE_LEFT_BUTTON_CLICKED;
	}
	else
	{
		if(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED)
		{
			bm_mouse |= MOUSE_LEFT_BUTTON_JUST_RELEASED;
		}
		bm_mouse &= ~MOUSE_LEFT_BUTTON_CLICKED;
	}
	
	if(bm & 2)
	{
		if(!(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED))
		{
			bm_mouse |= MOUSE_MIDDLE_BUTTON_JUST_CLICKED;
		}
		
		bm_mouse |= MOUSE_MIDDLE_BUTTON_CLICKED;
	}
	else
	{
		if(bm_mouse & MOUSE_MIDDLE_BUTTON_CLICKED)
		{
			bm_mouse |= MOUSE_MIDDLE_BUTTON_JUST_RELEASED;
		}
		bm_mouse &= ~MOUSE_MIDDLE_BUTTON_CLICKED;
	}
	
	if(bm & 4)
	{
		if(!(bm_mouse & MOUSE_RIGHT_BUTTON_CLICKED))
		{
			bm_mouse |= MOUSE_RIGHT_BUTTON_JUST_CLICKED;
		}
		bm_mouse |= MOUSE_RIGHT_BUTTON_CLICKED;
	}
	else
	{
		if(bm_mouse & MOUSE_RIGHT_BUTTON_CLICKED)
		{
			bm_mouse |= MOUSE_RIGHT_BUTTON_JUST_RELEASED;
		}
		bm_mouse &= ~MOUSE_RIGHT_BUTTON_CLICKED;
	}
	
	for(i = 0; i < registered_keys_count; i++)
	{
		q = kb_keys[registered_keys[i].key];
		
		if(registered_keys[i].bm_flags & KEY_JUST_PRESSED)
		{
			registered_keys[i].bm_flags &= ~KEY_JUST_PRESSED;
		}
		else if(registered_keys[i].bm_flags & KEY_JUST_RELEASED)
		{
			registered_keys[i].bm_flags &= ~KEY_JUST_RELEASED;
		}
		
		if(q)
		{
			if(!(registered_keys[i].bm_flags & KEY_PRESSED))
			{
				registered_keys[i].bm_flags |= KEY_JUST_PRESSED;
			}
			registered_keys[i].bm_flags |= KEY_PRESSED;
		}
		else
		{
			if(registered_keys[i].bm_flags & KEY_PRESSED)
			{
				registered_keys[i].bm_flags |= KEY_JUST_RELEASED;
			}
			
			registered_keys[i].bm_flags &= ~KEY_PRESSED;
		}
	}
	
	
	normalized_mouse_x=(float)mouse_x/(float)r_window_width;
	normalized_mouse_y=(float)mouse_y/(float)r_window_height;
	
	normalized_mouse_x*=2.0;
	normalized_mouse_x-=1.0;
	
	normalized_mouse_y*=2.0;
	normalized_mouse_y-=1.0;
	
	//input.bm_mouse &= ~MOUSE_OVER_WIDGET;
	
	//if(!pew.b_console)
	if(engine_state == ENGINE_PLAYING || engine_state == ENGINE_EDITING)
	{
		
		if(last_mouse_x || last_mouse_y)
		{
			normalized_mouse_x = 0.0;
			normalized_mouse_y = 0.0;
			
			//printf("last mouse\n");
		}
		
		SDL_ShowCursor(0);
		SDL_WarpMouseInWindow(window, r_window_width/2, r_window_height/2);
		mouse_dx=normalized_mouse_x;
		mouse_dy=normalized_mouse_y;
		last_mouse_x = 0.0;
		last_mouse_y = 0.0;
	}
	else
	{
		SDL_ShowCursor(1);
		//cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
		//SDL_SetCursor(h_arrows);
		
		mouse_dx = normalized_mouse_x - last_mouse_x;
		mouse_dy = normalized_mouse_y - last_mouse_y;
		
		last_mouse_x = normalized_mouse_x;
		last_mouse_y = normalized_mouse_y;
	}
	
	//printf("%d\n", kb_event->window.event);
	
	/*if(kb_event->window.event == SDL_WINDOWEVENT_CLOSE)
	{
		printf("close!\n");
		engine_SetEngineState(ENGINE_QUIT);
	}*/
	
	
	//input_GetEsc();
	
	
	
	//printf("%f %f\n", input.normalized_mouse_x, input.normalized_mouse_y);
	//printf("%d\n", input.bm_mouse);
	
	return;
}

void input_SetCursor(int cursor)
{
	switch(cursor)
	{
		case CURSOR_ARROW:
			SDL_SetCursor(arrow);
		break;
		
		case CURSOR_HORIZONTAL_ARROWS:
			SDL_SetCursor(h_arrows);
		break;
		
		case CURSOR_VERTICAL_ARROWS:
			SDL_SetCursor(v_arrows);
		break;
		
		case CURSOR_I_BEAM:
			SDL_SetCursor(ibeam);
		break;
		
		case CURSOR_LEFT_DIAGONAL_ARROWS:
			SDL_SetCursor(dl_arrows);
		break;
		
		case CURSOR_RIGHT_DIAGONAL_ARROWS:
			SDL_SetCursor(dr_arrows);
		break;
	}
}


int input_GetKeyPressed(int key)
{
	return kb_keys[key];
}

int input_GetKeyStatus(int key)
{
	if(key_pos_map[key] >= 0)
	{
		return registered_keys[key_pos_map[key]].bm_flags;
	}
	return 0;
	
}


int input_GetMouseButton(int button)
{
	//return input.bm_mouse&button;
	int i = 0;
	switch(button)
	{
		case SDL_BUTTON_LEFT:
			i = bm_mouse & (MOUSE_LEFT_BUTTON_CLICKED | MOUSE_LEFT_BUTTON_JUST_CLICKED | MOUSE_LEFT_BUTTON_JUST_RELEASED);
		break;
		
		case SDL_BUTTON_MIDDLE:
			i = bm_mouse & (MOUSE_MIDDLE_BUTTON_CLICKED | MOUSE_MIDDLE_BUTTON_JUST_CLICKED | MOUSE_MIDDLE_BUTTON_JUST_RELEASED);
		break;
		
		case SDL_BUTTON_RIGHT:
			i = bm_mouse & (MOUSE_RIGHT_BUTTON_CLICKED | MOUSE_RIGHT_BUTTON_JUST_CLICKED | MOUSE_RIGHT_BUTTON_JUST_RELEASED);
		break;
	}
	
	return i;
}

int input_RegisterKey(int key)
{
	key_t *t;
	int i;
	
	if(key_pos_map[key] != -1)
	{
		return -1;
	}
	
	if(!registered_keys)
	{
		registered_keys = (key_t *)malloc(sizeof(key_t));
		max_registered_keys = 1;
	}
	else if(registered_keys_count >= max_registered_keys)
	{
		t =	(key_t *)malloc(sizeof(key_t) * (max_registered_keys + 1));
		memcpy(t, registered_keys, sizeof(key_t) * max_registered_keys);
		free(registered_keys);
		registered_keys = t;
		max_registered_keys++;
	}
	
	registered_keys[registered_keys_count].key = key;
	registered_keys[registered_keys_count].bm_flags = 0;
	key_pos_map[key] = registered_keys_count;
	registered_keys_count++;
	
}

int input_UnregisterKey(int key)
{
	int i;
	int k;
	
	if(key_pos_map[key] == -1)
	{
		return -1;
	}
	
	if(registered_keys_count < max_registered_keys)
	{
		i = max_registered_keys - 1;
		k = registered_keys[i].key;
		
		registered_keys[key_pos_map[key]] = registered_keys[i];
		key_pos_map[k] = key_pos_map[key];
	}

	key_pos_map[key] = -1;
	registered_keys_count--;
}








