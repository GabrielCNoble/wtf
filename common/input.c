#include "input.h"
//#include "renderer.h"
#include "engine.h"
#include "memory.h"

#include "SDL2\SDL.h"
//#include "SDL2\SDL_scancode.h"
//#include "console.h"
//#include "pew.h"
//#include "draw.h"

//input_cache input;
//extern console_t console;
//extern pew_t pew;
//extern renderer_t renderer;
//extern int engine_state;

#define DOUBLE_CLICK_TIME 250.0


Uint8 *kb_keys = NULL;
SDL_Event kb_event;
int max_registered_keys = 0;
int registered_keys_count = 0;
//key_t *registered_keys = NULL;
int bm_mouse = 0;
int mouse_x = 0;
int mouse_y = 0;
float normalized_mouse_x = 0.0;
float normalized_mouse_y = 0.0;
float mouse_dx = 0.0;
float mouse_dy = 0.0;

int text_buffer_in = 0;
int text_buffer_out = 0;
int text_buffer[TEXT_BUFFER_SIZE];

int b_text_input = 0;


float last_mouse_x = 0.0;
float last_mouse_y = 0.0;

float mouse_left_button_timer = 0.0;
float mouse_right_button_timer = 0.0;

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
key_t registered_keys[SDL_NUM_SCANCODES];



#ifdef __cplusplus
extern "C"
{
#endif

/*
=============
input_Init
=============
*/
int input_Init()
{
	int i;
	float f;

	ibeam = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	h_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	v_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	dl_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	dr_arrows = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);

	mouse_dx=0.0;
	mouse_dy=0.0;
	//kb_event=(SDL_Event *)calloc(5, sizeof(SDL_Event));
	//SDL_WarpMouseInWindow(window, r_window_width / 2, r_window_height / 2);
	bm_mouse = 0;
	registered_keys_count = 0;
	//max_registered_keys = 0;
	//registered_keys = memory_Malloc(sizeof(key_t) * SDL_NUM_SCANCODES);
	//registered_keys = NULL;

	for(i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		key_pos_map[i] = -1;
	}
	/*if(input_ParseFloat(&f, "25.0"))
		printf("%f\n", f);*/


	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);

	input_GetInput(0.0);
	return 1;
}

/*
=============
input_Finish
=============
*/
void input_Finish()
{
	//free(kb_event);
	return;
}

/*
=============
input_GetInput
=============
*/
void input_GetInput(double delta_time)
{
	int bm;
	int i;
	int q;
	//SDL_Cursor *cursor;

	bm_mouse &= ~(MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN | MOUSE_OVER_WIDGET);

	if(!b_text_input)
	{
		while(SDL_PollEvent(&kb_event))
		{
			switch(kb_event.type)
			{
				case SDL_WINDOWEVENT:
					if(kb_event.window.event == SDL_WINDOWEVENT_CLOSE)
					{
						engine_SetEngineState(ENGINE_QUIT);
					}
				break;

				case SDL_MOUSEWHEEL:
					if(kb_event.wheel.y == 1)
					{
						bm_mouse |= MOUSE_WHEEL_UP;
					}
					else if(kb_event.wheel.y == -1)
					{
						bm_mouse |= MOUSE_WHEEL_DOWN;
					}
				break;
			}
		}

		kb_keys = (Uint8 *)SDL_GetKeyboardState(NULL);

		for(i = 0; i < registered_keys_count; i++)
		{
			q = kb_keys[registered_keys[i].key];

			registered_keys[i].bm_flags &= ~(KEY_JUST_RELEASED | KEY_JUST_PRESSED);

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
	}
	else
	{
		for(i = 0; i < registered_keys_count; i++)
		{
			q = kb_keys[registered_keys[i].key];
			registered_keys[i].bm_flags &= ~(KEY_JUST_RELEASED | KEY_JUST_PRESSED | KEY_PRESSED);
		}

		input_BufferTextInput();
	}


    if(engine_state == ENGINE_PLAYING)
    {
        SDL_SetRelativeMouseMode(1);
        bm = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
        mouse_y = -mouse_y;

        last_mouse_x = 0.0;
        last_mouse_y = 0.0;

        normalized_mouse_x = (float)mouse_x / (float)r_window_width;
        normalized_mouse_y = (float)mouse_y / (float)r_window_height;
    }
    else
    {
        SDL_SetRelativeMouseMode(0);
        bm = SDL_GetMouseState(&mouse_x, &mouse_y);
        mouse_y = r_window_height - mouse_y;

        normalized_mouse_x = ((float)mouse_x / (float)r_window_width) * 2.0 - 1.0;
        normalized_mouse_y = ((float)mouse_y / (float)r_window_height) * 2.0 - 1.0;

    }

    mouse_dx = normalized_mouse_x - last_mouse_x;
    mouse_dy = normalized_mouse_y - last_mouse_y;

    last_mouse_x = normalized_mouse_x;
    last_mouse_y = normalized_mouse_y;

	/*bm = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_y = r_window_height - mouse_y - 1;


	normalized_mouse_x=(float)mouse_x/(float)(r_window_width - 1);
	normalized_mouse_y=(float)mouse_y/(float)(r_window_height - 1);

	normalized_mouse_x*=2.0;
	normalized_mouse_x-=1.0;

	normalized_mouse_y*=2.0;
	normalized_mouse_y-=1.0;

	if(engine_state == ENGINE_PLAYING)
	{

		if(last_mouse_x || last_mouse_y)
		{
			normalized_mouse_x = 0.0;
			normalized_mouse_y = 0.0;
		}

		SDL_ShowCursor(0);
		SDL_WarpMouseInWindow(window, (r_window_width )/2 - 1, (r_window_height)/2 - 1);
		mouse_dx = normalized_mouse_x;
		mouse_dy = normalized_mouse_y;

		last_mouse_x = 0.0;
		last_mouse_y = 0.0;

		printf("[%f %f] -- [%d %d] -- [%d %d]\n", mouse_dx, mouse_dy, mouse_x, mouse_y, (r_window_width) / 2 - 1, (r_window_height) / 2 - 1);
	}
	else
	{
	    SDL_SetRelativeMouseMode(0);
		SDL_ShowCursor(1);

		mouse_dx = normalized_mouse_x - last_mouse_x;
		mouse_dy = normalized_mouse_y - last_mouse_y;

		last_mouse_x = normalized_mouse_x;
		last_mouse_y = normalized_mouse_y;
	}*/








	bm_mouse &= ~(MOUSE_LEFT_BUTTON_JUST_CLICKED | MOUSE_RIGHT_BUTTON_JUST_CLICKED | MOUSE_RIGHT_BUTTON_DOUBLE_CLICKED |
				  MOUSE_LEFT_BUTTON_JUST_RELEASED | MOUSE_RIGHT_BUTTON_JUST_RELEASED | MOUSE_LEFT_BUTTON_DOUBLE_CLICKED |
				  MOUSE_MIDDLE_BUTTON_JUST_CLICKED | MOUSE_MIDDLE_BUTTON_JUST_RELEASED);

	if(mouse_left_button_timer >= 0.0)
	{
		mouse_left_button_timer += delta_time;
		if(mouse_left_button_timer >= DOUBLE_CLICK_TIME)
		{
			mouse_left_button_timer = -1.0;
			//printf("double click timeout!\n");
		}
	}

	if(bm & 1)
	{

		if(!(bm_mouse & MOUSE_LEFT_BUTTON_CLICKED))
		{
			bm_mouse |= MOUSE_LEFT_BUTTON_JUST_CLICKED;

			if(mouse_left_button_timer < 0.0)
			{
				mouse_left_button_timer = 0.0;
			}
			else if(mouse_left_button_timer < DOUBLE_CLICK_TIME)
			{
				bm_mouse |= MOUSE_LEFT_BUTTON_DOUBLE_CLICKED;
				mouse_left_button_timer = -1.0;
				//printf("double click!\n");
			}
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

vec2_t input_GetMouseDelta()
{
	return (vec2_t){mouse_dx, mouse_dy};
}

vec2_t input_GetMousePosition()
{
	return (vec2_t){(float)mouse_x, (float)mouse_y};
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

void input_EnableTextInput(int enable)
{
	b_text_input = enable;
}

void input_BufferTextInput()
{
	if(!b_text_input)
		return;

	int i;
	int c;
	int mod = 0;
	int key;

	//printf("buffer\n");

	/* flush buffer... */
	//text_buffer_out = text_buffer_in;

	text_buffer_out = 0;

	while(SDL_PollEvent(&kb_event))
	{

		if(kb_event.key.keysym.mod & KMOD_LSHIFT)
		{
			mod |= 32;
		}
		else
		{
			mod &= ~32;
		}

		if(kb_event.key.keysym.mod & KMOD_ALT)
		{
			mod |= 16;
		}
		else
		{
			mod &= ~16;
		}

		if(kb_event.key.type == SDL_KEYDOWN)
		{
			switch(kb_event.key.keysym.sym)
			{
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					continue;
				break;

				case SDLK_KP_1:
					mod = 0;
				case SDLK_1:
					if(mod & 32) key = '!';
					else key = '1';
				break;

				case SDLK_KP_2:
					mod = 0;
				case SDLK_2:
					if(mod & 32) key = '@';
					else key = '2';
				break;

				case SDLK_KP_3:
					mod = 0;
				case SDLK_3:
					if(mod & 32) key = '#';
					else key = '3';
				break;

				case SDLK_KP_4:
					mod = 0;
				case SDLK_4:
					if(mod & 32) key = '$';
					else key = '4';
				break;

				case SDLK_KP_5:
					mod = 0;
				case SDLK_5:
					if(mod & 32) key = '%';
					else key = '5';
				break;

				case SDLK_KP_6:
					mod = 0;
				case SDLK_6:
					if(mod & 32) key = '¨';
					else key = '6';
				break;

				case SDLK_KP_7:
					mod = 0;
				case SDLK_7:
					if(mod & 32) key = '&';
					else key = '7';
				break;

				case SDLK_KP_8:
					mod = 0;
				case SDLK_8:
					if(mod & 32) key = '*';
					else key = '8';
				break;

				case SDLK_KP_9:
					mod = 0;
				case SDLK_9:
					if(mod & 32) key = '(';
					else key = '9';
				break;

				case SDLK_KP_0:
					mod = 0;
				case SDLK_0:
					if(mod & 32) key = ')';
					else key = '0';
				break;


				case SDLK_KP_MINUS:
					mod = 0;
				case SDLK_MINUS:
					if(mod & 32) key = '_';
					else key = '-';
				break;

				case SDLK_KP_PLUS:
					mod = 0;
				case SDLK_PLUS:
					if(mod & 32) key = '=';
					else key = '+';
				break;

				case SDLK_KP_MULTIPLY:
					key = '*';
				break;

				case SDLK_KP_DIVIDE:
					key = '/';
				break;

				case SDLK_KP_PERIOD:
					key = '.';
				break;

				case SDLK_COMMA:
					if(mod & 32) key = '<';
					else key = ',';
				break;

				case SDLK_PERIOD:
					if(mod & 32) key = '>';
					else key = '.';
				break;

				case SDLK_SEMICOLON:
					if(mod & 32) key = ':';
					else key = ';';
				break;

				case SDLK_KP_ENTER:
					key = SDLK_RETURN;
				break;

				case SDLK_q:
					if(mod & 16) key = '/';
					else key = 'q';
				break;

				case SDLK_LALT:
				case SDLK_RALT:
					continue;
				break;

				default:
					key = kb_event.key.keysym.sym & (~mod);
				break;
			}

			if(text_buffer_out == TEXT_BUFFER_SIZE - 1)
			{
				break;
			}

			text_buffer[text_buffer_out] = key;
			text_buffer_out++;
			//text_buffer_in = (text_buffer_out + 1) % TEXT_BUFFER_SIZE;

		}

		text_buffer[text_buffer_out] = 0;
	}
}

int *input_GetTextBuffer()
{
	return text_buffer;
}

void input_ClearTextBuffer()
{
	text_buffer[0] = 0;
	text_buffer_out = 0;
}

char *input_GetKeyArray()
{
	return kb_keys;
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

int input_AnyKeyStatus()
{
    int i;

    int status = 0;

    for(i = 0; i < registered_keys_count; i++)
    {
        if(registered_keys[i].key > -1)
        {
            status |= registered_keys[i].bm_flags;
        }
    }

    return status;
}


int input_GetMouseButton(int button)
{
	//return input.bm_mouse&button;
	int i = 0;
	switch(button)
	{
		//case SDL_BUTTON_LEFT:
		case MOUSE_BUTTON_LEFT:
			i = bm_mouse & (MOUSE_LEFT_BUTTON_CLICKED | MOUSE_LEFT_BUTTON_JUST_CLICKED | MOUSE_LEFT_BUTTON_JUST_RELEASED);
		break;

		//case SDL_BUTTON_MIDDLE:
		case MOUSE_BUTTON_MIDDLE:
			i = bm_mouse & (MOUSE_MIDDLE_BUTTON_CLICKED | MOUSE_MIDDLE_BUTTON_JUST_CLICKED | MOUSE_MIDDLE_BUTTON_JUST_RELEASED);
		break;

		//case SDL_BUTTON_RIGHT:
		case MOUSE_BUTTON_RIGHT:
			i = bm_mouse & (MOUSE_RIGHT_BUTTON_CLICKED | MOUSE_RIGHT_BUTTON_JUST_CLICKED | MOUSE_RIGHT_BUTTON_JUST_RELEASED);
		break;

		case MOUSE_BUTTON_WHEEL:
			i = bm_mouse & (MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN);
		break;
	}

	return i;
}

int input_GetMouseStatus()
{
	return bm_mouse;
}

int input_RegisterKey(int key)
{
	key_t *t;
	int i;

	if(key_pos_map[key] != -1)
	{
		return -1;
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

int input_ParseFloat(float *value, char *str)
{
	int i = 0;
	int dot = -1;
	int snot = 0;
	char *s;
	int len;

	s = str;

	/* empty string... */
	if(s[0] == '\0')
		return 0;


	len = strlen(s);

	/* strip away white/new line chars... */
	while(len >= 0 && (s[len] == ' ' || s[len] == '\n')) len--;

	/* string formed by only spaces... */
	if(!len)
		return 0;

	i = 0;
	/* strip away white/spaces at the beginning... */
	while(s[i] == ' ') i++;

	while(s[i] != '\0' && i < len)
	{
		if(s[i] > '9' || s[i] < '0')
		{
			switch(s[i])
			{
				case '.':
					if(dot != -1)
					{
						return 0;
					}

					dot = i++;

					if(s[i] > '9' || s[i] < '0')
					{
						return 0;
					}

				break;

				case 'e':
				case 'E':
					if(snot || !i)
					{
						return 0;
					}

					snot = i++;
				break;

				case '-':
				case '+':

					if(i)
					{
						if(s[i - 1] != 'e' || s[i - 1] != 'E')
						{
							return 0;
						}
					}
				break;

				default:
					return 0;
				break;
			}
		}

		i++;
	}

	*value = atof(s);

	return 1;

}

int input_ParseInt(int *value, char *str)
{
	int i = 0;
	int hex = 0;
	int oct = 0;
	char *s;
	int len;

	s = str;

	/* empty string... */
	if(s[0] == '\0')
		return 0;

	len = strlen(s);

	/* strip away white/new line chars at the end... */
	while(len >= 0 && (s[len] == ' ' || s[len] == '\n')) len--;

	/* string formed by only spaces... */
	if(!len)
		return 0;

	i = 0;
	/* strip away white/spaces at the beginning... */
	while(s[i] == ' ') i++;

	while(s[i] != '\0' && i < len)
	{
		//if(s[i] > '9' || s[i] < '0')
		//{
			switch(s[i])
			{
				/* hex prefix is already handled by another case,
				so if we end up here, it means it's duplicated... */
				case 'x':
				case 'X':
					return 0;
				break;

				case '0':

					i++;

					if(s[i] == 'x' || s[i] == 'X')
					{
						hex = 1;
					}
					else
					{
						oct = 1;
					}

				break;

				case 'a': case 'A':
				case 'b': case 'B':
				case 'c': case 'C':
				case 'd': case 'D':
				case 'e': case 'E':
				case 'f': case 'F':
					if(!hex)
					{
						return 0;
					}
				break;

				/* we can have an arbitrary number of minuses and
				pluses in the beginning of the number, but none
				in between... */
				case '-':
				case '+':

					/* we're past the first char... */
					if(i)
					{
						/* if the prev char is not a sign, something
						is wrong with this constant... */
						if(s[i - 1] != '-' || s[i - 1] != '+')
						{
							return 0;
						}
					}
				break;

				case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':

				break;

				default:
					return 0;
				break;

			}
		//}


		i++;
	}

	*value = atoi(s);

	return 1;
}

int input_ParseShort(short *value, char *str)
{

}

int input_ParseVec3(vec3_t *value, char *str)
{

}


#ifdef __cplusplus
}
#endif





