/*
	PROCGROUP is the data storage for a process group.
	Each PROCGROUP contains group pid, current status, and the command line that started the process group
	Everytime the shell execute a job, a PROCGROUP is created with the corresponding data
	If the job is in foreground and exits, the shell frees the PROCGROUP
	If the job is started in background or is placed in the background, PROCGROUP is loaded into pidtable.
	If the job is in the pidtable when it exits, the pidtable will handle the deallocation
*/

#ifndef _PROCGROUP_H_
#define _PROCGROUP_H_

#include "include.h"

#define STOPPED 2
#define RUNNING 3

#define PROCGROUP_BUF 64


/* Typedef PROCGROUP
   Stores pid/pgid, status and command line
   At init, group_pid and status defaults to 0, cmdline is dynamically allocated and set to '\0'
*/
typedef struct procgroup {
	int group_pid;
	int count;
	short status;
	char *cmdline;
} PROCGROUP;


/* Function: procgroup_init
   Initializes PROCGROUP
   At init, group_pid and status defaults to 0, cmdline is dynamically allocated and set to '\0'
*/
PROCGROUP *procgroup_init();


/* Function: procgroup_clear
   Clears current data, reset to same state as after initialization
   Precondition: pg is a valid pointer to a PROCGROUP struct
*/
void procgroup_clear(PROCGROUP *pg);


/* Function: procgroup_free
   Deallocates cmdline and PROCGROUP
   Precondition: pg is a valid pointer to a PROCGROUP struct
*/
void procgroup_free(PROCGROUP *pg);


/* Function: procgroup_load
   Load in gpid and status value
   char *line is copied to cmdline, limited to size of PROCGROUP_BUF
   Precondition: pg is a valid pointer to a PROCGROUP struct
*/
void procgroup_load(PROCGROUP *pg, int gpid, short status, char *line);


/* Function: procgroup_print
   Prints out process info
   Precondition: pg is a valid pointer to a PROCGROUP struct
*/
void procgroup_print(PROCGROUP *pg);


#endif /* _PROCGROUP_H_ */
