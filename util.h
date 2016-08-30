#include <stdarg.h>
#include <stdio.h>

/*
https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-g_t_0040code_007bformat_007d-function-attribute-3238
*/

void log_print(const char* msg, ...)
	__attribute__((format (printf, 1, 2)));
