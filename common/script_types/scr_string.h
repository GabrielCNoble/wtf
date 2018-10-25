#ifndef SCR_STRING_H
#define SCR_STRING_H

struct script_string_t;

#ifdef __cplusplus
extern "C"
{
#endif

char *script_string_GetRawString(struct script_string_t *script_string);

void script_string_TestPrint(struct script_string_t *script_string);


#ifdef __cplusplus
}
#endif



#endif
