#include "procgroup.h"


/* Function: procgroup_init
   Initialize procgroup struct
*/
PROCGROUP *procgroup_init()
{
	PROCGROUP *pg;
	pg = (PROCGROUP*) malloc(sizeof (PROCGROUP));
	pg->group_pid = 0;
	pg->count = 0;
	pg->status = 0;
	pg->cmdline = (char*) malloc(sizeof (char) * PROCGROUP_BUF);
	memset(pg->cmdline, '\0', PROCGROUP_BUF);

#ifdef DEBUG_PROCGROUP_INFO
	printf("PROCGROUP: Struct initialized (%d bytes)\n", sizeof (PROCGROUP));
#endif

	return pg;
}


/* Function: procgroup_clear
   Reset procgroup to default values
*/
void procgroup_clear(PROCGROUP *pg)
{
	pg->group_pid = 0;
	pg->count = 0;
	pg->status = 0;
	memset(pg->cmdline, '\0', PROCGROUP_BUF);

#ifdef DEBUG_PROCGROUP_INFO
	printf("PROCGROUP: Procgroup data reset\n");
#endif
}


/* Function: procgroup_free
   Deallocates procgroup
*/
void procgroup_free(PROCGROUP *pg)
{
	free(pg->cmdline);
	free(pg);
	pg = NULL;

#ifdef DEBUG_PROCGROUP_INFO
	printf("PROCGROUP: Struct destroyed\n");
#endif
}


/* Function: procgroup_load
   Load process info (gpid, status, and commandline) into procgroup
*/
void procgroup_load(PROCGROUP *pg, int gpid, short status, char *line)
{
	pg->group_pid = gpid;
	pg->status = status;
	pg->count = 1;
	strncpy(pg->cmdline, line, PROCGROUP_BUF);

#ifdef DEBUG_PROCGROUP_INFO
	printf("PROCGROUP: Data loaded\n");
#endif
}


/* Function: procgroup_print
   Print out procgroup (debug)
*/
void procgroup_print(PROCGROUP *pg)
{
	switch(pg->status)
	{
		case RUNNING:
			printf("Running\t%s\n", pg->cmdline);
			break;
		case STOPPED:
			printf("Stopped\t%s\n", pg->cmdline);
			break;
	}
}

