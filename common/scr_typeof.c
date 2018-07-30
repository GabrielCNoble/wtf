#include "scr_typeof.h"
#include "angelscript.h"
#include "script.h"

#include <stdio.h>


#ifdef __cplusplus
extern "C"
{
#endif

void *script_TypeofConstructor(void *type_info)
{
	return type_info;
}

void *script_TypeofDestructor(void *type_info)
{
	
}

void *script_TestTypeof(void *type_info)
{
	asITypeInfo *tinfo;
	asITypeInfo *stinfo;
	
	tinfo = (asITypeInfo *)type_info;
	stinfo = tinfo->GetSubType();
	
	
	int size = script_GetTypeSize(type_info);
	
	
	if(!stinfo)
	{
		printf("type is: %s\nsize is: %d\n", tinfo->GetName(), size);
	}
	else
	{
		printf("type is: %s\nsize is %d\n", stinfo->GetName(), size);
	}
	
}

#ifdef __cplusplus
}
#endif




