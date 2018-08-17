#ifndef SCR_SOUND_H
#define SCR_SOUND_H

#include "sound.h"
#include "script_types/scr_string.h"


#ifdef __cplusplus
extern "C"
{
#endif

struct sound_handle_t sound_ScriptGetSound(struct script_string_t *name);

void sound_ScriptPlaySound(struct sound_handle_t sound, vec3_t *position, float gain, int loop);

void sound_ScriptPauseAllSounds();

void sound_ScriptResumeAllSounds();

#ifdef __cplusplus
}
#endif


#endif // SCR_SOUND_H
