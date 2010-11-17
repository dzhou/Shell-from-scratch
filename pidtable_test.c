#include "pidtable.h"

/* tmp command */
#define CMD1 "sleep 10"
#define CMD2 "cat file | less"
#define CMD3 "wc < test.py"

#define PID_MAX 65000

/* prototypes */
void test_setup();
void test_free();
void test_add(int size);
void test_remove();
void test_size(int num_link, int num_size);

PIDTABLE *ptable;


/* Function: test_setup
   Setup for unittest. Creates the PIDTABLE
*/
void test_setup()
{
#ifdef DEBUG_TEST
	printf("TEST: PIDTABLE Initialized\n");
#endif

	ptable = pidtable_init();
	assert(ptable != NULL);
}


/* Function: test_destory
   Deallocate PIDTABLE
*/
void test_destroy()
{
#ifdef DEBUG_TEST
	printf("TEST: PIDTABLE deallocated\n");
#endif

	pidtable_free(ptable);
}


/* Function: test_add
   Test basic add function
*/
void test_add(int size)
{
#ifdef DEBUG_TEST
	printf("TEST: Adding %d elements to PIDTABLE\n", size);
#endif

	int i;
	srand(time(NULL));
	PROCGROUP *pg;
	char *cmd[3] = {CMD1, CMD2, CMD3};

	// add to table
	for (i = 0; i < size; i++)
	{
		pg = procgroup_init();
		procgroup_load(pg, i, RUNNING, cmd[rand()%3]);
		pidtable_add(ptable, pg);
	}
}



/* Function: test_remove
   Test basic remove, clears the table
*/
void test_remove()
{
#ifdef DEBUG_TEST
	printf("TEST: Removing PIDTABLE entries\n");
#endif

	int i;
	for (i = 0; i < PID_MAX + 2; i++)
	{
		while(pidtable_delpid(ptable, i, FALSE, TRUE) == TRUE) {}
	}
}


/* Function: test_size
   Test table size and capacity for correctness
*/
void test_size(int num_size, int capacity)
{
#ifdef DEBUG_TEST
	printf("TEST: num_size: %d == %d\n", pidtable_getsize(ptable), num_size);
	printf("TEST: capacity: %d == %d\n", pidtable_getcapacity(ptable), capacity);
#endif
	assert(pidtable_getsize(ptable) == num_size);
	assert(pidtable_getcapacity(ptable) == capacity);
}


/* Function: test_get
   Test get procgroup by PID
*/
void test_get(int size)
{
	int i;
	srand(time(NULL));
	PROCGROUP *pg, *pg1, *pg2;
	char *cmd[3] = {CMD1, CMD2, CMD3};

	// add a procgroup to table
	for (i = 1; i < size; i++)
	{
		pg = procgroup_init();
		procgroup_load(pg, i, RUNNING, cmd[rand()%3]);
		pidtable_add(ptable, pg);
	}
	// get procgroup
	for (i = 1; i < size; i++)
	{
		pg1 = pidtable_getpid(ptable, i);
		pg2 = pidtable_getindex(ptable, i);
		assert(pg1 != NULL);
		assert(pg2 != NULL);
		assert(pg1->group_pid == i);
		assert(pg2->group_pid == i);
	}
}


/* Run tests */
int main()
{
#ifdef DEBUG_TEST
	printf("Begin Unittest: PIDTABLE Module\n");
#endif

	test_setup();

	int i, n = 5;
	int count[] = {0, 1, 64, 625, 1024};

	// Loop for testing add/del/getsize
	for (i = 0; i < n; i++)
	{
		// test add/remove
		test_add(count[i]);
		test_size(count[i], (count[i]/PTABLE_SIZE+1)*PTABLE_SIZE);
		test_remove();
		test_size(0, PTABLE_SIZE);

		// Test get function
		test_get(count[i]);
		test_remove();
		test_size(0, PTABLE_SIZE);
	}

	test_destroy();

#ifdef DEBUG_TEST
	printf("End Unittest: PIDTABLE Module\n");
#endif

	return 0;
}
