#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sound.h"

#include "al.h"
#include "alc.h"
#include "efx.h"
#include "camera.h"
#include "c_memory.h"
#include "containers/stack_list.h"
#include "containers/stack.h"

#include "SDL2\SDL.h"

#include "ogg/ogg.h"


#define MAX_SOUND_SOURCES 128
#define MAX_SOUNDS 512
#define MAX_SOUND_COMMANDS 512
#define MAX_SOUND_PARAM_BUFFERS 512

ALCdevice *sound_device;
ALCcontext *sound_context;
SDL_Thread *sound_thread;


//static int sound_list_size;
//static int sound_count;
//static int sound_free_position_stack_top;
//static int *sound_free_position_stack;
//static sound_t *sounds;



//static int max_sound_sources;
//static int sound_source_free_stack_top;
//static int *sound_source_free_stack;
//static int active_sound_sources_count;
//static int *active_sound_sources;
//static sound_source_t *sound_sources;


SDL_mutex *snd_sound_commands_mutex;
//static int max_sound_commands;
//static int last_in;
//static int last_out;
//static sound_command_t *sound_commands;


//static int max_sound_param_buffers;
//static int sound_param_free_stack_top;
//static int *sound_param_free_stack;
//static int last_used_sound_param_buffer;
//static int last_released_sound_param_buffer;
//static sound_param_buffer_t *sound_param_buffers;




//struct stack_list_t snd_sound_sources;
//struct stack_t snd_sound_sources_stack;


int snd_next_in_sound_command = 0;
int snd_next_out_sound_command = 0;
struct sound_command_t *snd_sound_commands = NULL;

int snd_next_in_sound_source = 0;
int snd_next_out_sound_source = 0;
/* ring buffer with indices into the sound source buffer... */
int *snd_sound_sources_indices = NULL;

int snd_active_sound_sources_count = 0;
/* indices of the active sources... */
int *snd_active_sound_sources_indices = NULL;


struct sound_source_t *snd_sound_sources;




//int *snd_sound_sources_stack = NULL;
//int snd_sound_sources_stack_top = -1;
struct stack_list_t snd_sounds;



#ifdef __cplusplus
extern "C"
{
#endif

int sound_Init()
{
	sound_device = alcOpenDevice(NULL);
	int i;

	int source_handle;

	struct sound_source_t sound_source;

	int context_params[3] = {ALC_STEREO_SOURCES, 256, 0};

	if(!sound_device)
	{
		printf("Oh shit...\n");
		exit(4);
	}

	sound_context = alcCreateContext(sound_device, context_params);
	alcMakeContextCurrent(sound_context);
	sound_thread = SDL_CreateThread(sound_SoundThread, "sound thread", NULL);




	//sound_list_size = 64;
	//sound_count = 0;
	//sound_free_position_stack_top = -1;
	//sound_free_position_stack = malloc(sizeof(int) * sound_list_size);
	//sounds = malloc(sizeof(sound_t) * sound_list_size);

	//max_sound_commands = MAX_SOUND_COMMANDS;
//	last_in = 0;
//	last_out = 0;
//	sound_commands = malloc(sizeof(sound_command_t) * MAX_SOUND_COMMANDS);
//	sound_commands_mutex = SDL_CreateMutex();

	//max_
	//sound_param_free_stack_top = -1;
//	last_used_sound_param_buffer = 0;
//	last_released_sound_param_buffer = 0;
	//sound_param_free_stack = malloc(sizeof(int) * MAX_SOUND_PARAM_BUFFERS);
//	sound_param_buffers = malloc(sizeof(sound_param_buffer_t) * MAX_SOUND_PARAM_BUFFERS);

    //snd_sound_commands_mutex = SDL_CreateMutex();


    //snd_sound_commands = memory_Calloc(sizeof(struct sound_command_t) * MAX_SOUND_COMMANDS);


	snd_active_sound_sources_indices = memory_Calloc(sizeof(int), MAX_SOUND_SOURCES);
	snd_sound_sources_indices = memory_Calloc(sizeof(int), MAX_SOUND_SOURCES);
	snd_sound_sources = memory_Calloc(sizeof(struct sound_source_t), MAX_SOUND_SOURCES);
	snd_sound_commands = memory_Calloc(sizeof(struct sound_command_t), MAX_SOUND_COMMANDS);

	snd_sounds = stack_list_create(sizeof(struct sound_t), MAX_SOUNDS, NULL);

	for(i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		alGenSources(1, &snd_sound_sources[i].source_handle);
		snd_sound_sources_indices[i] = i;
	}


	return 1;

}

void sound_Finish()
{
	int i;
	alcDestroyContext(sound_context);
	alcCloseDevice(sound_device);

	for(i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		alDeleteSources(1, &snd_sound_sources[i].source_handle);
	}

	/*for(i = 0; i < sound_count; i++)
	{
		free(sounds[i].name);
		free(sounds[i].data);
		alDeleteBuffers(1, &sounds[i].al_buffer_handle);
	}*/

	//free(sound_free_position_stack);
	//free(sound_source_free_stack);
	//free(sound_param_free_stack);


	//free(sounds);
	//free(sound_commands);
	//free(sound_param_buffers);
	//free(active_sound_sources);

	stack_list_destroy(&snd_sounds);
	memory_Free(snd_sound_commands);
	memory_Free(snd_sound_sources);

//	memory_Free(snd_sound_sources_stack);
	//stack_list_destroy(&snd_sound_sources);

	//SDL_DestroyMutex(sound_commands_mutex);

	/* FIX ME: the sound thread does not
	check to see if it should stop. Waiting
	for it here will hang engine... */

	//SDL_WaitThread(sound_thread, NULL);
}

struct sound_handle_t sound_CreateEmptySound(char *name)
{
    struct sound_t *sound;
    struct sound_handle_t handle;

    handle.sound_index = stack_list_add(&snd_sounds, NULL);

    sound = stack_list_get(&snd_sounds, handle.sound_index);
    memset(sound, 0, sizeof(struct sound_t));

    sound->name = memory_Strdup(name);

    return handle;
}

void sound_SetSoundData(struct sound_handle_t sound, void *data, int size, int format, int frequency)
{
    struct sound_t *sound_ptr;

    int data_size;

    sound_ptr = sound_GetSoundPointer(sound);


    if(sound_ptr)
    {
        switch(format)
        {
            case AL_FORMAT_MONO8:
            case AL_FORMAT_STEREO8:
                data_size = sizeof(unsigned char);
            break;

            case AL_FORMAT_MONO16:
            case AL_FORMAT_STEREO16:
                data_size = sizeof(unsigned short);
            break;

            default:
                printf("sound_SetSoundData: bad sound format for sound [%s]\n", sound_ptr->name);
                return;

        }

        if(!sound_ptr->al_buffer_handle)
        {
            alGenBuffers(1, &sound_ptr->al_buffer_handle);
        }

        alBufferData(sound_ptr->al_buffer_handle, format, data, data_size * size, frequency);
    }
}

struct sound_handle_t sound_LoadSound(char *file_name, char *name)
{
	/*struct sound_t *sound;
	sound = sound_LoadWAV(file_name);

	if(sound)
	{
		sound->name = strdup(name);
		return sound->sound_index;
	}

	return -1;*/
}

struct sound_handle_t sound_GenerateWhiteNoise(char *name, float lenght)
{
    unsigned short *sound_data;
    struct sound_handle_t sound;
    int i;

    int data_frequency = 48000;
    int data_lenght = data_frequency * lenght;

    sound_data = memory_Calloc(data_lenght, sizeof(unsigned short));

    for(i = 0; i < data_lenght; i++)
    {
        sound_data[i] = 0x7fff * (((float)rand() / (float)RAND_MAX) * 2.0 - 1.0);
    }

    sound = sound_CreateEmptySound(name);
    sound_SetSoundData(sound, sound_data, data_lenght, AL_FORMAT_MONO16, data_frequency);

    return sound;
}

struct sound_handle_t sound_GenerateSineWave(char *name, float length, float frequency)
{
    unsigned short *sound_data;
    struct sound_handle_t sound;

    float angle_step;
    float cur_angle;

    int i;

    double c;

    int data_frequency = 48000;
    int data_lenght = data_frequency * length;

    sound_data = memory_Calloc(data_lenght, sizeof(unsigned short));

    /* this gives us how much to step in the unity cycle in
    each sample to fulfill the desired amount of oscillations
    after a second... */
    angle_step = frequency / (float) data_frequency;

    cur_angle = 0.0;

    for(i = 0; i < data_lenght; i++)
    {
        sound_data[i] = 0x7fff * sin(3.14159265 * 2.0 * cur_angle);

        /* must keep cur_angle within [0.0, 1.0] to preserve
        precision... */
        cur_angle = modf(cur_angle + angle_step, &c);
    }

    sound = sound_CreateEmptySound(name);
    sound_SetSoundData(sound, sound_data, data_lenght, AL_FORMAT_MONO16, data_frequency);

    return sound;
}

struct sound_t *sound_LoadWAV(char *file_name)
{

	#if 0
	FILE *f;
	unsigned long long file_size;
	char *file_data;
	int *sound_data;
	short compression;
	short channels;
	short bits_per_sample;
	int sample_rate;
	int data_size;
	int i;
	int c;

	int sound_index;
	sound_t *sound = NULL;

	if(!(f = fopen(file_name, "rb")))
	{
		printf("couldn't open %s!\n", file_name);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	rewind(f);

	//fseek(f, 20, SEEK_CUR);


	/**((char *)&compression)=fgetc(f);	//any compression?
	*((char *)&compression+1)=fgetc(f);

	*((char *)&channels)=fgetc(f);		//how many channels.
	*((char *)&channels+1)=fgetc(f);

	*((char *)&sample_rate)=fgetc(f);	//sample rate
	*((char *)&sample_rate+1)=fgetc(f);
	*((char *)&sample_rate+2)=fgetc(f);
	*((char *)&sample_rate+3)=fgetc(f);

	fseek(f, 6, SEEK_CUR);
	*((char *)&bits_per_sample)=fgetc(f);
	*((char *)&bits_per_sample+1)=fgetc(f);

	fseek(f, 4, SEEK_CUR);
	*((char *)&data_size)=fgetc(f);
	*((char *)&data_size+1)=fgetc(f);
	*((char *)&data_size+2)=fgetc(f);
	*((char *)&data_size+3)=fgetc(f);*/


	file_data = malloc(file_size);
	fread(file_data, 1, file_size, f);
	fclose(f);

	c = 20;
	compression = *(short *)(&file_data[c]);
	c += 2;
	channels = *(short *)(&file_data[c]);
	c += 2;
	sample_rate = *(int *)(&file_data[c]);
	c += 10;
	bits_per_sample = *(short *)(&file_data[c]);
	c += 6;
	data_size = *(int *)(&file_data[c]);
	c += 4;
	sound_data = malloc(data_size);

	for(i = 0; i < data_size >> 2; i++)
	{
		sound_data[i] = *(int *)&file_data[c + i * 4];
	}

	free(file_data);


	if(sound_free_position_stack_top >= 0)
	{
		sound_index = sound_free_position_stack[sound_free_position_stack_top--];
	}
	else
	{
		sound_index = sound_count++;

		if(sound_index >= sound_list_size)
		{
			free(sound_free_position_stack);

			sound_free_position_stack = malloc(sizeof(int) * (sound_list_size + 16));
			sound = malloc(sizeof(sound_t) * (sound_list_size + 16));

			memcpy(sound, sounds, sizeof(sound_t) * sound_list_size);
			free(sounds);
			sounds = sound;
			sound_list_size += 16;
		}
	}

	sound = &sounds[sound_index];


	switch(channels)
	{
		case 1:
			if(bits_per_sample == 1)
			{
				sound->format = AL_FORMAT_MONO8;
			}
			else
			{
				sound->format = AL_FORMAT_MONO16;
			}
		break;

		case 2:
			if(bits_per_sample == 1)
			{
				sound->format = AL_FORMAT_STEREO8;
			}
			else
			{
				sound->format = AL_FORMAT_STEREO16;
			}
		break;
	}

	sound->bits_per_sample = bits_per_sample;
	sound->sample_rate = sample_rate;
	sound->size = data_size;
	sound->data = sound_data;
	sound->sound_index = sound_index;
	alGenBuffers(1, &sound->al_buffer_handle);
	alBufferData(sound->al_buffer_handle, sound->format, sound->data, sound->size, sound->sample_rate);

	return sound;

	#endif

}


struct sound_t *sound_LoadOGG(char *file_name)
{

}

struct sound_t *sound_GetSoundPointer(struct sound_handle_t sound)
{
	struct sound_t *sound_ptr = NULL;

    if(sound.sound_index != INVALID_SOUND_INDEX)
	{
        sound_ptr = stack_list_get(&snd_sounds, sound.sound_index);
	}

	return sound_ptr;
};

int sound_PlaySound(struct sound_handle_t sound, vec3_t position, float gain)
{
	int source_index = -1;
	int stack_index;
	int sound_param_buffer_index;
	int command_index = -1;

	struct sound_t *sound_ptr;

	struct sound_command_t *sound_command;
	struct sound_source_t *sound_source;

	/* first we get the position inside the indice ring buffer... */
	source_index = snd_next_in_sound_source;

    if(snd_next_in_sound_source == snd_next_out_sound_source - 1)
    {
        return -1;
    }

    snd_next_in_sound_source = (snd_next_in_sound_source + 1) % MAX_SOUND_SOURCES;

    command_index = snd_next_in_sound_command;

    if(snd_next_in_sound_command == snd_next_out_sound_command - 1)
	{
		return -1;
	}

	snd_next_in_sound_command = (snd_next_in_sound_command + 1) % MAX_SOUND_COMMANDS;

	/* then we get the actual source indice from the ring buffer... */
	source_index = snd_sound_sources_indices[source_index];

    sound_command = &snd_sound_commands[command_index];

	sound_ptr = sound_GetSoundPointer(sound);

    sound_command->cmd_id = SOUND_COMMAND_TYPE_START_SOUND;
    sound_command->source = source_index;
    sound_command->params.position = position;
    sound_command->params.gain = gain;
    sound_command->params.al_sound_buffer = sound_ptr->al_buffer_handle;

    //snd_next_in_sound_source = source_index;
    //snd_next_in_sound_command = command_index;

	return source_index;

	return -1;
}

void sound_ProcessSound()
{

	#if 0
//	mat3_t orientation;
	camera_t *active_camera = camera_GetActiveCamera();

	//orientation = active_camera->world_orientation;

	vec3_t orientation[2];
	orientation[0] = active_camera->world_orientation.f_axis;

	orientation[0].x = -orientation[0].x;
	orientation[0].y = -orientation[0].y;
	orientation[0].z = -orientation[0].z;

	orientation[1] = active_camera->world_orientation.u_axis;



	//mat3_t_transpose(&orientation);

	alListener3f(AL_POSITION, active_camera->world_position.x, active_camera->world_position.y, active_camera->world_position.z);
	alListenerfv(AL_ORIENTATION, &orientation[0].floats[0]);

	#endif

}

void sound_SuspendSoundBackend()
{

}

void sound_ResumeSoundBackend()
{

}


int sound_SoundThread(void *param)
{
	vec3_t source_position;
	unsigned int source_handle;
	unsigned int source_index;
	unsigned int source_state;
	unsigned int sound_param_buffer;

	struct sound_command_t *sound_command;
	struct sound_source_t *sound_source;

	int r;
	while(1)
	{
		//printf("sound thread\n");
		SDL_Delay(17);				/* 58.82 Hz is good enough for this... */

		while(snd_next_out_sound_command != snd_next_in_sound_command)
		{
			sound_command = &snd_sound_commands[snd_next_out_sound_command];
			sound_source = &snd_sound_sources[sound_command->source];

			switch(sound_command->cmd_id)
			{
				case SOUND_COMMAND_TYPE_START_SOUND:
                    sound_source->params = sound_command->params;

                    /* we only need the al handle to check the source status... */
                    snd_active_sound_sources_indices[snd_active_sound_sources_count] = sound_command->source;
                    snd_active_sound_sources_count++;

                    sound_source->flags = SOURCE_FLAG_PLAYING;

                    alSourcei(sound_source->source_handle, AL_BUFFER, sound_source->params.al_sound_buffer);
                    alSourcef(sound_source->source_handle, AL_GAIN, sound_source->params.gain);
                    alSource3f(sound_source->source_handle, AL_POSITION, sound_source->params.position.x, sound_source->params.position.y, sound_source->params.position.z);
					alSourcePlay(sound_source->source_handle);

				break;

				case SOUND_COMMAND_TYPE_STOP_SOUND:
					alSourceStop(sound_source->source_handle);
				break;
			}

			snd_next_out_sound_command = (snd_next_out_sound_command + 1) % MAX_SOUND_COMMANDS;
		}

		for(r = 0; r < snd_active_sound_sources_count; r++)
		{
			source_index = snd_active_sound_sources_indices[r];
			sound_source = &snd_sound_sources[source_index];
			source_handle = sound_source->source_handle;

			alGetSourcei(source_handle, AL_SOURCE_STATE, &source_state);


			if(source_state == AL_STOPPED)
			{
				/* this source is done playing, so free it... */
				sound_source->flags &= ~SOURCE_FLAG_PLAYING;

				if(r < snd_active_sound_sources_count - 1)
				{
					snd_active_sound_sources_indices[r] = snd_active_sound_sources_indices[snd_active_sound_sources_count - 1];
					r--;
				}

				snd_active_sound_sources_count--;

				/* add its indice back into the ring buffer... */
				snd_sound_sources_indices[snd_next_out_sound_source] = source_index;

				/*if(snd_next_out_sound_source == snd_next_in_sound_source - 1)
				{
					continue;
				}*/

				snd_next_out_sound_source = (snd_next_out_sound_source + 1) % MAX_SOUND_SOURCES;
			}

		}
	}
}


#ifdef __cplusplus
}
#endif







