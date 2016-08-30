#include <stdarg.h>
#include <stdio.h>

void log_print(const char* msg, ...){
        va_list va;
        FILE* fd;
        va_start(va, msg);
        if((fd = fopen("../log.txt", "a")) != 0){
                vfprintf(fd, msg, va);
                fclose(fd);
        }
        va_end(va);
}
