#ifndef _SIGHANDLER_H_
#define _SIGHANDLER_H_

#include "include.h"
#include "procgroup.h"
#include "pidtable.h"
#include "parser.h"
#include "mysh.h"

void sighandler(int signum, siginfo_t *si, void *context);
void manage_job(PROCGROUP *pg);

#endif /* _SIGHANDLER_H_ */
