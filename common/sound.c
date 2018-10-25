#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sound.h"

#include "script.h"

#include "snd_script.h"

#include "r_common.h"
#include "r_main.h"

#include "al.h"
#include "alc.h"
#include "efx.h"
#include "camera.h"
#include "c_memory.h"
#include "containers/stack_list.h"
#include "containers/stack.h"
#include "path.h"
#include "log.h"

#include "SDL2\SDL.h"

//#include "ogg/ogg.h"

#include "vorbisfile.h"


#define MAX_SOUND_SOURCES 256
#define MAX_SOUNDS 512
#define MAX_SOUND_COMMANDS 512
#define MAX_SOUND_PARAM_BUFFERS 512

ALCdevice *sound_device;
ALCcontext *sound_context;
SDL_Thread *snd_sound_thread;


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


unsigned int snd_next_in_sound_command = 0;
unsigned int snd_next_out_sound_command = 0;
struct sound_command_t *snd_sound_commands = NULL;

unsigned int snd_next_in_sound_source = 0;
unsigned int snd_next_out_sound_source = 0;
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

int sound_SoundBackend(void *param);





int sound_Init()
{
	sound_device = alcOpenDevice(NULL);
	int i;

	int source_handle;

	struct sound_source_t sound_source;

	int context_params[3] = {ALC_STEREO_SOURCES, 256, 0};

	if(!sound_device)
	{
		log_LogMessage(LOG_MESSAGE_ERROR, 1, "%s: OpenAL didn't initalize!",__func__);
		return 0;
	}

	sound_context = alcCreateContext(sound_device, context_params);
	alcMakeContextCurrent(sound_context);
	snd_sound_thread = SDL_CreateThread(sound_SoundBackend, "sound backend", NULL);

	snd_active_sound_sources_indices = memory_Calloc(sizeof(int), MAX_SOUND_SOURCES);
	snd_sound_sources_indices = memory_Calloc(sizeof(int), MAX_SOUND_SOURCES);
	snd_sound_sources = memory_Calloc(sizeof(struct sound_source_t), MAX_SOUND_SOURCES);
	snd_sound_commands = memory_Calloc(sizeof(struct sound_command_t), MAX_SOUND_COMMANDS);

	snd_sounds = stack_list_create(sizeof(struct sound_t), MAX_SOUNDS, NULL);

	for(i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		alGenSources(1, &snd_sound_sources[i].source_handle);
		alSourcei(snd_sound_sources[i].source_handle, AL_MAX_GAIN, 50.0);
		snd_sound_sources_indices[i] = i;
	}

    script_RegisterEnum("SOURCE_FLAGS");
	script_RegisterEnumValue("SOURCE_FLAGS", "SOURCE_FLAG_LOOP", SOURCE_FLAG_LOOP);
	script_RegisterEnumValue("SOURCE_FLAGS", "SOURCE_FLAG_RELATIVE", SOURCE_FLAG_RELATIVE);
	script_RegisterEnumValue("SOURCE_FLAGS", "SOURCE_FLAG_FADE_IN", SOURCE_FLAG_FADE_IN);
	script_RegisterEnumValue("SOURCE_FLAGS", "SOURCE_FLAG_FADE_OUT", SOURCE_FLAG_FADE_OUT);


	script_RegisterObjectType("sound_handle_t", sizeof(struct sound_handle_t), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
	script_RegisterGlobalFunction("sound_handle_t sound_GetSound(string &in name)", sound_ScriptGetSound);
	script_RegisterGlobalFunction("int sound_PlaySound(sound_handle_t sound, vec3_t &in position, float gain, int flags)", sound_ScriptPlaySound);
	script_RegisterGlobalFunction("void sound_PauseSound(int sound_source, int fade_out)", sound_ScriptPauseSound);
	script_RegisterGlobalFunction("void sound_StopSound(int sound_source, int fade_out)", sound_ScriptStopSound);
    script_RegisterGlobalFunction("int sound_IsSourcePlaying(int sound_source)", sound_ScriptIsSourcePlaying);
    script_RegisterGlobalFunction("int sound_IsSourceAssigned(int sound_source)", sound_ScriptIsSourceAssigned);
    script_RegisterGlobalFunction("void sound_SetSourcePosition(int sound_source, vec3_t &in position)", sound_ScriptSetSourcePosition);




	log_LogMessage(LOG_MESSAGE_NOTIFY, 0, "%s: subsystem initialized properly!", __func__);


	return 1;

}

void sound_Finish()
{
	int i;

	//sound_EmitSoundCommand(SOUND_COMMAND_TYPE_STOP_BACKEND, -1);

	//SDL_WaitThread(snd_sound_thread, NULL);

	/*alcDestroyContext(sound_context);
	alcCloseDevice(sound_device);

	for(i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		alDeleteSources(1, &snd_sound_sources[i].source_handle);
	}

	stack_list_destroy(&snd_sounds);
	memory_Free(snd_sound_commands);
	memory_Free(snd_sound_sources);*/
}

struct sound_handle_t sound_CreateEmptySound(char *name)
{
    struct sound_t *sound;
    struct sound_handle_t handle;

    handle.sound_index = stack_list_add(&snd_sounds, NULL);

    sound = stack_list_get(&snd_sounds, handle.sound_index);
    memset(sound, 0, sizeof(struct sound_t));

	if(name)
	{
		sound->name = memory_Strdup(name);
	}

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
                data_size = sizeof(char);
            break;

            case AL_FORMAT_MONO16:
            case AL_FORMAT_STEREO16:
                data_size = sizeof(short);
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
    char *file_ext;
	struct sound_handle_t sound = INVALID_SOUND_HANDLE;
	struct sound_t *sound_ptr;

    file_ext = path_GetFileExtension(file_name);

	char *file_path;

	struct sound_handle_t (*loader)(char *file_name) = NULL;

    if(!strcmp(file_ext, "ogg"))
	{
		loader = sound_LoadOGG;
	}
	else if(!strcmp(file_ext, "wav"))
	{
		loader = sound_LoadWAV;
	}

	sound = sound_GetSound(name);

	if(sound.sound_index != INVALID_SOUND_INDEX)
	{
        printf("sound_LoadSound: sound [%s] already exists!\n", name);
		return sound;
	}


	if(loader)
	{
		file_path = path_GetPathToFile(file_name);

		if(file_path)
		{
			sound = loader(file_path);

			sound_ptr = sound_GetSoundPointer(sound);

			if(sound_ptr)
			{
				sound_ptr->name = memory_Strdup(name);
			}
		}
		else
		{
			printf("sound_LoadSound: couldn't find file [%s]\n", file_name);
		}
	}
	else
	{
		printf("sound_LoadSound: bad sound format for file [%s]\n", file_name);
	}


	return sound;
}

void sound_DestroySoundHandle(struct sound_handle_t sound)
{
	struct sound_t *sound_ptr;

	sound_ptr = sound_GetSoundPointer(sound);

	if(sound_ptr)
	{
        sound_ptr->flags |= SOUND_FLAG_INVALID;
		memory_Free(sound_ptr->data);
		alDeleteBuffers(1, &sound_ptr->al_buffer_handle);

		stack_list_remove(&snd_sounds, sound.sound_index);
	}
}

void sound_DestroySound(char *name)
{
	struct sound_handle_t sound;
	sound = sound_GetSound(name);
	sound_DestroySoundHandle(sound);
}

struct sound_handle_t sound_GenerateWhiteNoise(char *name, float lenght)
{
    short *sound_data;
    struct sound_handle_t sound;
    int i;

    int data_frequency = 48000;
    int data_lenght = data_frequency * lenght;

    sound_data = memory_Calloc(data_lenght, sizeof(short));

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
    short *sound_data;
    struct sound_handle_t sound;

    float angle_step;
    float cur_angle;

    int i;

    double c;

    int data_frequency = 48000;
    int data_lenght = data_frequency * length;

    sound_data = memory_Calloc(data_lenght, sizeof(short));

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




struct wav_riff_chunk_t
{
    unsigned int chunk_id;
    unsigned int chunk_size;
    unsigned int format;
};


struct wav_fmt_chunk_t
{
	unsigned int sub_chunk_id;
	unsigned int sub_chunk_size;
	unsigned short audio_format;
	unsigned short channel_count;
	unsigned int sample_rate;
	unsigned int byte_rate;
	unsigned short block_align;
	unsigned short bits_per_sample;
};

struct wav_data_chunk_t
{
    unsigned int sub_chunk_id;
    unsigned int sub_chunk_size;
};


struct sound_handle_t sound_LoadWAV(char *file_name)
{
    FILE *file;

	char *file_buffer;
	unsigned int file_buffer_size;
	struct sound_handle_t sound = INVALID_SOUND_HANDLE;

	float relative_sample_value;

	int format;

	int i;

	int j;

	struct wav_riff_chunk_t *riff_chunk;
	struct wav_fmt_chunk_t *fmt_chunk;
	struct wav_data_chunk_t *data_chunk;

	char *sound_buffer;
	char *temp_sound_buffer;
	unsigned int sound_buffer_size;

	int sample;

	char *in;

    file = path_TryOpenFile(file_name);

    if(file)
	{
		file_buffer_size = path_GetFileSize(file);

		file_buffer = memory_Calloc(1, file_buffer_size);
		fread(file_buffer, file_buffer_size, 1, file);
		fclose(file);

        in = file_buffer;

        riff_chunk = (struct wav_riff_chunk_t *)in;
        in += sizeof(struct wav_riff_chunk_t);

		fmt_chunk = (struct wav_fmt_chunk_t *)in;
		in += sizeof(struct wav_fmt_chunk_t);

        data_chunk = (struct wav_data_chunk_t *)in;
        in += sizeof(struct wav_data_chunk_t);


		/* seems counter-intuitive to divide this by eight here, but sound_SetSoundData
		requires sound_buffer_size as being the amount of samples inside it, regardless
		of their size... */
		sound_buffer_size = data_chunk->sub_chunk_size / (fmt_chunk->bits_per_sample >> 3);
		sound_buffer = memory_Calloc(fmt_chunk->bits_per_sample >> 3, sound_buffer_size);

		memcpy(sound_buffer, in, sound_buffer_size);

		switch(fmt_chunk->bits_per_sample)
		{
			#if 0
			case 24:
                /* this will need to be resampled... */

                temp_buffer = memory_Calloc(2, sound_buffer_size);

                for(i = 0, j = 0; i < data_chunk->sub_chunk_size;)
				{
					sample = in[i];
					i++;
					sample |= ((int)in[i]) << 8;
					i++;
					sample |= ((int)in[i]) << 16;

					relative_sample_value = (float)sample / (float)0xffffff;

                    temp_buffer[j] = 0xffff



				}

			break;

			#endif

			case 16:
				if(fmt_chunk->channel_count == 2)
				{
                    format = AL_FORMAT_STEREO16;
				}
				else
				{
					format = AL_FORMAT_MONO16;
				}
			break;

			case 8:
				if(fmt_chunk->channel_count == 2)
				{
					format = AL_FORMAT_STEREO8;
				}
				else
				{
					format = AL_FORMAT_MONO8;
				}
			break;
		}


		sound = sound_CreateEmptySound(NULL);
		sound_SetSoundData(sound, sound_buffer, sound_buffer_size, format, fmt_chunk->sample_rate);


		memory_Free(file_buffer);
	}

	return sound;
}


struct sound_handle_t sound_LoadOGG(char *file_name)
{
	OggVorbis_File ogg_file;
	vorbis_info *file_info;
	struct sound_handle_t sound = INVALID_SOUND_HANDLE;
//	FILE *file;

//	char *path;

	unsigned long samples;
	int channels;
	double duration;

	unsigned long sound_buffer_size;
	short *sound_buffer;

	int format;
	int bytes_read;
	int sample_size = 1;
	unsigned int write_offset = 0;

	//path = path_GetPathToFile(file_name);

	int cur_bitstream;

	//if(path)
	//{
	if(!ov_fopen(file_name, &ogg_file))
	{
		file_info = ov_info(&ogg_file, -1);

		if(file_info)
		{
            samples = ov_pcm_total(&ogg_file, -1);
            duration = ov_time_total(&ogg_file, -1);
            channels = file_info->channels;

			/* seems like ov_pcm_total reports the amount of samples
			per channel, instead of the amount of samples for all channels... */
            sound_buffer_size = samples * channels;

			sound_buffer = memory_Calloc(sound_buffer_size, sizeof(short));

				//while(ov_read(&ogg_file, (char *)sound_buffer, 4096, 0, 1, 1, &cur_bitstream));

			do
			{
				bytes_read = ov_read(&ogg_file, (char *)sound_buffer + write_offset, 8192, 0, 2, 1, &cur_bitstream);
				write_offset += bytes_read;
			}while(bytes_read);


			switch(file_info->channels)
			{
				case 2:
					format = AL_FORMAT_STEREO16;
				break;

				case 1:
					format = AL_FORMAT_MONO16;
				break;


			}


			sound = sound_CreateEmptySound(NULL);
			sound_SetSoundData(sound, sound_buffer, sound_buffer_size, format, file_info->rate);
		}

		ov_clear(&ogg_file);
	}
	//}

	return sound;
}

struct sound_handle_t sound_GetSound(char *name)
{
	int i;
	int c;
	struct sound_t *sounds;
	struct sound_handle_t sound_handle = INVALID_SOUND_HANDLE;

	c = snd_sounds.element_count;
	sounds = (struct sound_t *)snd_sounds.elements;

	for(i = 0; i < c; i++)
	{
        if(sounds[i].flags & SOUND_FLAG_INVALID)
		{
			continue;
		}

		if(!strcmp(name, sounds[i].name))
		{
            sound_handle.sound_index = i;
            break;
		}
	}

	if(sound_handle.sound_index == INVALID_SOUND_INDEX)
	{
		printf("sound_GetSound: no sound named [%s] found\n", name);
	}

	return sound_handle;
}

struct sound_t *sound_GetSoundPointer(struct sound_handle_t sound)
{
	struct sound_t *sound_ptr = NULL;

    if(sound.sound_index != INVALID_SOUND_INDEX)
	{
        sound_ptr = stack_list_get(&snd_sounds, sound.sound_index);
	}

	return sound_ptr;
}



struct sound_source_t *sound_GetSoundSourcePointer(int sound_source)
{
	struct sound_source_t *source;

    if(sound_source >= 0 && sound_source < MAX_SOUND_SOURCES)
	{
		source = &snd_sound_sources[sound_source];

		if(source->flags & SOURCE_FLAG_ASSIGNED)
		{
			return source;
		}
	}

	return NULL;
}


int sound_IsSoundSourceAvailable()
{
	if(snd_next_in_sound_source == snd_next_out_sound_source - 1)
    {
        return 0;
    }

    return 1;
}

int sound_IsSoundCommandAvailable()
{
	if(snd_next_in_sound_command == snd_next_out_sound_command - 1)
	{
		return 0;
	}

	return 1;
}


int sound_AllocSoundSource()
{
	int source_index;

	/* first we get the position inside the indice ring buffer... */
	source_index = snd_next_in_sound_source;

    if(snd_next_in_sound_source == snd_next_out_sound_source - 1)
    {
        return -1;
    }

    snd_next_in_sound_source = (snd_next_in_sound_source + 1) % MAX_SOUND_SOURCES;

	/* then we get the actual source indice from the ring buffer... */
	source_index = snd_sound_sources_indices[source_index];

    return source_index;
}


int sound_EmitSoundCommand(int type, int source)
{
    int command_index;
    int source_index;
	struct sound_command_t *sound_command;

	//struct sound_t *sound_ptr;

	command_index = snd_next_in_sound_command;

    if(snd_next_in_sound_command == snd_next_out_sound_command - 1)
	{
		return -1;
	}

	if(type < SOUND_COMMAND_TYPE_START_SOUND || type > SOUND_COMMAND_TYPE_LAST)
	{
		return -1;
	}

	//printf("sound_EmitSoundCommand\n");

	sound_command = &snd_sound_commands[command_index];

	sound_command->cmd_id = type;
	sound_command->source = source;

	/* "submit" this command to the backend queue... */
	snd_next_in_sound_command = (snd_next_in_sound_command + 1) % MAX_SOUND_COMMANDS;

    return command_index;
}


int sound_PlaySound(struct sound_handle_t sound, vec3_t position, float gain, int flags)
{
	int source_index = -1;
	int stack_index;
	int sound_param_buffer_index;
	int command_index = -1;

	struct sound_t *sound_ptr;

	struct sound_command_t *sound_command;
	struct sound_source_t *sound_source;

    if(sound_IsSoundSourceAvailable() && sound_IsSoundCommandAvailable())
	{
		sound_ptr = sound_GetSoundPointer(sound);

		if(sound_ptr)
		{
			source_index = sound_AllocSoundSource();
			sound_source = &snd_sound_sources[source_index];

			sound_source->params.position = position;
			sound_source->params.gain = gain;
			sound_source->params.al_sound_buffer = sound_ptr->al_buffer_handle;
			sound_source->flags = flags | SOURCE_FLAG_ASSIGNED;
			sound_EmitSoundCommand(SOUND_COMMAND_TYPE_START_SOUND, source_index);
		}
		else
		{
			printf("sound_PlaySound: bad sound handle\n");
		}
	}

	return source_index;
}


void sound_PauseSound(int sound_source, int fade_out)
{
	struct sound_source_t *source;

	source = sound_GetSoundSourcePointer(sound_source);


	if(source)
	{
		if(fade_out)
		{
			source->flags |= SOURCE_FLAG_FADE_OUT;
		}

        sound_EmitSoundCommand(SOUND_COMMAND_TYPE_PAUSE_SOUND, sound_source);
	}
	else
	{
		printf("sound_PauseSound: bad sound source index\n");
	}
}

void sound_ResumeSound(int sound_source, int fade_in)
{
	struct sound_source_t *source;

	source = sound_GetSoundSourcePointer(sound_source);


    if(source)
	{
		if(fade_in)
		{
            source->flags |= SOURCE_FLAG_FADE_IN;
		}

		sound_EmitSoundCommand(SOUND_COMMAND_TYPE_RESUME_SOUND, sound_source);
	}
	else
	{
		printf("sound_ResumeSound: bad sound source index\n");
	}
}

void sound_StopSound(int sound_source, int fade_out)
{
	struct sound_source_t *source;

	source = sound_GetSoundSourcePointer(sound_source);


	if(source)
	{
		if(fade_out)
		{
			source->flags |= SOURCE_FLAG_FADE_OUT;
		}

        sound_EmitSoundCommand(SOUND_COMMAND_TYPE_STOP_SOUND, sound_source);
	}
	else
	{
		printf("sound_StopSound: bad sound source index\n");
	}
}

int sound_IsSourcePlaying(int sound_source)
{
    struct sound_source_t *source;
	int flags = 0;

    source = sound_GetSoundSourcePointer(sound_source);


    if(source)
	{
		flags = (source->flags & SOURCE_FLAG_PLAYING) && 1;
	}
	else
	{
		printf("sound_IsSourcePlaying: bad sound source index\n");
	}

	return flags;
}

int sound_IsSourceAssigned(int sound_source)
{
    struct sound_source_t *source;
	int flags = 0;

    source = sound_GetSoundSourcePointer(sound_source);


    if(source)
	{
		flags =  (source->flags & SOURCE_FLAG_ASSIGNED) && 1;
	}
	else
	{
		printf("sound_IsSourceAssigned: bad sound source index\n");
	}

	return flags;
}

void sound_SetSourcePosition(int sound_source, vec3_t position)
{
    struct sound_source_t *source;

    source = sound_GetSoundSourcePointer(sound_source);


    if(source)
    {
        source->params.position = position;
    }
    else
	{
		printf("sound_SetSourcePosition: bad sound source index\n");
	}
}

void sound_ResumeAllSounds()
{

}

void sound_PauseAllSounds()
{

}

void sound_StopAllSounds()
{
	sound_EmitSoundCommand(SOUND_COMMAND_TYPE_STOP_ALL_SOUNDS, -1);
}

void sound_ProcessSound()
{
//	camera_t *active_camera = camera_GetActiveCamera();

    view_def_t *active_view = renderer_GetActiveView();

	vec3_t orientation[2];

	orientation[0] = active_view->world_orientation.f_axis;
	orientation[1] = active_view->world_orientation.u_axis;

	orientation[0].x = -orientation[0].x;
	orientation[0].y = -orientation[0].y;
	orientation[0].z = -orientation[0].z;



	alListener3f(AL_POSITION, active_view->world_position.x, active_view->world_position.y, active_view->world_position.z);
	alListenerfv(AL_ORIENTATION, &orientation[0].floats[0]);
}

void sound_SuspendSoundBackend()
{

}

void sound_ResumeSoundBackend()
{

}


int sound_SoundBackend(void *param)
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
				case SOUND_COMMAND_TYPE_RESUME_SOUND:

                    /* we only need the al handle to check the source status... */
                    snd_active_sound_sources_indices[snd_active_sound_sources_count] = sound_command->source;
                    snd_active_sound_sources_count++;

                    sound_source->flags |= SOURCE_FLAG_PLAY;
				break;

				case SOUND_COMMAND_TYPE_PAUSE_SOUND:
					sound_source->flags |= SOURCE_FLAG_PAUSE;
				break;

				case SOUND_COMMAND_TYPE_STOP_SOUND:
					sound_source->flags |= SOURCE_FLAG_STOP;
				break;




				case SOUND_COMMAND_TYPE_RESUME_ALL_SOUNDS:
					for(r = 0; r < snd_active_sound_sources_count; r++)
					{
                        sound_EmitSoundCommand(SOUND_COMMAND_TYPE_RESUME_SOUND, snd_active_sound_sources_indices[r]);
					}
				break;

				case SOUND_COMMAND_TYPE_PAUSE_ALL_SOUNDS:
					for(r = 0; r < snd_active_sound_sources_count; r++)
					{
                        sound_EmitSoundCommand(SOUND_COMMAND_TYPE_PAUSE_SOUND, snd_active_sound_sources_indices[r]);
					}
				break;

				case SOUND_COMMAND_TYPE_STOP_ALL_SOUNDS:
					for(r = 0; r < snd_active_sound_sources_count; r++)
					{
                        sound_EmitSoundCommand(SOUND_COMMAND_TYPE_STOP_SOUND, snd_active_sound_sources_indices[r]);
					}
				break;


				case SOUND_COMMAND_TYPE_STOP_BACKEND:
					return 0;
			}

			snd_next_out_sound_command = (snd_next_out_sound_command + 1) % MAX_SOUND_COMMANDS;
		}


		for(r = 0; r < snd_active_sound_sources_count; r++)
		{
			source_index = snd_active_sound_sources_indices[r];

			sound_source = &snd_sound_sources[source_index];


			if(sound_source->flags & (SOURCE_FLAG_PLAY | SOURCE_FLAG_RESUME))
			{
				if(sound_source->flags & SOURCE_FLAG_FADE_IN)
				{
					sound_source->current_gain = 0.0;
					sound_source->flags |= SOURCE_FLAG_FADING_IN;
					sound_source->flags &= ~SOURCE_FLAG_FADE_IN;
				}
				else
				{
					sound_source->current_gain = sound_source->params.gain;
				}

				if(sound_source->flags & SOURCE_FLAG_PLAY)
				{
					alSourcei(sound_source->source_handle, AL_BUFFER, sound_source->params.al_sound_buffer);
				}

                sound_source->flags |= SOURCE_FLAG_PLAYING;
				sound_source->flags &= ~(SOURCE_FLAG_PLAY | SOURCE_FLAG_RESUME | SOURCE_FLAG_PAUSE | SOURCE_FLAG_STOP);

				alSourcePlay(sound_source->source_handle);
				alSourcei(sound_source->source_handle, AL_LOOPING, (sound_source->flags & SOURCE_FLAG_LOOP ? AL_TRUE : AL_FALSE));
				alSourcei(sound_source->source_handle, AL_SOURCE_RELATIVE, (sound_source->flags & SOURCE_FLAG_RELATIVE ? AL_TRUE : AL_FALSE));
			}


			else if(sound_source->flags & (SOURCE_FLAG_PAUSE | SOURCE_FLAG_STOP))
			{
                //if(sound_source->flags & SOURCE_FLAG_FADE_TRANSITION)
                if(sound_source->flags & SOURCE_FLAG_FADE_OUT)
				{
                    if(sound_source->current_gain > 0.0)
					{
                        sound_source->flags |= SOURCE_FLAG_FADING_OUT;
                        sound_source->flags &= ~(SOURCE_FLAG_FADING_IN | SOURCE_FLAG_FADE_OUT);
					}
				}

				if(!(sound_source->flags & SOURCE_FLAG_FADING_OUT))
				{
					if(sound_source->flags & SOURCE_FLAG_PAUSE)
					{
						sound_source->flags |= SOURCE_FLAG_PAUSED;
						alSourcePause(sound_source->source_handle);
					}
					else
					{
						sound_source->flags |= SOURCE_FLAG_STOPPED;
						alSourceStop(sound_source->source_handle);
					}

					sound_source->flags &= ~(SOURCE_FLAG_PLAY | SOURCE_FLAG_RESUME | SOURCE_FLAG_PAUSE | SOURCE_FLAG_STOP);
				}
			}



			if(sound_source->flags & SOURCE_FLAG_FADING_IN)
			{
				if(sound_source->current_gain < sound_source->params.gain)
				{
					sound_source->current_gain += 0.01;

					if(sound_source->current_gain > sound_source->params.gain)
					{
						sound_source->current_gain = sound_source->params.gain;
						sound_source->flags &= ~SOURCE_FLAG_FADING_IN;
					}
				}
			}
			else if(sound_source->flags & SOURCE_FLAG_FADING_OUT)
			{
                if(sound_source->current_gain > 0.0)
				{
					sound_source->current_gain -= 0.01;

					if(sound_source->current_gain < 0.0)
					{
						sound_source->current_gain = 0.0;
						sound_source->flags &= ~SOURCE_FLAG_FADING_OUT;
					}
				}
			}

			alGetSourcei(sound_source->source_handle, AL_SOURCE_STATE, &source_state);

			switch(source_state)
			{
				case AL_STOPPED:
					/* this source is done playing, so free it... */
					sound_source->flags = 0;

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

					continue;
				break;

				case AL_PLAYING:

				break;
			}

			//sound_source->flags &= ~(SOURCE_FLAG_PLAY | SOURCE_FLAG_PAUSE | SOURCE_FLAG_RESUME | SOURCE_FLAG_STOP);

			//alSourcef(sound_source->source_handle, AL_GAIN, sound_source->params.gain);
			alSourcef(sound_source->source_handle, AL_GAIN, sound_source->current_gain);
			alSource3f(sound_source->source_handle, AL_POSITION, sound_source->params.position.x, sound_source->params.position.y, sound_source->params.position.z);
		}
	}
}


#ifdef __cplusplus
}
#endif







