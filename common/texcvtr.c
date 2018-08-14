#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "tex_ptx.h"


int main(int argc, char **argv) 
{
	int i;
	
	int input_files_count = 0;;
	
	char **input_files;
	char *arg;
	char output_name[PATH_MAX];
	
	input_files = malloc(sizeof(char *) * 65536);
	
	int expect_file = 0;
	
	if(argc > 1)
	{
		for(i = 1; i < argc; i++)
		{
			arg = argv[i];
			
			if(arg[0] == '-')
			{
				if(expect_file)
				{
					goto _nope;
				}
				
				if(arg[1] == 'f')
				{
					expect_file = 1;
				}
				else if(arg[1] == 'o')
				{
					expect_file = 2;
				}
				else
				{
					goto _nope;
				}
			}
			else if(expect_file)
			{
				switch(expect_file)
				{
					case 1:
						input_files[input_files_count] = strdup(arg);
						input_files_count++;
					break;
					
					case 2:
						strcpy(output_name, arg);
					break;
				}
				
				expect_file = 0;
			}
		}
		
		ptx_write(output_name, input_files, input_files_count);
		
	}
	else
	{
		_nope:
			
		printf("***nope***\n");
	}
	
	free(input_files);
	
	
	
	return 0;
}
