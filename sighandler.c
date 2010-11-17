#include "sighandler.h"

extern sigset_t fullset;
extern PIDTABLE *ptable;
extern PROCGROUP *foreground;
extern int ttyd;


/* Function: sighandler
*/
void sighandler(int signum, siginfo_t *si, void *context)
{
	if (-1 == sigprocmask(SIG_SETMASK, &fullset, NULL))
	{
		perror("sigprocmask");
	}
	int cldpid = si->si_pid, ret;
	PROCGROUP *cldgrp = NULL;
	cldgrp = (PROCGROUP*) pidtable_getpid(ptable, cldpid);
	switch(signum)
	{
		case SIGINT:
			break;
		case SIGQUIT:
			break;
		case SIGTSTP:
			break;
		case SIGCHLD:
			switch(si->si_code)
			{
				case CLD_EXITED:
					ret = tcgetpgrp(ttyd);
					if (ret == -1)
					{
						perror("tcgetpgrp");
					}
					else if (cldpid == ret)
					{
						pidtable_delpid(ptable, cldpid, FALSE, TRUE);
					}
					else
					{
						pidtable_delpid(ptable, cldpid, JOB_EXITED, TRUE);
					}
					break;

				case CLD_STOPPED:
					if (cldgrp != NULL)
					{
						cldgrp->status = STOPPED;
						printf("[%d] ", cldgrp->group_pid);
						procgroup_print(cldgrp);
					}
					else
					{
						int pgid;
						pgid = tcgetpgrp(ttyd);
						if (pgid == -1)
						{
							perror("tcgetpgrp");
						}
						else if (pgid != getpid())
						{
							if (-1 == kill(pgid, SIGSTOP))
							{
								perror("kill");
							}
							foreground->status = STOPPED;
							int table_id = pidtable_add(ptable, foreground);
							printf("[%d] %d\n", table_id, pgid);
							foreground = procgroup_init();
						}
						if (-1 == tcsetpgrp(ttyd, getpid()))
						{
							perror("tcsetpgrp");
						}
					}
					break;

				case CLD_KILLED:
					if (cldgrp != NULL && (--cldgrp->count) <= 0)
					{
						pidtable_delpid(ptable, cldpid, JOB_KILLED, TRUE);
					}
					break;

				case CLD_CONTINUED:
					if (cldgrp != NULL)
					{
						cldgrp->status = RUNNING;
					}
					break;

				default: break;
			}
	}

	int i, max;
	max = pidtable_getcapacity(ptable);
	for (i = 0; i < max; i++)
	{
		cldgrp = pidtable_getindex(ptable, i);
		manage_job(cldgrp);
	}
	if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
	{
		perror("sigprocmask");
	}
}


/* Function: manage_job
*/
void manage_job(PROCGROUP *pg)
{
	int status, res;
	if (pg != NULL)
	{
		status = 0;
		res = waitpid(-pg->group_pid, &status, WNOHANG|WUNTRACED|WCONTINUED);
		if (res == -1)
		{
			perror("waitpid");
		}
		if (res == 0)
		{
			return;
		}
		if (WIFSTOPPED(status) == TRUE)
		{
			pg->status = STOPPED;
		}
		else if (WIFEXITED(status) == TRUE)
		{
			{
				pidtable_delpid(ptable, pg->group_pid, JOB_EXITED, TRUE);
			}
		}
		else if (WIFSIGNALED(status) == TRUE)
		{
			{
				pidtable_delpid(ptable, pg->group_pid, JOB_KILLED, TRUE);
			}
		}
		else if (WIFCONTINUED(status) == TRUE)
		{
			pg->status = RUNNING;
		}
	}
}

