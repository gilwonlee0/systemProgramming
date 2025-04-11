#include "csapp.h"
#include <errno.h>
#include <stdbool.h>

#define MAXARGS   128
#define MAXBINPATH   512
#define MAXPIPES 10

typedef struct Command {
	char* argv[MAXARGS];
	int argc;
	bool background;
} Command;

typedef struct Pipeline {
	Command commands[MAXPIPES];
	int cmd_count;
	bool background;
} Pipeline;

void eval(char *cmdline);
void parse_single_command(char *buf, Command* cmd);
int builtin_command(char **argv);
Pipeline* parse_cmdline(char *cmdline);


int main()
{
	char cmdline[MAXLINE];

    while (1) {
		printf("CSE4100-SP-P2> ");
		/* Read */
		fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin))
		    exit(0);

		/* Evaluate */
		eval(cmdline);
    }
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
{
    char buf[MAXLINE];   /* Holds modified command line */
    int *iptr = malloc(sizeof(int));
	pid_t pid = 0;           /* Process id */
	int pipes[MAXPIPES][2];

    strcpy(buf, cmdline);

	Pipeline* pipeline = parse_cmdline(buf);

	// Skip empty lines
    if (pipeline->cmd_count == 0) {
    	free(pipeline);
	    return;
    }

	// Init pipes
	for (int i = 0; i < pipeline->cmd_count - 1; i++) {
		if (pipe(pipes[i]) < 0) {
			perror("pipe error");
			exit(1);
		}
	}

	// Run command
	for (int i = 0; i < pipeline->cmd_count; i++) {
		if (!builtin_command(pipeline->commands[i].argv)) {
			if ((pid = Fork()) == 0) {
				if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO);
				if (i < pipeline->cmd_count - 1) dup2(pipes[i][1], STDOUT_FILENO);

				for (int j = 0; j < pipeline->cmd_count - 1; j++) {
					close(pipes[j][0]);
					close(pipes[j][1]);
				}

				if (execvp(pipeline->commands[i].argv[0], pipeline->commands[i].argv) < 0) {
		            printf("%s: Command not found.\n", pipeline->commands[i].argv[0]);
		            exit(0);
			    }
			}
		}
	}

	// Parent process clean-ups
	for (int i = 0; i < pipeline->cmd_count - 1; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

	if (!pipeline->background) {
		Waitpid(pid, iptr, 0);
		free(pipeline);

		// for (int i = 0; i < pipeline->cmd_count - 1; i++) {
		// 	wait(NULL);
		// }
	}
	else printf("[%d] %s ... &\n", pid, pipeline->commands[0].argv[0]);

}
/* $end eval */

// Run a shell built-in commands, such as exit, cd - Specification 2.1
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "&")) return 1;  /* Ignore singleton & */
    if (!strcmp(argv[0], "exit")) exit(0);
	if (!strcmp(argv[0], "cd")) {
		// Manually route to home directory without any argument
		if (argv[1] == NULL || !strcmp(argv[1], "~")) argv[1] = getenv("HOME");
		if (chdir(argv[1]) == -1) printf("%s: No such file or directory.\n", argv[1]);
		return 1;
	}
    return 0;  /* Not a builtin command */
}

Pipeline* parse_cmdline(char* cmdline) {
	Pipeline* pipeline = malloc(sizeof(Pipeline));
	pipeline->cmd_count = 0;

	size_t len = strlen(cmdline);

	// Remove new line
	if (len > 0 && cmdline[len-1] == '\n') cmdline[len-1] = '\0';

	char* cmd_str = strtok(cmdline, "|");
	while (cmd_str != NULL && pipeline->cmd_count < MAXPIPES) {
		Command* cmd = &pipeline->commands[pipeline->cmd_count];
		parse_single_command(cmd_str, cmd);

		if (cmd->argv[0] != NULL) pipeline->cmd_count++;
		if (cmd->background) pipeline->background = true;

		cmd_str = strtok(NULL, "|");
	}

	return pipeline;
}

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
void parse_single_command(char *buf, Command* cmd)
{
    int argc;            /* Number of args */
	while (*buf && (*buf == ' ')) buf++;  /* Ignore leading spaces */

    /* Build the argv list */
    argc = 0;
    while (*buf) {
		char* start = buf;

    	// Parse quoted string
		if (*buf == '"' || *buf == '\'') {
			char quote = *buf;
			start = ++buf;

			while (*buf && *buf != quote) buf++;

			if (*buf == quote) {
				*buf = '\0';
				buf++;
			}
		}
    	// Parse unquoted string
		else {
			while (*buf && *buf != ' ') buf++;

			if (*buf == ' ') {
				*buf = '\0';
				buf++;
			}
		}

    	cmd->argv[argc++] = strdup(start);

    	while (*buf && (*buf == ' ')) buf++;  /* Ignore spaces */
    }

	cmd->background = false;
	if (cmd->argv[argc-1][0] == '&') {
		argc--;
		cmd->background = true;
	}

	cmd->argv[argc] = NULL;
	cmd->argc = argc;
}
/* $end parseline */
