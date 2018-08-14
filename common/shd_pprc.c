#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shd_pprc.h"
#include "c_memory.h"
#include "path.h"



struct shader_define *global_defines = NULL;
struct shader_define *shader_defines = NULL;


//struct shader_conditional *conditionals = NULL;


void shader_Preprocess(char **text)
{
	int text_cursor;
	char *text_str;

	struct shader_conditional *conditionals;

	text_str = *text;
	text_cursor = 0;

	while(text_str[text_cursor] != '\0')
	{
		switch(text_str[text_cursor])
		{
			case '#':
				//text_cursor++;
				if(text_str[text_cursor + 1] == 'i' &&
				   text_str[text_cursor + 2] == 'n' &&
				   text_str[text_cursor + 3] == 'c' &&
				   text_str[text_cursor + 4] == 'l' &&
				   text_str[text_cursor + 5] == 'u' &&
				   text_str[text_cursor + 6] == 'd' &&
				   text_str[text_cursor + 7] == 'e' &&
				   text_str[text_cursor + 8] == ' ')
				{
					//text_cursor += 7;
					shader_Include(&text_str, text_cursor);
					break;
				}
				else if(text_str[text_cursor + 1] == 'd' &&
						text_str[text_cursor + 2] == 'e' &&
						text_str[text_cursor + 3] == 'f' &&
						text_str[text_cursor + 4] == 'i' &&
						text_str[text_cursor + 5] == 'n' &&
						text_str[text_cursor + 6] == 'e' &&
						text_str[text_cursor + 7] == ' ')
				{
					//text_cursor += 6;
					shader_Define(&text_str, &text_cursor);
					break;
				}
				else if(text_str[text_cursor + 1] == 'i' &&
						text_str[text_cursor + 2] == 'f')
				{
					shader_Conditional(&text_str, &text_cursor, &conditionals);
					shader_ResolveConditionals(&text_str, &text_cursor, conditionals);
					break;
				}

				goto _inc_counter;

			break;

			default:
				_inc_counter:
				text_cursor++;
			break;

			case '/':
				text_cursor++;
				if(text_str[text_cursor] == '/')
				{
					text_cursor++;
					while(text_str[text_cursor] != '\0' && text_str[text_cursor] != '\n') text_cursor++;
				}
				else if(text_str[text_cursor] == '*')
				{
					text_cursor++;
					while(text_str[text_cursor] != '\0')
					{
						if(text_str[text_cursor] == '*')
						{
							if(text_str[text_cursor + 1] == '/')
							{
								text_cursor += 2;
								break;
							}
						}

						text_cursor++;
					}
				}
			break;
		}
	}

	shader_ClearDefines(0);

//	printf("%s\n", text_str);

	*text = text_str;
}

/*
===================================================
===================================================
===================================================
*/

void shader_AddDefine(char *name, char *value, int global)
{
	struct shader_define *define;


	define = memory_Malloc(sizeof(struct shader_define));

	define->name = memory_Strdup(name);
	define->value = NULL;

	if(value)
	{
		if(value[0] != '\0')
		{
			define->value = memory_Strdup(value);
		}
	}

	if(global)
	{
		define->next = global_defines;
		global_defines = define;
	}
	else
	{
		define->next = shader_defines;
		shader_defines = define;
	}


}

void shader_DropDefine(char *name, int global)
{
	struct shader_define *define = NULL;
	struct shader_define *prev = NULL;

	if(global)
	{
		define = global_defines;
	}
	else
	{
		define = shader_defines;
	}


	while(define)
	{
		if(!strcmp(name, define->name))
		{
			if(prev)
			{
				prev->next = define->next;
			}
			else
			{
				if(global)
				{
					global_defines = global_defines->next;
				}
				else
				{
					shader_defines = shader_defines->next;
				}

			}

			memory_Free(define->name);

			if(define->value)
			{
				memory_Free(define->value);
			}

			memory_Free(define);

			return;
		}

		prev = define;
		define = define->next;
	}
}

struct shader_define *shader_GetDefine(char *name)
{
	struct shader_define *define;

	define = shader_defines;

	while(define)
	{
		if(!strcmp(define->name, name))
		{
			break;
		}

		define = define->next;
	}

	define = global_defines;

	while(define)
	{
		if(!strcmp(define->name, name))
		{
			break;
		}

		define = define->next;
	}


	return define;
}

void shader_ClearDefines(int global)
{
	struct shader_define *next;
	if(global)
	{
		while(global_defines)
		{
			next = global_defines->next;

			memory_Free(global_defines->name);

			if(global_defines->value)
			{
				memory_Free(global_defines->value);
			}

			memory_Free(global_defines);

			global_defines = next;
		}
	}
	else
	{
		while(shader_defines)
		{
			next = shader_defines->next;

			memory_Free(shader_defines->name);

			if(shader_defines->value)
			{
				memory_Free(shader_defines->value);
			}

			memory_Free(shader_defines);

			shader_defines = next;
		}
	}

}


/*
===================================================
===================================================
===================================================
*/

int shader_Conditional(char **text, int *cursor, struct shader_conditional **conditionals)
{
	char *s;
	int i;
	int start;
	int end;
	int type;
	int cur_level = -1;


	struct shader_conditional *cond = NULL;
	struct shader_conditional *conds = NULL;
	struct shader_conditional *last_cond = NULL;

	int value_str_cursor;
	char value_str[512];


	s = *text;
	i = *cursor;

	//while(s[i] != '\0')
	do
	{
		/* skip any white spaces... */
		while(s[i] == ' ' || s[i] == '\n' || s[i] == 'r') i++;

		if(s[i    ] == '#' &&
		   s[i + 1] == 'i' &&
		   s[i + 2] == 'f')
		{
			start = i;
			i += 3;

			if(s[i	  ] == 'n' &&
			   s[i + 1] == 'd' &&
			   s[i + 2] == 'e' &&
			   s[i + 3] == 'f' &&
			   s[i + 4] == ' ')
			{
				i += 5;
				type = SHADER_CONDITIONAL_IFNDEF;
			}

			else if(s[i    ] == 'd' &&
			        s[i + 1] == 'e' &&
			   	 	s[i + 2] == 'f' &&
			   	 	s[i + 3] == ' ')
			{
				i += 4;
				type = SHADER_CONDITIONAL_IFDEF;
			}

			else
			{
				type = SHADER_CONDITIONAL_IF;
			}
		}
		else if(s[i    ] == '#' &&
				s[i + 1] == 'e')
		{
			start = i;
			i += 2;

			if(s[i    ] == 'l')
			{
				i++;

				if(s[i    ] == 'i' &&
				   s[i + 1] == 'f')
				{
					i += 2;
					type = SHADER_CONDITIONAL_ELIF;
				}

				else if(s[i    ] == 's' &&
					    s[i + 1] == 'e')
				{
					i += 2;
					type = SHADER_CONDITIONAL_ELSE;
				}

				else
				{
					type = SHADER_CONDITIONAL_UNKNOWN;
				}
			}

			else if(s[i    ] == 'n' &&
					s[i + 1] == 'd' &&
					s[i + 2] == 'i' &&
					s[i + 3] == 'f')
			{
				i += 4;
				end = i;
				type = SHADER_CONDITIONAL_ENDIF;
			}

			else
			{
				//goto _nope;
				continue;
				type = SHADER_CONDITIONAL_UNKNOWN;
			}
		}
		else if(s[i] == '/')
		{
			i++;

			if(s[i] == '/')
			{
				i++;
				while(s[i] != '\n' && s[i] != '\0') i++;
				continue;
				//goto _nope;
			}
			else if(s[i] == '*')
			{
				i++;

				while(s[i] != '\0')
				{
					if(s[i] == '*')
					{
						if(s[i + 1] == '/')
						{
							i += 2;
							break;
						}
					}
					i++;
				}

				//goto _nope;
				continue;
			}
			else
			{
				i++;
				continue;
			}

		}
		else
		{
			//_nope:
			i++;
			continue;
		}



		/* skip white spaces after directive... */
		while(s[i] == ' ' && s[i] != '\n' && s[i] != '\0' && s[i] != '\r')i++;

		value_str_cursor = 0;


		/* get whatever is after this directive, but avoid other directives...*/
		while(s[i] != '\n' && s[i] != '\r' && s[i] != '\0' && s[i] != '#')
		{
			value_str[value_str_cursor] = s[i];
			value_str_cursor++;
			i++;
		}

		value_str[value_str_cursor] = '\0';




		switch(type)
		{
			case SHADER_CONDITIONAL_IF:
				if(!value_str_cursor)
				{
					printf("shader_Conditional: #if with no expression\n");
					return 0;
				}
				cur_level++;
			break;

			case SHADER_CONDITIONAL_ELIF:
				if(!value_str_cursor)
				{
					printf("shader_Conditional: #elif with no expression\n");
					return 0;
				}
			break;

			case SHADER_CONDITIONAL_ELSE:
				if(value_str_cursor)
				{
					printf("shader_Conditional: extra tokens after #else directive\n");
				}
			break;

			case SHADER_CONDITIONAL_ENDIF:
				if(value_str_cursor)
				{
					printf("shader_Conditional: extra tokens after #endif directive\n");
				}
				cur_level--;

				if(cur_level == 0)
				{
					continue;
				}
			break;

			case SHADER_CONDITIONAL_IFDEF:
				if(!value_str_cursor)
				{
					printf("shader_Conditional: no macro given in #ifdef directive\n");
					return 0;
				}
				cur_level++;
			break;

			case SHADER_CONDITIONAL_IFNDEF:
				if(!value_str_cursor)
				{
					printf("shader_Conditional: no macro given in #ifndef directive\n");
					return 0;
				}
				cur_level++;
			break;

			case SHADER_CONDITIONAL_UNKNOWN:

			break;

		}

		if(cur_level < 1)
		{
			cond = memory_Malloc(sizeof(struct shader_conditional));
			cond->next = NULL;
			cond->type = type;
			cond->start = start;
			cond->end = end;
			cond->value = NULL;

			if(value_str_cursor)
			{
				cond->value = memory_Strdup(value_str);
			}

			if(!conds)
			{
				conds = cond;
			}
			else
			{
				last_cond->next = cond;
			}

			last_cond = cond;
		}


	}while(s[i] != '\0' && cur_level > -1);

	cond = conds;

	while(cond)
	{
		if(cond->next)
		{
			cond->end = cond->next->start;
		}

		cond = cond->next;
	}


	*conditionals = conds;

	return 1;
}

void shader_ResolveConditionals(char **text, int *cursor, struct shader_conditional *conditionals)
{
	struct shader_conditional *cond = NULL;
	struct shader_conditional *next = NULL;
	struct shader_conditional *nextnext = NULL;
	struct shader_define *define = NULL;

	int passed = 0;
	struct shader_conditional *passed_cond = NULL;

	cond = conditionals;


	while(cond && (!passed))
	{
		switch(cond->type)
		{
			case SHADER_CONDITIONAL_IF:

			break;

			case SHADER_CONDITIONAL_ELIF:

			break;

			case SHADER_CONDITIONAL_ELSE:
				passed = 1;
				passed_cond = cond;
			break;

			case SHADER_CONDITIONAL_ENDIF:

			break;

			case SHADER_CONDITIONAL_IFDEF:
			case SHADER_CONDITIONAL_IFNDEF:
				define = shader_GetDefine(cond->value);

				if(cond->type == SHADER_CONDITIONAL_IFDEF)
				{
					if(!define)
					{
						cond = cond->next;
						continue;
					}
				}
				else
				{
					if(define)
					{
						cond = cond->next;
						continue;
					}
				}


				passed = 1;
				passed_cond = cond;
			break;
		}

		cond = cond->next;
	}

	cond = conditionals;

	while(cond)
	{
		next = cond->next;

		if(cond == passed_cond)
		{
			shader_DeleteConditional(text, cursor, cond, 1);
		}
		else
		{
			shader_DeleteConditional(text, cursor, cond, 0);
		}

		cond = next;
	}
}


void shader_ResolveConditionalExp(char *exp)
{

}

void shader_DeleteConditional(char **text, int *cursor, struct shader_conditional *conditional, int conditional_text_only)
{
	int i = 0;
	char *s = *text;

	if(conditional_text_only)
	{
		for(i = conditional->start; s[i] != '\n' && s[i] != '\0' && s[i] != '\r'; i++)
		{
			s[i] = ' ';
		}
	}
	else
	{
		for(i = conditional->start; i < conditional->end; i++)
		{
			s[i] = ' ';
		}
	}

	if(conditional->value)
	{
		memory_Free(conditional->value);
	}
	memory_Free(conditional);
}


/*
===================================================
===================================================
===================================================
*/


int shader_Include(char **text, int cursor)
{
	int old_text_include_end;
	int old_text_include_start;
	int old_text_restart;

	int k;
	//int j;
	char *old_text;
	int file_name_cursor;
	char file_name[PATH_MAX];
	char *file_path;
	unsigned long include_file_size;
	char *include_text;
	int new_text_len;
	char *new_text;

	FILE *file = NULL;

	old_text = *text;
	old_text_include_end = cursor;

	/* skip the whole #define directive... */
	while(old_text[old_text_include_end] != ' ' &&
		  old_text[old_text_include_end] != '\0' &&
		  old_text[old_text_include_end] != '\n' &&
		  old_text[old_text_include_end] != '\r')
	{
		old_text_include_end++;
	}

	/* skip any white spaces before the file name... */
	while(old_text[old_text_include_end] == ' ' &&
		  old_text[old_text_include_end] != '\0' &&
		  old_text[old_text_include_end] != '\n' &&
		  old_text[old_text_include_end] != '\r')
	{
	  	old_text_include_end++;
	}

	if(old_text[old_text_include_end] == '"')
	{
		old_text_include_end++;

		file_name_cursor = 0;

		while(old_text[old_text_include_end] != '"' && old_text[old_text_include_end] != '\0' && old_text[old_text_include_end] != '\n')
		{
			file_name[file_name_cursor] = old_text[old_text_include_end];
			file_name_cursor++;
			old_text_include_end++;
		}

		file_name[file_name_cursor] = '\0';

		old_text_include_end++;

		file = fopen(file_name, "rb");

		if(!file)
		{
			file_path = path_GetPathToFile(file_name);

			if(!file_name)
			{
				printf("shader_Include: couldn't find file %s!\n", file_name);
				return 0;
			}

			file = fopen(file_path, "rb");
		}

		fseek(file, 0, SEEK_END);
		include_file_size = ftell(file);
		rewind(file);

		include_text = memory_Malloc(include_file_size + 1);
		fread(include_text, include_file_size, 1, file);
		fclose(file);

		include_text[include_file_size] = '\0';

		new_text_len = strlen(*text) + 1 + include_file_size;

		new_text = memory_Malloc(new_text_len);


		/*for(old_text_include_start = old_text_include_end; old_text_include_start >= 0; old_text_include_start--)
		{
			if(old_text[old_text_include_start] == '#')
			{
				break;
			}
		}

		*cursor = old_text_include_start;*/

		old_text_include_start = cursor;

		/* copy everything before the directive... */
		for(k = 0; k < old_text_include_start; k++)
		{
			new_text[k] = old_text[k];
		}

		/* copy the included text... */
		for(old_text_restart = 0; old_text_restart < include_file_size; old_text_restart++)
		{
			new_text[old_text_include_start + old_text_restart] = include_text[old_text_restart];
		}

		/* skip the whole #include directive and copy the rest of the old text... */
		for(k = 0; old_text[old_text_include_end + k] != '\0'; k++)
		{
			new_text[old_text_include_start + old_text_restart + k] = old_text[old_text_include_end + k];
		}

		new_text[old_text_include_start + old_text_restart + k] = '\0';

		//printf("%s\n", new_text);

		memory_Free(old_text);
		memory_Free(include_text);

		*text = new_text;

		return 1;
	}
	else
	{
		/* something wrong went on... */
	}

}


int shader_Define(char **text, int *cursor)
{
	int i;
	char *str;
	int str_cursor;
	char define_name[512];
	char define_value[512];
	struct shader_define *define;

	str = *text;
	i = *cursor;

	/* skip the #define directive... */
	while(str[i] != ' ' && str[i] != '\0' && str[i] != '\n' && str[i] != '\r')i++;

	/* skip any white spaces before whatever is being defined... */
	while(str[i] == ' ' && str[i] != '\0') i++;

	str_cursor = 0;

	while(str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
	{
		define_name[str_cursor] = str[i];
		str_cursor++;
		i++;
	}

	define_name[str_cursor] = '\0';

	define_value[0] = '\0';

	while(str[i] == ' ' && str[i] != '\0' && str[i] != '\n') i++;

	if(str[i] != '\0' && str[i] != '\n')
	{
		str_cursor = 0;

		while(str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
		{
			define_value[str_cursor] = str[i];
			str_cursor++;
			i++;
		}

		define_value[str_cursor] = '\0';
	}

	define = shader_GetDefine(define_name);

	if(define)
	{
		printf("shader_Define: %s redefined!\n", define_name);

		if(define->value)
		{
			memory_Free(define->value);
			define->value = NULL;
		}

		if(define_value[0] != '\0')
		{
			define->value = memory_Strdup(define_value);
		}
	}
	else
	{
		shader_AddDefine(define_name, define_value, 0);
	}

	*cursor = i;

}













