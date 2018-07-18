#ifndef SCR_ARRAY_H
#define SCR_ARRAY_H

struct script_array_t
{
	int element_size;
	int element_count;
	void *type_info;
	void *buffer;
};


struct script_array_t *script_array_Constructor(void *type_info);

struct script_array_t *script_array_Constructor_Sized(void *type_info, int size);

void script_array_Destructor(void *this_pointer);

void script_array_AddRef(void *this_pointer);

void script_array_Release(void *this_pointer);

void *script_array_ElementAt(void *this_pointer, int index);

int script_array_Count(void *this_pointer);


#endif
