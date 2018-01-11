#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sound.h"

#include "al.h"
#include "alc.h"
#include "efx.h"
#include "camera.h"

#include "SDL2\SDL.h"


#define MAX_SOUND_SOURCES 128
#define MAX_SOUND_COMMANDS 512
#define MAX_SOUND_PARAM_BUFFERS 512

ALCdevice *sound_device;
ALCcontext *sound_context;
SDL_Thread *sound_thread;


static int sound_list_size;
static int sound_count;
static int sound_free_position_stack_top;
static int *sound_free_position_stack;
static sound_t *sounds;



//static int max_sound_sources;
static int sound_source_free_stack_top;
static int *sound_source_free_stack;
static int active_sound_sources_count;
static int *active_sound_sources;
static sound_source_t *sound_sources;


SDL_mutex *sound_commands_mutex;
//static int max_sound_commands;
static int last_in;
static int last_out;
static sound_command_t *sound_commands;


//static int max_sound_param_buffers;
static int sound_param_free_stack_top;
static int *sound_param_free_stack;
static int last_used_sound_param_buffer;
static int last_released_sound_param_buffer;
static sound_param_buffer_t *sound_param_buffers;

void sound_Init()
{
	sound_device = alcOpenDevice(NULL);
	int i;
	
	int context_params[3] = {ALC_MONO_SOURCES, 256, 0};
	
	if(!sound_device)
	{
		printf("Oh shit...\n");
		exit(4);
	}
	
	sound_context = alcCreateContext(sound_device, context_params);
	alcMakeContextCurrent(sound_context);
	sound_thread = SDL_CreateThread(sound_SoundThread, "sound thread", NULL); 
	
	
	sound_list_size = 64;
	sound_count = 0;
	sound_free_position_stack_top = -1;
	sound_free_position_stack = malloc(sizeof(int) * sound_list_size);
	sounds = malloc(sizeof(sound_t) * sound_list_size);
	
	//max_sound_commands = MAX_SOUND_COMMANDS;
	last_in = 0;
	last_out = 0;
	sound_commands = malloc(sizeof(sound_command_t) * MAX_SOUND_COMMANDS);
	sound_commands_mutex = SDL_CreateMutex();
	
	//max_
	//sound_param_free_stack_top = -1;
	last_used_sound_param_buffer = 0;
	last_released_sound_param_buffer = 0;
	//sound_param_free_stack = malloc(sizeof(int) * MAX_SOUND_PARAM_BUFFERS);
	sound_param_buffers = malloc(sizeof(sound_param_buffer_t) * MAX_SOUND_PARAM_BUFFERS);
	
	//max_sound_sources = MAX_SOUND_SOURCES;
	sound_source_free_stack_top = -1;
	sound_source_free_stack = malloc(sizeof(int) * MAX_SOUND_SOURCES);
	active_sound_sources = malloc(sizeof(int) * MAX_SOUND_SOURCES);
	sound_sources = malloc(sizeof(sound_source_t) * MAX_SOUND_SOURCES);
	
	for(i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		alGenSources(1, &sound_sources[i].source_handle);
		sound_source_free_stack_top++;
		sound_source_free_stack[sound_source_free_stack_top] = i;
	}
	
	
	
}

void sound_Finish()
{
	int i;
	alcDestroyContext(sound_context);
	alcCloseDevice(sound_device);
	
	for(i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		alDeleteSources(1, &sound_sources[i].source_handle);
	}
	
	for(i = 0; i < sound_count; i++)
	{
		free(sounds[i].name);
		free(sounds[i].data);
		alDeleteBuffers(1, &sounds[i].al_buffer_handle);
	}
	
	free(sound_free_position_stack);
	free(sound_source_free_stack);
	//free(sound_param_free_stack);
	
	
	free(sounds);
	free(sound_commands);
	free(sound_param_buffers);
	free(active_sound_sources);
	
	SDL_DestroyMutex(sound_commands_mutex);
	
	/* FIX ME: the sound thread does not
	check to see if it should stop. Waiting
	for it here will hang engine... */
	
	//SDL_WaitThread(sound_thread, NULL);
}

int sound_LoadSound(char *file_name, char *name)
{
	sound_t *sound;
	sound = sound_LoadWAV(file_name);
	if(sound)
	{
		sound->name = strdup(name);
		return sound->sound_index;
	}
	
	return -1;
}

sound_t *sound_LoadWAV(char *file_name)
{
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
	
}

int sound_PlaySound(int sound_index, vec3_t position, float gain)
{
	int source_index;
	int sound_param_buffer_index;
	int sound_command_index;
	
	if(sound_source_free_stack_top >= 0)
	{
		SDL_LockMutex(sound_commands_mutex);
		
		
		//sound_command_index = (last_in + 1) % MAX_SOUND_COMMANDS;
		
		
		/* all buffers are in use... */
		
		if(last_in == last_out - 1)
		{
			SDL_UnlockMutex(sound_commands_mutex);
			return -1;
		}
		
		
		/*if(last_used_sound_param_buffer == last_released_sound_param_buffer - 1)
		{
			SDL_UnlockMutex(sound_commands_mutex);
			return -1;
		} */
		
		//printf("%d %d\n", last_used_sound_param_buffer, last_released_sound_param_buffer);
		
		source_index = sound_source_free_stack[sound_source_free_stack_top--];
		
		sound_param_buffer_index = last_used_sound_param_buffer;
		
		last_used_sound_param_buffer = (last_used_sound_param_buffer + 1) % MAX_SOUND_PARAM_BUFFERS;
				
		sound_command_index = last_in;
		sound_commands[sound_command_index].command_id = SOUND_COMMAND_START_SOUND;
		sound_commands[sound_command_index].param_buffer = sound_param_buffer_index;
		
		sound_param_buffers[sound_param_buffer_index].sound_source_index = source_index;
		sound_param_buffers[sound_param_buffer_index].position = position;
		sound_param_buffers[sound_param_buffer_index].al_buffer_handle = sounds[sound_index].al_buffer_handle;
		sound_param_buffers[sound_param_buffer_index].gain = gain;
		
		active_sound_sources[active_sound_sources_count] = source_index;
		active_sound_sources_count++;
		
		//printf("%d\n", source_index);
		
		//last_in++;
		//if(last_in + 1 >= MAX_SOUND_SOURCES) last_in = 0;
		/*last_in++;
		last_in %= MAX_SOUND_SOURCES;*/
		
		
		/* This could cause a loss in performance by
		an RFO request... */
		last_in = (last_in + 1) % MAX_SOUND_SOURCES;
		
		SDL_UnlockMutex(sound_commands_mutex);
		
		return source_index;
		
	}
	
	return -1;
}

void sound_ProcessSound()
{
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
	int r;
	while(1)
	{
		//printf("sound thread\n");
		SDL_Delay(17);				/* 58.82 Hz is good enough for this... */
		
		while(last_out != last_in)
		{
			switch(sound_commands[last_out].command_id)
			{
				case SOUND_COMMAND_START_SOUND:
					
					sound_param_buffer = sound_commands[last_out].param_buffer;
					
					source_index = sound_param_buffers[sound_param_buffer].sound_source_index;
					source_position = sound_param_buffers[sound_param_buffer].position;
					source_handle = sound_sources[source_index].source_handle;
					
					//last_released_sound_param_buffer++;
					
					
					
					last_released_sound_param_buffer = (last_released_sound_param_buffer + 1) % MAX_SOUND_PARAM_BUFFERS;
					
					alSourcei(source_handle, AL_BUFFER, sound_param_buffers[sound_param_buffer].al_buffer_handle);
					alSourcef(source_handle, AL_GAIN, sound_param_buffers[sound_param_buffer].gain);
					alSource3f(source_handle, AL_POSITION, source_position.x, source_position.y, source_position.z);
					alSourcePlay(source_handle);
					sound_sources[source_index].bm_status |= SOURCE_ASSIGNED | SOURCE_PLAYING;
					
					last_out = (last_out + 1) % MAX_SOUND_SOURCES;
					
					//printf("%d %d %d\n", source_index, last_in, last_out);

				break;
				
				case SOUND_COMMAND_PAUSE_SOUND:
					last_out = (last_out + 1) % MAX_SOUND_SOURCES;
				break;
				
				case SOUND_COMMAND_STOP_SOUND:
					last_out = (last_out + 1) % MAX_SOUND_SOURCES;
				break;
			}
		}
		//else
		//{
		for(r = 0; r < active_sound_sources_count; r++)
		{
			source_index = active_sound_sources[r];
			source_handle = sound_sources[source_index].source_handle; 
			alGetSourcei(source_handle, AL_SOURCE_STATE, &source_state);
				
				/* this source is done playing, so free it... */
			if(source_state == AL_STOPPED)
			{
				sound_sources[source_index].bm_status &= ~SOURCE_ASSIGNED;
				sound_source_free_stack_top++;
				sound_source_free_stack[sound_source_free_stack_top] = source_index;
					
				//printf("%d %d\n", source_inde);
					
					/* if there's more than one source in the list
					(and this source is not the last), pull the last
					index to this position and decrement the counter,
					making the loop process the newly replaced index... */
				if(r < active_sound_sources_count - 1)
				{
					active_sound_sources[r] = active_sound_sources[active_sound_sources_count - 1];
					r--;
				}
					
				active_sound_sources_count--;
			}
		}
		//}
		
		
	}
}










