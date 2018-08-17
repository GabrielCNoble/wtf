#include "scr_sound.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct sound_handle_t sound_ScriptGetSound(struct script_string_t *name)
{
    char *sound_name = script_string_GetRawString(name);
    return sound_GetSound(sound_name);
}

void sound_ScriptPlaySound(struct sound_handle_t sound, vec3_t *position, float gain, int loop)
{
	sound_PlaySound(sound, *position, gain, loop);
}

void sound_ScriptPauseAllSounds()
{

}

void sound_ScriptResumeAllSounds()
{

}

#ifdef __cplusplus
}
#endif

