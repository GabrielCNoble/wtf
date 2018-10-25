#include "scr_string.h"
#include <string>
#include <stdio.h>

struct script_string_t
{
	std::string script_string;
};

#ifdef __cplusplus
extern "C"
{
#endif

char *script_string_GetRawString(struct script_string_t *script_string)
{
	return (char *)script_string->script_string.c_str();
}

void script_string_TestPrint(struct script_string_t *script_string)
{
	printf(script_string->script_string.c_str());
}

#ifdef __cplusplus
}
#endif
