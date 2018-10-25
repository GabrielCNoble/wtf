#ifndef MACROS_H
#define MACROS_H



#define ALWAYS_FORCE_INLINE __attribute__((always_inline)) inline
#define OFFSETOF(type, field) ((int)(&((type *)0)->field))

#endif // MACROS_H
