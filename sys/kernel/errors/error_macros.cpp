#include "error_macros.h"
#include "stdio.h"

void _err_print_error(const char *p_function, const char *p_file, int p_line, const char *p_error, ErrorHandlerType p_type) {
	printf("Error %s Occured in: %s, located in %s.line %d!\r\n", p_error, p_function, p_file, p_line);
}