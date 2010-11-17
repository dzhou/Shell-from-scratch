/*
	PIDTABLE is an unrolled linked list for storing a list of running processes in the background.
	Each node of the pidtable contains an array of fixed size (specified by PTABLE_SIZE).
	At initialization, the pidtable contains a single node. New nodes are automatically created
	when the current capacity is full, and empty nodes are deallocated with regards certain
	threshold level.

	Main functions:
		add: insert a PROCGROUP struct into the first available stop in the array
		delete: delete (and/or deallocate) a PROCGROUP from the table
		get by pid: returns PROCGROUP with the matching group pid
		get by index: returns the nth PROCGROUP in the array
*/

#ifndef _PIDTABLE_H_
#define _PIDTABLE_H_

#include "include.h"
#include "procgroup.h"

#define JOB_KILLED 32
#define JOB_EXITED 12

/* Internal Array size
   Table capacity is #node * PTABLE_SIZE
*/
#define PTABLE_SIZE 100

/* current node size limit for deleting next empty node
   must be < PTABLE_SIZE
*/
#define PTABLE_LIMIT 50

/* Typedef: PIDTABLE
   Unrolled linkedlist structure for storing background process info
   Each node stores up to PTABLE_SIZE elements. New node is created and added to
   the end when the current table is full.
   All values are set to zero or null at initialization
*/
typedef struct pidtable{
	int offset;
	int size;
	PROCGROUP *job[PTABLE_SIZE];
	struct pidtable *next;
} PIDTABLE;


/* Function: init_pidtable
   Pidtable constructor
   Returns dynamically allocated PIDTABLE with a single node. The returned pointer
   must be passed to free_pidtable() to be deallocated at the end.
*/
PIDTABLE *pidtable_init();


/* Function: free_pidtable
   Recusively deallocate each node in the table
   Precondition: *table is a valid pointer to a PIDTABLE created by init_pidtable()
*/
void pidtable_free(PIDTABLE *table);


/* Function: add_pidtable
   Add a new process to the table. If the current table is full, a new node is
   created at the end.
   Precondition: *table is a valid pointer to a PIDTABLE
   Parameter:
		pg - a valid procgroup struct
   Returns the table index of the added process
*/
int pidtable_add(PIDTABLE *table, PROCGROUP *pg);


/* Function: del_pidtable
   Find pid in the table and delete the cooresponding entry. Function will iterate through
   the current node and any node afterwards. If the last node is empty and the second to last
   node's size is below PTABLE_LIMIT, then the last node is destroyed.
   Precondition: *table is a valid pointer to a PIDTABLE
   Returns TRUE is delete is successful, returns FALSE if unsuccessful or pid could
   not be found in the table.
   Parameter:
		pid 	- group pid of PROCGROUP
		echo 	- print deleted PROCGROUP on success
		free 	- free PROCGROUP struct if TRUE
*/
int pidtable_delpid(PIDTABLE *table, int pid, int echo, int free);


/* Function: print_pidtable
   Print the current table. In non-debug mode, print lists all non-empty entries in the table
   Precondition: assume process entries are valid, ie pointer to command is not null
   Precondition: *table is a valid pointer to a PIDTABLE
*/
void pidtable_print(PIDTABLE *table);


/* Function: shrink_pidtable (internal)
   Deallocate unused part of the table. This is called by del_pidtable to dynamically shrink the
   table when entries are deleted.
   Precondition: *table is a valid pointer to a PIDTABLE
*/
void pidtable_shrink(PIDTABLE *table);


/* Function: getsize_pidtable
   Returns total # of entries in the table
   Precondition: *table is a valid pointer to a PIDTABLE
*/
int pidtable_getsize(PIDTABLE *table);


/* Function: pidtable_getindex
   Retrieve the PROCGROUP in the nth spot in the array. Index begins at 1.
   Returns procgroup struct. If index is out of bound or the array location is empty, returns NULL
   Precondition: *table is a valid pointer to a PIDTABLE
*/
PROCGROUP *pidtable_getindex(PIDTABLE *table, int n);


/* Function: pidtable_getpid
   Find and return the procgroup with group_id pid.
   Returns procgroup struct. If pid is not found, returns NULL
   Precondition: *table is a valid pointer to a PIDTABLE
*/
PROCGROUP *pidtable_getpid(PIDTABLE *table, int pid);


/* Function: pidtable_getcapacity
   Returns the current table capacity
   Calculated as PTABLE_SIZE * # of nodes
   Precondition: *table is a valid pointer to a PIDTABLE
*/
int pidtable_getcapacity(PIDTABLE *table);

#endif /* _PIDTABLE_H_ */

