#ifndef SOUND_H
#define SOUND_H

#include "vector_types.h"

enum SOURCE_FLAGS
{
	SOURCE_FADING_IN = 1,
	SOURCE_FADING_OUT = 1 << 1,
	SOURCE_PLAYING = 1 << 2,
	SOURCE_PAUSED = 1 << 3,
	SOURCE_STOPPED = 1 << 4,
	SOURCE_JUST_RESUMED = 1 << 5,
	SOURCE_JUST_PAUSED = 1 << 6,
	SOURCE_JUST_STOPPED = 1 << 7,
	SOURCE_ASSIGNED = 1 << 8,
};


enum SOUND_COMMAND_TYPE
{
	SOUND_COMMAND_START_SOUND = 1,
	SOUND_COMMAND_PAUSE_SOUND,
	SOUND_COMMAND_STOP_SOUND,
};

typedef struct
{
	float duration;
	unsigned short format;
	unsigned short sample_rate;
	unsigned short bits_per_sample;
	unsigned short align2;
	unsigned int size;
	unsigned int al_buffer_handle;
	unsigned int sound_index;
	void *data;
	char *name;
}sound_t;


typedef struct
{
	//vec3_t position;
	//vec3_t velocity;
	unsigned int source_handle;
	short bm_status;
}sound_source_t;


typedef struct
{
	int command_id;
	int param_buffer;
}sound_command_t;

typedef union
{
	struct
	{
		vec3_t position;
		unsigned int al_buffer_handle;
		unsigned int sound_source_index;
		float gain;
	};
}sound_param_buffer_t;


#ifdef __cplusplus
extern "C"
{
#endif

int sound_Init();

void sound_Finish();

int sound_LoadSound(char *file_name, char *name);

sound_t *sound_LoadWAV(char *file_name);

sound_t *sound_LoadVorbis(char *file_name);

int sound_PlaySound(int sound_index, vec3_t position, float gain);

void sound_ProcessSound();

void sound_SuspendSoundBackend();

void sound_ResumeSoundBackend();

int sound_SoundThread(void *param);


#ifdef __cplusplus
}
#endif




#endif
