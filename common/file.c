#include "file.h"

#include <stdio.h>
#include <Windows.h>




int file_FileExists(char *file_name)
{
	FILE *file;
	file = fopen(file_name, "rb");
	
	if(file)
	{
		fclose(file);
		return 1;
	}
	
	return 0;
}

void file_CreateDirectory(char *name)
{
	
}

void file_ChangeDirectory(char *name)
{
	
} 
