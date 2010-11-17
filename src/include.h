#ifndef _INCLUDE_H_
#define _INCLUDE_H_

#include "debug.h"

/* C Standard library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include <stdarg.h>

/* System library */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>

/* Debugging messages for command line parser */
/*
#define DEBUG_PARSER_INFO
#define DEBUG_PARSER_PROCESS
*/

/* Debugging for PIDTABLE struct */
/*
#define DEBUG_PTABLE_INFO
*/

/* Debudding for procgroup */
/*
#define DEBUG_PROCGROUP_INFO
*/

/* general debugging */
/*
#define DEBUG
*/

/* Debugging for unittests */
#define DEBUG_TEST

/* output of warning/error messages */
#define WARNING

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


/* COLORS */
#define MYSH_BLACK "\033[22;30m"
#define MYSH_RED "\033[22;31m"
#define MYSH_GREEN "\033[22;32m"
#define MYSH_BROWN "\033[22;33m"
#define MYSH_BLUE "\033[22;34m"
#define MYSH_MAGENTA "\033[22;35m"
#define MYSH_CYAN "\033[22;36m"
#define MYSH_GRAY "\033[22;37m"
#define MYSH_DGRAY "\033[01;30m"
#define MYSH_LRED "\033[01;31m"
#define MYSH_LGREEN "\033[01;32m"
#define MYSH_YELLOW "\033[01;33m"
#define MYSH_LBLUE "\033[01;34m"
#define MYSH_LMAGENTA "\033[01;35m"
#define MYSH_LCYAN "\033[01;36m"
#define MYSH_WHITE "\033[01;37m"


#endif /* _INCLUDE_H_ */
