#ifndef R_SKYBOX_H
#define R_SKYBOX_H


typedef struct skybox_t
{
	struct skybox_t *next;
	unsigned int gl_handle;
	char *name;
}skybox_t;


int renderer_LoadSkybox(char *file_name, char *name);

void renderer_DeleteSkybox(char *name);

void renderer_SetSkybox(int skybox_index);

//void renderer_DrawSkybox();




#endif
