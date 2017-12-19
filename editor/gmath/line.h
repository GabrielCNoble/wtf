#ifndef LINE_H
#define LINE_H

#include "line_types.h"


line2_t line2(vec2_t a, vec2_t b);

line3_t line3(vec3_t a, vec3_t b);

int line_PointInLine2(line2_t line, vec3_t p);




#endif /* LINE_H */
