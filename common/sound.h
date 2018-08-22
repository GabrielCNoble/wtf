#ifndef SOUND_H
#define SOUND_H

#include "vector_types.h"

enum SOURCE_FLAGS
{
	SOURCE_FLAG_FADING_IN = 1,
	SOURCE_FLAG_FADING_OUT = 1 << 1,
	SOURCE_FLAG_PLAYING = 1 << 2,
	SOURCE_FLAG_PAUSED = 1 << 3,
	SOURCE_FLAG_STOPPED = 1 << 4,
	SOURCE_FLAG_JUST_RESUMED = 1 << 5,
	SOURCE_FLAG_JUST_PAUSED = 1 << 6,
	SOURCE_FLAG_JUST_STOPPED = 1 << 7,

	SOURCE_FLAG_START_SOUND = 1 << 8,
	SOURCE_FLAG_PAUSE_SOUND = 1 << 9,
	SOURCE_FLAG_STOP_SOUND = 1 << 10,

	SOURCE_FLAG_ASSIGNED = 1 << 11,
	SOURCE_FLAG_LOOP = 1 << 12,
};


enum SOUND_COMMAND_TYPE
{
	SOUND_COMMAND_TYPE_START_SOUND = 1,
	SOUND_COMMAND_TYPE_PAUSE_SOUND,
	SOUND_COMMAND_TYPE_RESUME_SOUND,
	SOUND_COMMAND_TYPE_STOP_SOUND,

	SOUND_COMMAND_TYPE_RESUME_ALL_SOUNDS,
	SOUND_COMMAND_TYPE_PAUSE_ALL_SOUNDS,
	SOUND_COMMAND_TYPE_STOP_ALL_SOUNDS,

	SOUND_COMMAND_TYPE_STOP_BACKEND,

	SOUND_COMMAND_TYPE_LAST,
	SOUND_COMMAND_TYPE_NONE,
};

enum SOUND_FLAGS
{
	SOUND_FLAG_INVALID = 1,
};

struct sound_t
{
	float duration;
	unsigned short format;
	unsigned short sample_rate;
	unsigned short bits_per_sample;
	unsigned short flags;
	unsigned int size;
	unsigned int al_buffer_handle;

	void *data;
	char *name;
};


struct sound_handle_t
{
	unsigned int sound_index;
};

#define INVALID_SOUND_INDEX 0xffffffff
#define INVALID_SOUND_HANDLE (struct sound_handle_t){0xffffffff}

struct sound_source_params_t
{
    vec3_t position;
    unsigned int al_sound_buffer;
    //struct sound_handle_t sound;
    float gain;
};


struct sound_source_t
{
	unsigned int source_handle;
	unsigned int flags;
	struct sound_source_params_t params;
};


struct sound_command_t
{
    int source;
    int cmd_id;
    struct sound_source_params_t params;
};

/*typedef union
{
	struct
	{
		vec3_t position;
		unsigned int al_buffer_handle;
		unsigned int sound_source_index;
		float gain;
	};
}sound_param_buffer_t;*/


#ifdef __cplusplus
extern "C"
{
#endif

int sound_Init();

void sound_Finish();

struct sound_handle_t sound_CreateEmptySound(char *name);

void sound_SetSoundData(struct sound_handle_t sound, void *data, int size, int format, int frequency);

struct sound_handle_t sound_LoadSound(char *file_name, char *name);

struct sound_handle_t sound_GenerateWhiteNoise(char *name, float length);

struct sound_handle_t sound_GenerateSineWave(char *name, float length, float frequency);

struct sound_handle_t sound_LoadWAV(char *file_name);

struct sound_handle_t sound_LoadOGG(char *file_name);

struct sound_handle_t sound_GetSound(char *name);

struct sound_t *sound_GetSoundPointer(struct sound_handle_t sound);

struct sound_source_t *sound_GetSoundSourcePointer(int sound_source);


/* those are for internal use only. Calling them is likely to
cause problems... */
int sound_IsSoundSourceAvailable();

int sound_IsSoundCommandAvailable();

int sound_AllocSoundSource();

int sound_EmitSoundCommand(int type, int source);




int sound_PlaySound(struct sound_handle_t sound, vec3_t position, float gain, int loop);

void sound_PauseSound(int sound_source);

void sound_ResumeSound(int sound_source);

void sound_StopSound(int sound_source);

int sound_IsSourcePlaying(int sound_source);

void sound_ResumeAllSounds();

void sound_PauseAllSounds();

void sound_StopAllSounds();






void sound_ProcessSound();





#ifdef __cplusplus
}
#endif




#endif
