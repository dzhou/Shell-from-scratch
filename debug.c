#include "debug.h"


void print_debug(const char *s)
{
#ifdef DEBUG
    printf("%s%s%s\n", MYSH_RED, s, MYSH_GRAY);
    fflush(stdout);
#endif
}
