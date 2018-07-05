#ifndef ED_EVENT_H
#define ED_EVENT_H


#ifdef __cplusplus
extern "C"
{
#endif

void editor_EventEditorInit();

void editor_EventEditorFinish();

void editor_EventEditorSetup();

void editor_EventEditorShutdown();

void editor_EventEditorRestart();

void editor_EventEditorMain(float delta_time);

#ifdef __cplusplus
}
#endif



#endif
