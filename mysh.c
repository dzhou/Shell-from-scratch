#include "mysh.h"

extern sigset_t fullset;
extern PIDTABLE *ptable;
extern PROCGROUP *foreground;
extern int ttyd;


/* Function: shell_pwd
   print current directory
*/
int shell_pwd()
{
	int size;
	char *p = NULL;

	for (size = INTERNAL_BUF; ; size += INTERNAL_BUF)
	{
		char buf[size];
		p = getcwd((char*)&buf, size);
		if (p == NULL && errno == ERANGE)
		{
			continue;
		}
		else if (p == NULL)
		{
			perror("pwd");
		}
		printf("%s", p);
		break;
	}

	return 0;
}


/* Function: shell_cd
   change current working directory
*/
int shell_cd(const char *path)
{
	if (path == NULL)
	{
		return 0;
	}

	if (chdir(path) == -1)
	{
		printf("-mysh: cd %s: ", path);
		fflush(stdout);
	}

	return 0;
}


/* Function
*/
int shell_atoi(const char *s)
{
	if (s != NULL && *s++ == '%')
	{
		return atoi(s);
	}
	else
	{
		return -1;
	}
}


/* Function: shell_run
   Handles all shell built-ins
*/
int shell_run(const char *cmd, const char *arg)
{
	int table_id = shell_atoi(arg);
	PROCGROUP *pgrp = NULL;

	if (cmd == NULL)
	{
		// no command
		return MYSH_NEXT;
	}

	else if (strncmp(cmd, "jobs", 4) == 0)
	{
		pidtable_print(ptable);
	}

	else if (strncmp(cmd, "exit", 4) == 0)
	{
		return MYSH_EXIT;
	}

	else if (strncmp(cmd, "kill", 4) == 0)
	{
		pgrp = (PROCGROUP*) pidtable_getindex(ptable, table_id);
		if (pgrp == NULL || table_id == -1)
		{
#ifdef WARNING
	printf("-mysh: kill: %s: no such job\n", arg);
#endif
		}
		else
		{
			if (-1 == kill(pgrp->group_pid, SIGKILL))
			{
				perror("kill");
			}
		}
	}

	else if (strncmp(cmd, "pwd", 3) == 0)
	{
		shell_pwd();
		printf("\n");
	}

	else if (strncmp(cmd, "cd", 2) == 0)
	{
		shell_cd(arg);
	}

	else if (strncmp(cmd, "bg", 2) == 0)
	{
		pgrp = (PROCGROUP*) pidtable_getindex(ptable, table_id);
		if (pgrp == NULL || table_id == -1)
		{
#ifdef WARNING
	printf("-mysh: bg: %s: no such job\n", arg);
#endif
		}
		else
		{
			if (-1 == kill(pgrp->group_pid, SIGCONT))
			{
				perror("kill");
			}
		}
	}

	else if (strncmp(cmd, "fg", 2) == 0)
	{
		if (-1 == sigprocmask(SIG_SETMASK, &fullset, NULL))
		{
			perror("sigprocmask");
		}
		procgroup_free(foreground);
		foreground = (PROCGROUP*) pidtable_getindex(ptable, table_id);
		if (foreground == NULL || table_id == -1)
		{
#ifdef WARNING
	printf("-mysh: fg: %s: no such job\n", arg);
#endif
			foreground = procgroup_init();
			if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
			{
				perror("sigprocmask");
			}
		}
		else
		{
			if (-1 == kill(foreground->group_pid, SIGTSTP))
			{
				perror("kill");
			}
			printf("%s\n", foreground->cmdline);
			pidtable_delpid(ptable, foreground->group_pid, FALSE, FALSE);
			pid_t gid = getpgid(foreground->group_pid);
			if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
			{
				perror("sigprocmask");
			}
			if (gid == -1)
			{
				perror("getpgid");
			}
			if (-1 == tcsetpgrp(ttyd, gid))
			{
				perror("tcsetpgrp");
			}
			if (-1 == kill(foreground->group_pid, SIGCONT))
			{
				perror("kill");
			}
			int wait_id = -1;
			while(wait_id == -1)
			{
				wait_id = waitpid(-gid, NULL, WUNTRACED);
			}
			if (-1 == tcsetpgrp(ttyd, getpid()))
			{
				perror("tcsetpgrp");
			}
		}
	}

	else
	{
		// Not a built-in command
		return MYSH_EXTC;
	}

	return MYSH_NEXT;
}


/* Function: pipe_command
   piping
*/
int pipe_command(const COMMAND *cmp)
{
	print_debug("DEBUG: Begin piping");

	int pipefd[2] = {STDIN_FILENO,STDOUT_FILENO}, pipefd_old[2] = {STDIN_FILENO,STDOUT_FILENO};
	int pidn, fd, ret = 0, location = 0, wait_id = -1;
	int gpid = 0;
	int count = 1;

pipe_loop:
	if (cmp->next == NULL)
	{
		goto pipe_last;
	}

	pipefd_old[0] = pipefd[0];
	pipefd_old[1] = pipefd[1];
	if (pipe(pipefd) == -1)
	{
#ifdef WARNING
		perror("pipe");
#endif
		ret = -1;
		goto pipe_terminate;
	}

	count ++;
	pidn = fork();
	if (pidn == -1)
	{
		perror("fork");
		ret = -1;
		goto pipe_terminate;
	}

	// Child
	if (pidn == 0)
	{
		if (gpid == 0)
		{
			gpid = getpid();
		}
		if (-1 == setpgid(getpid(), gpid))
		{
			perror("setpgid");
		}
		if (cmp->background == FALSE)
		{
			if (-1 == tcsetpgrp(ttyd, gpid))
			{
				perror("tcsetpgrp");
			}
		}

#ifdef DEBUG
	printf("CHILD: New child %d in group %d\n", getpid(), gpid);
#endif

		// Input redirect
		if (cmp->infile != NULL)
		{
			print_debug("DEBUG: Setting input file");
			fd = open(cmp->infile, cmp->fdmode);
			if (fd == -1)
			{
				perror("open");
			}
			if (-1 == dup2(fd, STDIN_FILENO))
			{
				perror("dup2");
			}
			if (-1 == close(fd))
			{
				perror("close");
			}
		}

		if (location != 0)
		{
			if (-1 == dup2(pipefd_old[0], 0))
			{
				perror("dup");
			}
			if (-1 == close(pipefd_old[0]))
			{
				perror("close");
			}
			if (-1 == close(pipefd_old[1]))
			{
				perror("close");
			}
		}

		if (-1 == dup2(pipefd[1], STDOUT_FILENO))
		{
			perror("dup2");
		}
		if (-1 == close(pipefd[0]))
		{
			perror("close");
		}
		if (-1 == close(pipefd[1]))
		{
			perror("close");
		}

		if (execvp(cmp->argv[0], cmp->argv) == -1)
		{
			_exit(0);
		}
	}
	// Parent, close old pipes
	else{
		if (gpid == 0)
		{
			gpid = pidn;
		}
		if (-1 == setpgid(pidn, gpid))
		{
			// Dont report error
		}
		if (cmp->background == FALSE)
		{
			if (-1 == tcsetpgrp(ttyd, gpid))
			{
				perror("tcsetpgrp");
			}
		}

#ifdef DEBUG
	printf("PARENT: New child %d in group %d\n", pidn, gpid);
#endif

		if (location == 0)
		{
			if (-1 == sigprocmask(SIG_SETMASK, &fullset, NULL))
			{
				perror("sigprocmask");
			}
			procgroup_load(foreground, gpid, RUNNING, cmp->cmdline);
			if (cmp->background == TRUE)
			{
				int table_id = pidtable_add(ptable, foreground);
				foreground = procgroup_init();
				printf("[%d] %d\n", table_id, gpid);
			}
			if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
			{
				perror("sigprocmask");
			}
		}
		if (location++ != 0)
		{
			if (-1 == close(pipefd_old[0]))
			{
				perror("close");
			}
			if (-1 == close(pipefd_old[1]))
			{
				perror("close");
			}
		}

		cmp = cmp->next;
		if (cmp->pipe == TRUE)
		{
			goto pipe_loop;
		}
	}

pipe_last:

	pidn = fork();
	if (pidn == 0)
	{
		if (-1 == setpgid(getpid(), gpid))
		{
			perror("setpgid");
		}
#ifdef DEBUG
	printf("CHILD: Last New child %d in group %d\n", getpid(), gpid);
#endif

		// Output redirect
		if (cmp->outfile != NULL)
		{
			print_debug("DEBUG: Setting output file");
			fd = open(cmp->outfile, cmp->fdmode);
			if (fd == -1)
			{
				// File does not exists, create
				fd = creat(cmp->outfile, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			}
			if (fd == -1)
			{
				perror("creat");
			}
			if (-1 == dup2(fd, STDOUT_FILENO))
			{
				perror("dup2");
			}
			if (-1 == close(fd))
			{
				perror("close");
			}
		}

		if (-1 == dup2(pipefd[0], 0))
		{
			perror("dup2");
		}
		if (-1 == close(pipefd[0]))
		{
			perror("close");
		}
		if (-1 == close(pipefd[1]))
		{
			perror("close");
		}

		if (execvp(cmp->argv[0], cmp->argv) == -1)
		{
			_exit(0);
		}
	}
	// Parent
	else{
		if (-1 == setpgid(pidn, gpid))
		{
			// Dont report error here
		}

#ifdef DEBUG
	printf("PARENT: Last New child %d in group %d\n", pidn, gpid);
#endif

		if (-1 == close(pipefd[0]))
		{
			perror("close");
		}
		if (-1 == close(pipefd[1]))
		{
			perror("close");
		}

		foreground->count = count;
		if (cmp->background == FALSE)
		{
			wait_id = -1;
			while(wait_id == -1 || foreground->count > 0)
			{
				wait_id = waitpid(-gpid, NULL, WUNTRACED|WCONTINUED);
				foreground->count--;
			}
		}
		//printf("set %d to foreground\n", getpid());
		if (-1 == tcsetpgrp(ttyd, getpid()))
		{
			perror("tcsetpgrp");
		}
	}
	//printf("End pipe %d\n", count);
	print_debug("DEBUG: End piping");

pipe_terminate:

	if (cmp->next != NULL)
	{
		ret = exec_command(cmp->next);
	}

	return ret;
}


/* Function: exec_command
*/
int exec_command(const COMMAND *cmp)
{
	int ret = 0, wait_id = -1, cld_pid, fd, table_id;
	int fatal_err = FALSE;

	ret = shell_run(cmp->argv[0], cmp->argv[1]);
	switch(ret)
	{
		case MYSH_EXTC: break;
		case MYSH_NEXT:	goto exec_next;
		case MYSH_EXIT: goto exec_terminate;
		default: break;
	}

	if (cmp->pipe == TRUE)
	{
		pipe_command(cmp);
		goto exec_terminate;
	}

	cld_pid = fork();
	if (cld_pid == -1)
	{
		perror("fork");
		goto exec_terminate;
	}
	// Child
	else if (cld_pid == 0)
	{
		// set process group, get terminal if foreground
		if (-1 == setpgid(getpid(), getpid()))
		{
			perror("setpgid");
		}
		if (cmp->background == FALSE)
		{
			if (-1 == tcsetpgrp(ttyd, getpid()))
			{
				perror("tcsetpgrp");
			}
		}

		// Input redirect
		if (cmp->infile != NULL)
		{
			print_debug("DEBUG: Setting input file");
			fd = open(cmp->infile, cmp->fdmode);
			if (fd == -1)
			{
				perror("open");
				fatal_err = -1;
			}
			else
			{
				if (-1 == dup2(fd, 0))
				{
					perror("dup2");
					fatal_err = -1;
				}
				if (-1 == close(fd))
				{
					perror("close");
					fatal_err = -1;
				}
			}
		}

		// Output redirect
		if (cmp->outfile != NULL)
		{
			print_debug("DEBUG: Setting output file");
			fd = open(cmp->outfile, cmp->fdmode);
			if (fd == -1)
			{
				// File does not exists, create
				fd = creat(cmp->outfile, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			}
			if (-1 == dup2(fd, 1))
			{
				perror("dup2");
				fatal_err = -1;
			}
			if (-1 == close(fd))
			{
				perror("close");
				fatal_err = -1;
			}
		}

		if (fatal_err == -1)
		{
			_exit(-1);
		}

		// Execute command
		if (execvp(cmp->argv[0], cmp->argv) == -1)
		{
#ifdef WARNING
			switch(errno)
			{
				case ENOENT:
					printf("-mysh: %s: command not found\n", cmp->argv[0]);
					break;
				default:
					perror("-mysh: ");
			}
#endif
			_exit(-1);
		}

	}
	// Parent
	else {
		if (-1 == sigprocmask(SIG_SETMASK, &fullset, NULL))
		{
			perror("sigprocmask");
		}
		// Set process group
		if (-1 == setpgid(cld_pid, cld_pid))
		{
			//Don't report error here
		}
		// create procgroup
		procgroup_load(foreground, cld_pid, RUNNING, cmp->cmdline);
		if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
		{
			perror("sigprocmask");
		}

		// set to background, add to pidtable
		if (cmp->background == TRUE)
		{
			if (-1 == sigprocmask(SIG_SETMASK, &fullset, NULL))
			{
				perror("sigprocmask");
			}
			table_id = pidtable_add(ptable, foreground);
			foreground = procgroup_init();
			if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
			{
				perror("sigprocmask");
			}
			printf("[%d] %d\n", table_id, cld_pid);
		}
		// Set to foreground
		else {
			if (-1 == tcsetpgrp(ttyd, cld_pid))
			{
				perror("tcsetpgrp");
			}
			// Wait for a specific PID
			while(wait_id == -1)
			{
				wait_id = waitpid(-cld_pid, NULL, WUNTRACED|WCONTINUED);
			}
			if (-1 == tcsetpgrp(ttyd, getpid()))
			{
				perror("tcsetpgrp");
			}
		}
	}

exec_next:
	if (cmp->next != NULL)
	{
		ret = exec_command((COMMAND*)cmp->next);
	}

exec_terminate:
	return ret;
}


/* MAIN */
int main()
{
	// variable and data structures
	int ret, status;
	COMMAND *cmd = NULL;
	char buffer[CMD_MAX], *read;
	ptable = pidtable_init();
	ttyd = open("/dev/tty", O_RDWR, 0700);
	if (ttyd == -1)
	{
		perror("open");
	}

	// Create set containing all signals for blocking
	if (-1 == sigfillset(&fullset))
	{
		perror("sigfillset");
	}

	// Setup Signal Handler
	struct sigaction sa;
	sa.sa_flags |= SA_SIGINFO;
	sa.sa_sigaction = sighandler;
	if (-1 == sigaction(SIGINT, &sa, NULL))
	{
		perror("sigaction");
	}
	if (-1 == sigaction(SIGQUIT, &sa, NULL))
	{
		perror("sigaction");
	}
	if (-1 == sigaction(SIGCHLD, &sa, NULL))
	{
		perror("sigaction");
	}
	if (-1 == sigaction(SIGTSTP, &sa, NULL))
	{
		perror("sigaction");
	}
	if (-1 == sigaction(SIGTTOU, &sa, NULL))
	{
		perror("sigaction");
	}
	if (-1 == (int)signal(SIGTTOU, SIG_IGN))
	{
		perror("signal");
	}

#ifdef DEBUG
	printf("%s", MYSH_RED);
	int pgid;
	pgid = tcgetpgrp(ttyd);
	printf("DEBUG: Shell id (%d) in (%d)\n", getpid(), getpgid(getpid()));
	printf("DEBUG: Terminal Group id: %d\n", pgid);
	printf("%s", MYSH_GRAY);
#endif

	foreground = procgroup_init();

	// begin main loop
	while(TRUE)
	{
		tcsetpgrp(ttyd, getpid());

		ret = waitpid(-1, &status, WNOHANG|WCONTINUED);

		// Shell prompt
		printf("%sMysh%s ", MYSH_LGREEN, MYSH_LBLUE);
		shell_pwd();
		printf(" #%s ", MYSH_GRAY);

		// Get input from stdin
		memset(&buffer, CMD_MAX, '\0');
		read = NULL;
		while (read == NULL)
		{
			read = fgets(buffer, CMD_MAX, stdin);
		}

		if (-1 == sigprocmask(SIG_SETMASK, &fullset, NULL))
		{
			perror("sigprocmask");
		}
		procgroup_free(foreground);
		foreground = procgroup_init();
		if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
		{
			perror("sigprocmask");
		}

		// Parse/tokenize command
		cmd = command_parse(buffer);


#ifdef DEBUG
	printf("%s", MYSH_RED);
	command_print(cmd);
	printf("%s", MYSH_GRAY);
	fflush(stdout);
#endif

		ret = exec_command(cmd);
		print_debug("DEBUG: Free command struct");

		// reset data structure
		command_free(cmd);

		switch (ret)
		{
			case 10: goto finalize;
			default: break;
		}
	}

finalize:
	print_debug("DEBUG: Exiting shell");
	if (-1 == sigprocmask(SIG_SETMASK, &fullset, NULL))
	{
		perror("sigprocmask");
	}
	procgroup_free(foreground);
	pidtable_free(ptable);
	if (-1 == sigprocmask(SIG_UNBLOCK, &fullset, NULL))
	{
		perror("sigprocmask");
	}
	return 0;
}
