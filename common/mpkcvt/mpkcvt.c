#include <stdio.h>
#include <string.h>

#include "..\mpk_file.h"
#include "..\mpk_write.h"
#include "..\c_memory.h"



int main(int argc, char *argv[])
{
    int i;

    int input_state = 0;

    char *input_file;
    char *output_file;
    char output_path[512];

    if(argc > 1)
    {
        for(i = 1; i < argc; i++)
        {
            if(argv[i][0] == '-')
            {
                switch(argv[i][1])
                {
                    case 'f':
                        input_state = 1;
                    break;

                    case 'o':
                        input_state = 2;
                    break;

                    default:
                        return 0;

                }

                continue;
            }

            switch(input_state)
            {
                case 1:
                    input_file = argv[i];
                break;

                case 2:
                    output_file = argv[i];
                break;
            }


            input_state = 0;
        }


        if(input_file && output_file)
        {
            strcpy(output_path, input_file);

            i = 0;

            while(output_path[i])
            {
                if(output_path[i] == '\\')
                {
                    output_path[i] = '/';
                }
                i++;
            }

            while(output_path[i] != '/')
            {
                output_path[i] = '\0';
                i--;
            }

            i = 0;

            while(output_file[i]) i++;
            while(output_file[i] != '\\' && output_file[i] != '/' && i) i--;

            if(output_file[i] == '\\' || output_file[i] == '/')
            {
                i++;
            }

            strcat(output_path, output_file + i);

            output_file = output_path;

            i = 0;

            while(output_file[i]) i++;
            while(i && output_file[i] != '.') i--;

            if(strcmp(output_file + i, ".mpk"))
            {
                if(i)
                {
                    output_file[i] = '\0';
                }

                strcat(output_file, ".mpk");
            }

       //     printf("before memory_Init\n");
     //       memory_Init(0);
        //    printf("after memory_Init\n");

            mpk_write(output_file, input_file);

        //    memory_Finish();


        }
    }


}







