#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpk_file.h"
#include "mpk_write.h"

#include "gmath/vector.h"

int main(int argc, char *argv[]) 
{
	char output_name[PATH_MAX] = "a.mpk";
	char input_file[PATH_MAX] = "";
	int i = 0;
	
	struct output_params_t parms;
	
	if(argc > 1)
	{
		for(i = 1; i < argc; i++)
		{
			switch(argv[i][0])
			{
				case '-':
					if(argv[i][1] == 'o')
					{
						i++;
						strcpy(output_name, argv[i]);
					}
					else if(argv[i][1] == 'f')
					{
						i++;
						strcpy(input_file, argv[i]);
					}
				break;	
			}
		}
		mpk_convert(output_name, input_file);
		
		mpk_read(output_name, &parms);
	}
	
	return 0;
}
