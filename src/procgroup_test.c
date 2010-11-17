#include "procgroup.h"

/* tmp command */
#define CMD1 "sleep 10"
#define CMD2 "cat file | less"
#define CMD3 "wc < test.py"


PROCGROUP *pg;

/* prototypes */
void test_setup();
void test_destroy();
void test_load();


/* Function: test_setup
   Test procgroup initialization
*/
void test_setup()
{
#ifdef DEBUG_TEST
	printf("TEST: PROCGROUP Initialized\n");
#endif

	pg = (PROCGROUP*) procgroup_init();
	assert(pg != NULL);
}


/* Function: test_destroy
   Test procgroup deinit
*/
void test_destroy()
{
#ifdef DEBUG_TEST
	printf("TEST: PROCGROUP Deinitialized\n");
#endif

	procgroup_free(pg);
}


/* Function: test_load
   Testing load/clear functions
*/
void test_load()
{

#ifdef DEBUG_TEST
	printf("TEST: PROCGROUP Load/Clear\n");
#endif

	int pid, stat;

	/* test load */
	pid = 512; stat = 4;
	procgroup_load(pg, pid, stat, CMD1);
	assert(pg->group_pid == pid);
	assert(pg->status == stat);
	assert(strncmp(pg->cmdline, CMD1, strlen(CMD1)) == 0);

	/* test clear */
	procgroup_clear(pg);
	assert(pg->group_pid == 0);
	assert(pg->status == 0);
	assert(pg->cmdline[0] == '\0');

	/* test reload */
	pid = 6123; stat = 10;
	procgroup_load(pg, pid, stat, CMD2);
	assert(pg->group_pid == pid);
	assert(pg->status == stat);
	assert(strncmp(pg->cmdline, CMD2, strlen(CMD2)) == 0);
}


/* Run tests */
int main()
{

#ifdef DEBUG_TEST
	printf("Begin Unittest: PROCGROUP Modules\n");
#endif 

	test_setup();
	test_load();
	test_destroy();

#ifdef DEBUG_TEST
	printf("End Unittest: PROCGROUP Modules\n");
#endif 

	return 0;
}
