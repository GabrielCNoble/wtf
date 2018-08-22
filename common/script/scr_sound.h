#ifndef SCR_SOUND_H
#define SCR_SOUND_H

#include "sound.h"
#include "script_types/scr_string.h"


#ifdef __cplusplus
extern "C"
{
#endif

struct sound_handle_t sound_ScriptGetSound(struct script_string_t *name);

int sound_ScriptPlaySound(struct sound_handle_t sound, vec3_t *position, float gain, int loop);

void sound_ScriptPauseSound(int sound_source);

void sound_ScriptStopSound(int sound_source);

int sound_ScriptIsSourcePlaying(int sound_source);



void sound_ScriptLoadSound(struct script_string_t *name);

void sound_ScriptPauseAllSounds();

void sound_ScriptResumeAllSounds();

#ifdef __cplusplus
}
#endif


#endif // SCR_SOUND_H
