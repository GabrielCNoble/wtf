#ifndef SHD_PPRC_H
#define SHD_PPRC_H


#include "shd_common.h"

struct shader_define
{
	struct shader_define *next;
	char *name;
	char *value;
};

enum SHADER_CONDITIONAL_TYPES
{
	SHADER_CONDITIONAL_IF,
	SHADER_CONDITIONAL_ELIF,
	SHADER_CONDITIONAL_ELSE,
	SHADER_CONDITIONAL_ENDIF,
	SHADER_CONDITIONAL_IFDEF,
	SHADER_CONDITIONAL_IFNDEF,
	SHADER_CONDITIONAL_UNKNOWN,
};

struct shader_conditional
{
	int type;
	char *value;

	int start;
	int end;

	struct shader_conditional *nestled;
	struct shader_conditional *last_nestled;

	struct shader_conditional *parent;

	struct shader_conditional *next;
};



void shader_Preprocess(char **text);

/*
===================================================
===================================================
===================================================
*/



void shader_AddDefine(char *name, char *value, int global);

void shader_DropDefine(char *name, int global);

struct shader_define *shader_GetDefine(char *name);

void shader_ClearDefines(int global);


/*
===================================================
===================================================
===================================================
*/

int shader_Conditional(char **text, int *cursor, struct shader_conditional **conditionals);

void shader_ResolveConditionals(char **text, int *cursor, struct shader_conditional *conditionals);

void shader_ResolveConditionalExp(char *exp);

void shader_DeleteConditional(char **text, int *cursor, struct shader_conditional *conditional, int conditional_text_only);

/*
===================================================
===================================================
===================================================
*/

int shader_Include(char **text, int *cursor);

int shader_Define(char **text, int *cursor);

/*
===================================================
===================================================
===================================================
*/

void shader_ClearText(char **text, int start, int end);



#endif






