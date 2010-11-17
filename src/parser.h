#ifndef _PARSER_H_
#define _PARSER_H_

#include "include.h"


/* Typedef: COMMAND
   Basic struct for storing command
   buffer is dynamically allocated
   argv is an array of pointers
*/
typedef struct command
{
	char **argv;
	char *buffer;
	char *cmdline;
	char *infile;
	char *outfile;
	short token;
	short background;
	short pipe;
	short fdmode;
	struct command *next;
} COMMAND;


/* Function: command_print
   Debugging function. Recusively print out command
   Precondition: cmd is a valid pointer to COMMAND returned by parse_command()
*/
void command_print(const COMMAND *cmd);


/* Function: command_free
   Recursively deallocate COMMAND
   Precondition: cmd is a valid pointer to COMMAND returned by parse_command()
*/
void command_free(COMMAND *cmd);


/* Function: command_parse
   Parse buffer to create one or more COMMAND
   Returns the first command to be executed
*/
COMMAND* command_parse(const char *buffer);

#endif /* _PARSER_H_ */
