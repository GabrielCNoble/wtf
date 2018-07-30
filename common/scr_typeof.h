#ifndef SCR_TYPEOF_H
#define SCR_TYPEOF_H




#ifdef __cplusplus
extern "C"
{
#endif

void *script_TypeofConstructor(void *type_info);

void *script_TypeofDestructor(void *type_info);

void *script_TestTypeof(void *type_info);


#ifdef __cplusplus
}
#endif



#endif
