#ifndef _MYSH_H_
#define _MYSH_H_

#include "include.h"
#include "procgroup.h"
#include "pidtable.h"
#include "parser.h"
//#include "internal.h"
#include "sighandler.h"

/* Max length for each command line */
#define CMD_MAX 5120

#define INTERNAL_BUF 32

/* RETURN CODE */
#define MYSH_OK		0
#define MYSH_EXIT	10
#define MYSH_NEXT	12
#define MYSH_TERM	15
#define MYSH_ERR	-2
#define MYSH_EXTC	9

/* Foreground process */
PROCGROUP *foreground;

/* set of signal to block */
sigset_t fullset;

/* Pidtable data structure */
PIDTABLE *ptable;

/* Terminal File*/
int ttyd;

/* Functions */

/* Function: pipe_command
   Execute jobs require piping
*/
int pipe_command(const COMMAND *cmp);

/* Function: exec_command
   check command for builtin or pipe
   Otherwise execute a single command job
*/
int exec_command(const COMMAND *cmp);

/* Function: shell_run
   Check for shell builtin command and execute
*/
int shell_run(const char *cmd, const char *arg);

/* Function: shell_atoi
   atio function with error handling
*/
int shell_atoi(const char *s);

/* Function: shell_cd
   Builtin command cd
*/
int shell_cd(const char *path);

/* Function: shell_pwd
   Builtin command pwd
*/
int shell_pwd();


#endif /* _MYSH_H_ */
