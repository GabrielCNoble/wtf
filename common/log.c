#include "log.h"


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

char log_path[256];

FILE *log_file;


#ifdef __cplusplus
extern "C"
{
#endif

void log_Init()
{
	log_file = fopen("massacre.log", "w");
}



void log_Finish()
{
	fflush(log_file);
	fclose(log_file);
}

  
void log_LogMessage(int message_type, char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	switch(message_type)
	{
		case LOG_MESSAGE_NOTIFY:
			fputs("NOTIFY: ", log_file);
		break;
		
		case LOG_MESSAGE_WARNING:
			fputs("WARNING: ", log_file);
		break;
		
		case LOG_MESSAGE_ERROR:
			fputs("ERROR: ", log_file);
		break;
	}
	vfprintf(log_file, format, args);
	fputs("\n", log_file);
}


void log_FlushLog()
{
	fflush(log_file);
	fclose(log_file);
	log_file = fopen("massacre.log", "a");
}

#ifdef __cplusplus
}
#endif















