#ifndef RENDERER_FRAMEBUFFER_H
#define RENDERER_FRAMEBUFFER_H

#include "r_common.h"

int renderer_CreateFramebuffer(int width, int height);

void renderer_DestroyFramebuffer(int framebuffer);

void renderer_BindFramebuffer(int framebuffer);




#endif

