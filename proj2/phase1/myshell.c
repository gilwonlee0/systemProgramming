#include "csapp.h"
#include <errno.h>

#define MAXARGS   128
#define MAXBINPATH   512

void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

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
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int *iptr = malloc(sizeof(int));
	pid_t pid;           /* Process id */

    strcpy(buf, cmdline);

	// TODO: Support pipelining deliemeter '|' in phase2
    parseline(buf, argv);
    if (argv[0] == NULL) return;   /* Ignore empty lines */

	// TODO: Proocess forking logic will be functionalized, in phase2 so to support pipelining
	if (!builtin_command(argv)) {
		if ((pid = Fork()) == 0) {
			if (execvp(argv[0], argv) < 0) {	//ex) ls -al &
	            printf("%s: Command not found.\n", argv[0]);
	            exit(0);
		    }
		}

		// TODO: Implement not to wait for bg command in phase3
		// Parent, which is main shell
		Waitpid(pid, iptr, WCONTINUED);
	}

    return;
}
/* $end eval */

// Run a shell built-in commands, such as exit, cd - Specification 2.1
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "&")) return 1;  /* Ignore singleton & */
    if (!strcmp(argv[0], "exit")) exit(0);
	if (!strcmp(argv[0], "cd")) {
		if (!strcmp(argv[0], "cd")) {
			// Manually route to home directory without any argument
			if (argv[1] == NULL) argv[1] = getenv("HOME");
			if (chdir(argv[1]) == -1) printf("%s: No such file or directory.\n", argv[1]);
			return 1;
		}
    }
    return 0;                     /* Not a builtin command */
}

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv)
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) buf++;  /* Ignore spaces */
    }
    argv[argc] = NULL;

    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */
