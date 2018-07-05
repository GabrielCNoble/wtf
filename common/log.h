#ifndef LOG_H
#define LOG_H


enum LOG_MESSAGE_TYPE
{
	LOG_MESSAGE_NONE,
	LOG_MESSAGE_NOTIFY,
	LOG_MESSAGE_WARNING,
	LOG_MESSAGE_ERROR,
};


#ifdef __cplusplus
extern "C"
{
#endif

void log_Init();

void log_Finish();

void log_LogMessage(int message_type, char *format, ...);

void log_FlushLog();


#ifdef __cplusplus
}
#endif


#endif
