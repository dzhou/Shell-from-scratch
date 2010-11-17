#include "parser.h"


/* Function: command_print
   Recursive print function for command struct
*/
void command_print(const COMMAND *cmd)
{
	printf("COMMAND [B%d][P%d][A%d] {\n", cmd->background, cmd->pipe, cmd->fdmode);
	printf("   {%s}\n", cmd->cmdline);
	if (cmd->infile != NULL)
	{
		printf("  <[%s]\n", cmd->infile);
	}
	if (cmd->outfile != NULL)
	{
		printf("  >[%s]\n", cmd->outfile);
	}
	int i;
	for (i = 0; i < cmd->token; i++)
	{
		printf("  [%d]: %s\n", i, cmd->argv[i]);
	}
	printf("}\n");
	if (cmd->next != NULL)
	{
		command_print((COMMAND*)cmd->next);
	}
}


/* Function: command_free
   Recursive deallocate command struct memory
   Free buffer, argv, and command struct
*/
void command_free(COMMAND *cmd)
{
	if (cmd->next != NULL)
	{
		command_free((COMMAND*)cmd->next);
	}
	if (cmd->buffer != NULL)
	{
		free(cmd->buffer);
	}
	if (cmd->argv != NULL)
	{
		free(cmd->argv);
	}
	if (cmd->cmdline != NULL)
	{
		free(cmd->cmdline);
	}
	if (cmd != NULL)
	{
		free(cmd);
	}
#ifdef DEBUG_PARSER_INFO
	printf("PARSER: Command struct deallocated\n");
#endif
}


/* Function: command_parse
   Main command parser (recusive)
   Returns the first command in the list
*/
COMMAND* command_parse(const char *buffer)
{
	int i = 0, j = 0, count = 0;
	int finished = TRUE;

	// removes leading space
	while (i < strlen(buffer) && isspace(buffer[i]))
	{
		i++;
	}

	// find command length, support command separators: ';' and '|'
	for (; i < strlen(buffer); i++)
	{
		if ((buffer[i] == ';' || buffer[i] == '\n') && buffer[i-1] != '\\')
		{
			break;
		}
	}
	int buf_len = i;

#ifdef DEBUG_PARSER_PROCESS
	printf("PARSER: Parsing command length of %d\n", buf_len);
#endif

	// initialize command struct and reset all values
	COMMAND *cmd;
	cmd = (COMMAND*) malloc(sizeof (COMMAND));

	// Check for malloc failure
	if (cmd == NULL)
	{
#ifdef WARNING
	printf("ERROR: Could not allocate %d bytes for command struct\n", sizeof (COMMAND));
#endif
		goto parser_exit;
	}

	cmd->buffer = malloc(2 * buf_len);
	memset(cmd->buffer, '\0', 2 * buf_len);
	cmd->background = FALSE;
	cmd->pipe = FALSE;
	cmd->next = NULL;
	cmd->infile = NULL;
	cmd->outfile = NULL;
	cmd->fdmode = O_RDONLY;

	cmd->cmdline = malloc(buf_len + 2);
	memset(cmd->cmdline, '\0', buf_len + 2);

	// Check for malloc failure
	if (cmd->cmdline == NULL)
	{
#ifdef WARNING
	printf("ERROR: Could not allocated %d bytes for command line parsing\n", buf_len + 1);
#endif
		goto parser_exit;
	}
	// Copies in original command line
	strncpy(cmd->cmdline, buffer, buf_len);

	// Check for malloc failure
	if (cmd->buffer == NULL)
	{
#ifdef WARNING
	printf("ERROR: Could not allocated %d bytes for command line parsing\n", 2 * buf_len);
#endif
		goto parser_exit;
	}
	else
	{
#ifdef DEBUG_PARSER_PROCESS
	printf("PARSER: Allocated %d bytes for command line parsing\n", 2 * buf_len);
	printf("PARSER: Allocated %d bytes for command struct\n", sizeof (COMMAND));
#endif
	}

	i = 0;

	// removes leading space
	while (i < strlen(buffer) && isspace(buffer[i]))
	{
		i++;
	}


	// begin main parsing loop, exit at null terminator
	while (TRUE)
	{
		// support for escape sequence

/*
		if (buffer[i] == '\\')
		{
			cmd->buffer[j] = buffer[i+1];
			buffer[i+1] = 'X'; //shortcut
			i+=2; j++;
			continue;
		}
*/
		// copy in first whitespace
		if (isspace(buffer[i]))
		{
			cmd->buffer[j] = '\0';
			//i++; j++; count++;
			count++; j++;
		}

		// remove consecutive whitespace
		while(isspace(buffer[i]))
		{
			i++;
		}

		// new command
		if (buffer[i] == ';')
		{
			finished = FALSE;
			i++; count++;
			goto parser_finalize;
		}

		// set background task, end current command
		if (buffer[i] == '&')
		{
			cmd->background = TRUE;
			finished = FALSE;
			i++; count++;
			goto parser_finalize;
		}

		// pipe, end current command
		if (buffer[i] == '|')
		{
			cmd->pipe = TRUE;
			finished = FALSE;
			if (isspace(buffer[i-1]))
			{
				count--;
			}
			i++; count++;
			goto parser_finalize;
		}

		// set input redirect
		if (buffer[i] == '<' || buffer[i] == '>')
		{
			int width = 1;
			if (buffer[i] == '>')
			{
				cmd->fdmode = O_RDWR|O_TRUNC;
				if (buffer[i+1] == '>')
				{
					cmd->fdmode = O_RDWR|O_APPEND;
					width = 2;
					i++;
				}
			}
			if (!isspace(buffer[i-width]))
			{
				cmd->buffer[j++] = '\0';
				count++;
			}
			cmd->buffer[j++] = buffer[i];
			if (!isspace(buffer[i+1]))
			{
				cmd->buffer[j++] = '\0';
				count++;
			}
			i++;
			continue;
		}

		cmd->buffer[j] = buffer[i];

		if (buffer[i] == '\0')
		{
			break;
		}

		i++; j++;
	}

// created pointers to parsed tokens
parser_finalize:
	cmd->buffer[j] = '\0';
	cmd->token = count + 1;
	cmd->argv = calloc(sizeof(char*), cmd->token);

	int k = 0, h = 0, len = 0;
	for (j = k = h = 0; h < count; h++)
	{
		if ((len=strlen(&cmd->buffer[j])) > 0)
		{
			// support IO redirect
			if (cmd->buffer[j] == '<')
			{
				cmd->infile = &cmd->buffer[j+2];
				j += strlen(&cmd->buffer[j+2]) + 3;
				continue;
			}
			if (cmd->buffer[j] == '>')
			{
				cmd->outfile = &cmd->buffer[j+2];
				j += strlen(&cmd->buffer[j+2]) + 3;
				continue;
			}

			cmd->argv[k++] = &cmd->buffer[j];
			j += len+1;
		}
	}

	// There are more command, begin recursive parse
	if (finished == FALSE)
	{
#ifdef DEBUG_PARSER_PROCESS
	printf("PARSER: Begin parsing next command\n");
#endif
		cmd->next = (struct command*) command_parse(&buffer[i]);
		// Set background if next command goes to background
		if (cmd->next->background == TRUE)
		{
			cmd->background = TRUE;
		}
		// remove empty trailing struct
		if (cmd->next->token < 2)
		{
			command_free(cmd->next);
			cmd->next = NULL;
		}
	}
	// Done
	else
	{
#ifdef DEBUG_PARSER_INFO
	printf("PARSER: Command struct allocated\n");
#endif
	}

parser_exit:
	return cmd;
}
