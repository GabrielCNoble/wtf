#include "r_script.h"
#include "r_debug.h"


void renderer_ScriptDrawPoint(vec3_t *position, vec3_t *color)
{
    renderer_DrawPoint(*position, *color, 8.0, 1, 0, 1);
}

void renderer_ScriptDrawLine(vec3_t *from, vec3_t *to, vec3_t *color)
{
    renderer_DrawLine(*from, *to, *color, 1.0, 0);
}
