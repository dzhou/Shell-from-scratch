#include "parser.h"

#define CMD_MAX 1<<6

void test_standard();
void test_redirect();
void test_pipe();
void direct_input();


COMMAND *cmd;

/* Function: test_standard
   Several standard command
*/
void test_standard()
{
#ifdef DEBUG_TEST
	printf("TEST: Checking standard command\n");
#endif

	cmd = command_parse("cat testfile1 testfile2\n");
	assert(cmd != NULL);
	assert(cmd->token >= 3);
	assert(strcmp(cmd->argv[0], "cat") == 0);
	assert(strcmp(cmd->argv[1], "testfile1") == 0);
	assert(strcmp(cmd->argv[2], "testfile2") == 0);
	command_free(cmd);

	cmd = command_parse("echo line&\n");
	assert(cmd != NULL);
	assert(cmd->token >= 2);
	assert(cmd->background == TRUE);
	assert(strcmp(cmd->argv[0], "echo") == 0);
	assert(strcmp(cmd->argv[1], "line") == 0);
	command_free(cmd);
}


/* Function: test_redirect
   Test commands with io redirect
*/
void test_redirect()
{
#ifdef DEBUG_TEST
	printf("TEST: Checking I/O redirecting\n");
#endif

	cmd = command_parse("a < b > c\n");
	assert(cmd != NULL);
	assert(cmd->token >= 1);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->infile, "b") == 0);
	assert(strcmp(cmd->outfile, "c") == 0);
	assert(cmd->fdmode == (O_RDWR|O_TRUNC));
	command_free(cmd);

	cmd = command_parse("a<b>> c\n");
	assert(cmd != NULL);
	assert(cmd->token >= 1);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->infile, "b") == 0);
	assert(strcmp(cmd->outfile, "c") == 0);
	assert(cmd->fdmode == (O_RDWR|O_APPEND));
	command_free(cmd);

	cmd = command_parse("a<b>c\n");
	assert(cmd != NULL);
	assert(cmd->token >= 1);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->infile, "b") == 0);
	assert(strcmp(cmd->outfile, "c") == 0);
	assert(cmd->fdmode == (O_RDWR|O_TRUNC));
	command_free(cmd);

	cmd = command_parse("a <b >>c\n");
	assert(cmd != NULL);
	assert(cmd->token >= 1);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->infile, "b") == 0);
	assert(strcmp(cmd->outfile, "c") == 0);
	assert(cmd->fdmode == (O_RDWR|O_APPEND));
	command_free(cmd);
}


/* Function: test_pipe
   Test command with pipes
*/
void test_pipe()
{
#ifdef DEBUG_TEST
	printf("TEST: Checking pipes\n");
#endif

	cmd = command_parse("a | b | c \n");
	assert(cmd != NULL);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->next->argv[0], "b") == 0);
	assert(strcmp(cmd->next->next->argv[0], "c") == 0);
	command_free(cmd);

	cmd = command_parse("a| b |c \n");
	assert(cmd != NULL);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->next->argv[0], "b") == 0);
	assert(strcmp(cmd->next->next->argv[0], "c") == 0);
	command_free(cmd);

	cmd = command_parse("a|b|c \n");
	assert(cmd != NULL);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->next->argv[0], "b") == 0);
	assert(strcmp(cmd->next->next->argv[0], "c") == 0);
	command_free(cmd);

	cmd = command_parse("a<in|b|c > out &\n");
	assert(cmd != NULL);
	assert(strcmp(cmd->argv[0], "a") == 0);
	assert(strcmp(cmd->infile, "in") == 0);
	assert(strcmp(cmd->next->argv[0], "b") == 0);
	assert(strcmp(cmd->next->next->argv[0], "c") == 0);
	assert(strcmp(cmd->next->next->outfile, "out") == 0);
	assert(cmd->background == TRUE);
	assert(cmd->next->background == TRUE);
	assert(cmd->next->next->background == TRUE);
	command_free(cmd);

}


/* Function: direct_input
*/
void direct_input()
{
#ifdef DEBUG_TEST
	printf("TEST: Read and parse command from STDIN\n");
#endif
	char buffer[CMD_MAX];
	while(TRUE)
	{
		printf("# ");
		fgets(buffer, CMD_MAX, stdin);
		cmd = command_parse(buffer);
		command_print(cmd);
		command_free(cmd);
	}
}


/* Run tests */
int main()
{
	test_standard();
	test_redirect();
	test_pipe();
//	direct_input();

	return 0;
}
