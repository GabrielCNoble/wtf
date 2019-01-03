mingw32-gcc -g -g3 -m32 -c mpkcvt.c -o mpkcvt.o
mingw32-g++ -m32 -o mpkcvt.exe ..\gmath\vector.o ..\c_memory.o ..\mpk_write.o mpkcvt.o