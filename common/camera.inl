#ifndef CAMERA_INL
#define CAMERA_INL

#include "camera_types.h"
//#include "draw_types.h"

extern int camera_list_size;
extern int camera_count;
extern camera_t *camera_list;
extern int active_camera_index;
//extern renderer_t renderer;


/*__forceinline camera_t *camera_GetActiveCamera()
{
	return &camera_list[active_camera_index];
}*/

/*PEWAPI inline framebuffer_set_t *camera_GetActiveFramebufferSet()
{
	return &camera_a.framebuffers[active_camera_index];
}*/

/*camera_t *camera_GetCamera(char *name)
{
	int i;
	int c = camera_count;
	
	for(i = 0; i < c; i++)
	{
		if(!strcmp(name, camera_list[i].name))
		{
			return &camera_list[i];
		}
	}
	
	return NULL;
	
}*/




#endif /* CAMERA_INL */
