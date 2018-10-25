#include "snd_script.h"


#include "path.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct sound_handle_t sound_ScriptGetSound(struct script_string_t *name)
{
    char *sound_name = script_string_GetRawString(name);
    return sound_GetSound(sound_name);
}

int sound_ScriptPlaySound(struct sound_handle_t sound, vec3_t *position, float gain, int flags)
{
	return sound_PlaySound(sound, *position, gain, flags);
}

void sound_ScriptPauseSound(int sound_source, int fade_out)
{
	sound_PauseSound(sound_source, fade_out);
}

void sound_ScriptStopSound(int sound_source, int fade_out)
{
	sound_StopSound(sound_source, fade_out);
}

int sound_ScriptIsSourcePlaying(int sound_source)
{
	return sound_IsSourcePlaying(sound_source);
}

int sound_ScriptIsSourceAssigned(int sound_source)
{
    return sound_IsSourceAssigned(sound_source);
}

void sound_ScriptSetSourcePosition(int sound_source, vec3_t *position)
{
    sound_SetSourcePosition(sound_source, *position);
}



void sound_ScriptLoadSound(struct script_string_t *name)
{
    char *sound_name = script_string_GetRawString(name);
    sound_LoadSound(sound_name, path_GetNameNoExt(sound_name));
}

void sound_ScriptPauseAllSounds()
{

}

void sound_ScriptResumeAllSounds()
{

}
