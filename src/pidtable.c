#include "pidtable.h"


/* Function: init_pidtable
   Initialize pidtable
   Create a new node, return node pointer
*/
PIDTABLE *pidtable_init()
{
	// allocate to heap
	PIDTABLE *pt;
	pt = (PIDTABLE *) malloc(sizeof (PIDTABLE));

	// Set default values
	pt->next = NULL;
	pt->offset = 0;
	pt->size = 0;
	int i;
	for (i = 0; i < PTABLE_SIZE; i++)
	{
		pt->job[i] = NULL;
	}

#ifdef DEBUG_PTABLE_INFO
	printf("PIDTABLE: Pidtable created (%d bytes)\n", sizeof (PIDTABLE));
#endif

	return pt;
}


/* Function: free_pidtable
   Recursive pidtable destructor
*/
void pidtable_free(PIDTABLE *table)
{
	int i;
	for (i = 0; i < PTABLE_SIZE; i++)
	{
		if (table->job[i] != NULL)
		{
			procgroup_free(table->job[i]);
		}
	}

	// recursive free
	if (table->next != NULL)	{
		pidtable_free((PIDTABLE*)table->next);
	}

	// deallocate current node
	free (table);

#ifdef DEBUG_PTABLE_INFO
	printf("PIDTABLE: Pidtable destroyed\n");
#endif
}


/* Function: add_pidtable
   Add PROCGROUP type to table, If table is full then add new node
   Returns index of the procgroup
*/
int pidtable_add(PIDTABLE *table, PROCGROUP *pg)
{
#ifdef DEBUG_PTABLE_INFO
	printf("PIDTABLE: Adding (%d) to table\n", pid);
#endif

	PIDTABLE *np = table;

	// find the first node that's not full
	while(np->size == PTABLE_SIZE)
	{
		if (np->next != NULL)
		{
			np = (PIDTABLE*)np->next;
		}
		else
		{
#ifdef DEBUG_PTABLE_INFO
	printf("PIDTABLE: Table full, expanding node\n");
#endif
			PIDTABLE *new_np = NULL;
			new_np = pidtable_init();
			new_np->offset = np->offset + 1;
			np->next = new_np;
			return pidtable_add(new_np, pg);
		}
	}

	// add process
	int i;
	for (i = 0; i < PTABLE_SIZE; i++)
	{
		if (np->job[i] == NULL)
		{
			np->job[i] = pg;
			np->size++;
			return i + (PTABLE_SIZE*(np->offset))+1;
		}
	}

	return 0;
}


/* Function: del_pidtable
   Delete process from table
*/
int pidtable_delpid(PIDTABLE *table, int pid, int type, int free)
{
	PIDTABLE *np = table;
	while(np)
	{
		int i = 0;
		for (; i < PTABLE_SIZE; i++)
		{
			if (np->job[i] != NULL && np->job[i]->group_pid == pid)
			{
				// print entry
				switch(type)
				{
					case JOB_EXITED:
						printf("[%d]  Done\t %s\n", np->offset * PTABLE_SIZE + i + 1, np->job[i]->cmdline);
						break;
					case JOB_KILLED:
						printf("[%d]  Terminated\t %s\n", np->offset * PTABLE_SIZE + i + 1, np->job[i]->cmdline);
						break;
					case FALSE:
						break;
				}
				if (free == TRUE)
				{
					procgroup_free(np->job[i]);
				}
				np->job[i] = NULL;
				np->size--;

#ifdef DEBUG_PTABLE_INFO
	printf("PIDTABLE: Deleted (%d) from table\n", pid);
#endif
				/*
					if cur node size is 0 and prev node size is < limit
					deallocate current node and reset pointer
				*/
				if (pidtable_getsize(np) == 0)
				{
					pidtable_shrink(table);
				}
				return TRUE;
			}
		}
		np = (PIDTABLE*)np->next;
	}

	// pid not found
	return FALSE;
}


/* Function: shrink_pidtable
   Deallocate unused part of the table
*/
void pidtable_shrink(PIDTABLE *table)
{
	PIDTABLE *prev = table;
	PIDTABLE *np = table->next;
	while(np)
	{
		if (pidtable_getsize(np) == 0)
		{
			if (prev->size < PTABLE_LIMIT)
			{
				pidtable_free(np);
				prev->next = NULL;
				return;
			}
		}
		prev = np;
		np = (PIDTABLE*)np->next;
	}
}


/* Function: pidtable_print
   Print current process listed in the table
*/
void pidtable_print(PIDTABLE *table)
{
#ifdef DEBUG_PTABLE_INFO
	printf("Jobs[%d]\n", table->size);
#endif
	int i;

	// loop through table and print entries
	for (i = 0; i < PTABLE_SIZE; i++)
	{
		if (table->job[i] != NULL)
		{
			printf("[%d] ", 1 + i + (PTABLE_SIZE * table->offset));
			procgroup_print(table->job[i]);
		}
	}

	// recursive print
	if (table->next != NULL)
	{
		pidtable_print((PIDTABLE*)table->next);
	}
}


/* Function: getsize_pidtable
   Returns total # of entries in the table
*/
int pidtable_getsize(PIDTABLE *table)
{
	PIDTABLE *np = table;
	int count = 0;
	while(np)
	{
		count += np->size;
		np = np->next;
	}

	return count;
}


/* Function: pidtable_getindex
   Get procgroup by index
*/
PROCGROUP *pidtable_getindex(PIDTABLE *table, int n)
{
	// indexed from 1
	n--;

	// Check for boundary
	if (n < 0)
	{
		return NULL;
	}

	PIDTABLE *np = table;
	while(np)
	{
		if ( (np->offset+1) * PTABLE_SIZE <= n)
		{
			np = (PIDTABLE*)np->next;
		}
		else
		{
			return np->job[n - (np->offset * PTABLE_SIZE)];
		}
	}

	// procgroup not found
	return NULL;
}


/* Function: pidtable_getpid
   Get procgroup by process id
*/
PROCGROUP *pidtable_getpid(PIDTABLE *table, int pid)
{
	PIDTABLE *np = table;
	while(np)
	{
		int i = 0;
		for (; i < PTABLE_SIZE; i++)
		{
			if (np->job[i] != NULL && np->job[i]->group_pid == pid)
			{
				return (PROCGROUP*) np->job[i];
			}
		}
		np = (PIDTABLE*) np->next;
	}

	// pid not found
	return NULL;
}


int pidtable_getcapacity(PIDTABLE *table)
{
	PIDTABLE *np = table;
	int count = 0;
	while(np)
	{
		count++;
		np = np->next;
	}

	return count * PTABLE_SIZE;
}
