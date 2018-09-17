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
	log_file = fopen("engine.log", "w");
	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "LOG START");
}



void log_Finish()
{
	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "LOG END");
	fflush(log_file);
	fclose(log_file);
}


void log_LogMessage(int message_type, int echo, char *format, ...)
{
	va_list args;
	va_start(args, format);

	static char message_buffer[8192];
	static char temp_message_buffer[8192];
	char *message_type_str;

	switch(message_type)
	{
		case LOG_MESSAGE_NOTIFY:
			message_type_str = "NOTIFY: ";
		break;

		case LOG_MESSAGE_WARNING:
			message_type_str = "WARNING: ";
		break;

		case LOG_MESSAGE_ERROR:
			message_type_str = "ERROR: ";
		break;

		case LOG_MESSAGE_NONE:
			message_type_str = "";
		break;
	}

	strcpy(message_buffer, message_type_str);
	vsprintf(temp_message_buffer, format, args);

	strcat(message_buffer, temp_message_buffer);
	strcat(message_buffer, "\n");

	fputs(message_buffer, log_file);

	if(echo)
	{
		printf("%s", message_buffer);
	}
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















