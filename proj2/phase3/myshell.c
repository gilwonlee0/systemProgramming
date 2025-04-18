#include "csapp.h"
#include <errno.h>
#include <stdbool.h>

#define MAXARGS 128
#define MAXPIPES 10
#define MAXJOBS 50
#define MAXCHILDPROCESS 10


typedef enum {
	RUNNING,
	STOPPED,
	TERMINATED,
} JobState;

const char* JobState_str[] = {
	"Running",
	"Stopped",
	"Terminated"
};

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

typedef struct Job{
	pid_t pgid;
	pid_t pids[MAXCHILDPROCESS];
	char* cmdline;
	Pipeline* pipeline;
	JobState state;
} Job;

void eval(char *cmdline);
void parse_single_command(char *buf, Command* cmd);
int builtin_command(char **argv);
Pipeline* parse_cmdline(char *cmdline);

// Jobs related functions
int add_job(pid_t pid, pid_t pids[], char* cmdline, Pipeline* pipeline, JobState state);
int update_job(int job_id, JobState state);
int get_job_id_by_pid(pid_t pid);
int get_job_id_by_pgid(pid_t pgid);

// Builtin functions
void builtin_jobs();
void builtin_fg(char* arg);
void builtin_bg(char* arg);
void builtin_kill(char* arg);

// Signal handlers
void sigtstp_handler(int sig);
void sigchld_handler(int sig);

// Common variables used by shell and child processes.
Job* jobs[MAXJOBS];
pid_t shell_pgid;


int main()
{
	shell_pgid = getpgid(getpid());
	setpgid(0, 0);
	tcsetpgrp(STDIN_FILENO, shell_pgid);

	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	signal(SIGCHLD, sigchld_handler);

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
	// SIGCHLD mask
	sigset_t mask, prev_mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);

	char buf[MAXLINE];   /* Holds modified command line */
    int *iptr = malloc(sizeof(int));
	int pipes[MAXPIPES][2];

	pid_t pid = 0;
	pid_t pgid = 0;
	pid_t pids[MAXCHILDPROCESS] = {-1,};

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

	// Block sigchld before forking child processes.
	sigprocmask(SIG_BLOCK, &mask, &prev_mask);

	// Run command
	for (int i = 0; i < pipeline->cmd_count; i++) {
		if (!builtin_command(pipeline->commands[i].argv)) {
			if ((pid = Fork()) == 0) {
				signal(SIGINT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				signal(SIGTTOU, SIG_DFL);

				// Set as a same process group
				if (pgid == 0) pgid = getpid();
				setpgid(0, pgid);

				if (!pipeline->background) tcsetpgrp(STDIN_FILENO, pgid);

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
			else {
				if (pgid == 0) pgid = pid;
				setpgid(pid, pgid);

				pids[i] = pid;

				if (!pipeline->background) tcsetpgrp(STDIN_FILENO, pgid);
			}
		}
	}

	// Parent process clean-ups
	for (int i = 0; i < pipeline->cmd_count - 1; i++) {
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

	if (!pipeline->background) {
		int status;
		pid_t w;
		while ((w = waitpid(-pgid, &status, WUNTRACED)) > 0) {
			if (WIFSTOPPED(status)) {
				add_job(pgid, pids, cmdline, pipeline, STOPPED);
				break;
			}
		}

		sigprocmask(SIG_SETMASK, &prev_mask, NULL);
		tcsetpgrp(STDIN_FILENO, shell_pgid);

		free(pipeline);
	}
	else {
		int job_id = add_job(pgid, pids, cmdline, pipeline, RUNNING);
		printf("[%d] [%d]\n", job_id, pgid);

		sigprocmask(SIG_SETMASK, &prev_mask, NULL);
	}

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
	if (!strcmp(argv[0], "jobs")) {
		builtin_jobs();
		return 1;
	}
	if (!strcmp(argv[0], "bg")) {
		builtin_bg(argv[1]);
		return 1;
	}
	if (!strcmp(argv[0], "fg")) {
		builtin_fg(argv[1]);
		return 1;
	}
	if (!strcmp(argv[0], "kill")) {
		builtin_kill(argv[1]);
		return 1;
	}
    return 0;  /* Not a builtin command */
}

Pipeline* parse_cmdline(char* cmdline) {
	Pipeline* pipeline = malloc(sizeof(Pipeline));
	pipeline->cmd_count = 0;
	pipeline->background = false;

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


int add_job(pid_t pgid, pid_t pids[], char* cmdline, Pipeline* pipeline, JobState state){
	int job_id = -1;
	for (int i = 1; i < MAXJOBS; i++) {
		if (jobs[i] == NULL) {
			jobs[i] = malloc(sizeof(Job));

			jobs[i]->pgid = pgid;
			jobs[i]->state = state;
			jobs[i]->pipeline = pipeline;
			jobs[i]->cmdline = malloc(sizeof(char) * strlen(cmdline) + 1);
			strncpy(jobs[i]->cmdline, cmdline, strlen(cmdline));
			int k = 0;
			for (int j = 0; j < pipeline->cmd_count; j++) if (pids[j] != -1) jobs[i]->pids[k++] = pids[j];

			job_id = i;
			break;
		}
	}
	return job_id;
}


void sigchld_handler(int sig) {
	int status;
	pid_t pid, _pid;

	while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED )) > 0) {
		int job_id = get_job_id_by_pid(pid);
		if (job_id == -1) continue;

		if (WIFEXITED(status) || WIFSIGNALED(status)) jobs[job_id]->state = TERMINATED;
		else if (WIFSTOPPED(status)) {
			jobs[job_id]->state = STOPPED;
			tcsetpgrp(STDIN_FILENO, shell_pgid);
		}
		else if (WIFCONTINUED(status)) jobs[job_id]->state = RUNNING;
	}

	return;
}

int get_job_id_by_pid(pid_t pid) {
	for (int job_id = 1; job_id < MAXJOBS; job_id++) {
		Job *job = jobs[job_id];
		if (job == NULL) continue;
		if (job->state == RUNNING) {
			for (int j = 0; j < MAXCHILDPROCESS; j++)
				if (pid == job->pids[j]) return job_id;
		}
	}

	return -1;
}

int get_job_id_by_pgid(pid_t pgid) {
	for (int job_id = 1; job_id < MAXJOBS; job_id++) {
		Job *job = jobs[job_id];
		if (job == NULL) continue;
		if (pgid == job->pgid) return job_id;
	}

	return -1;
}


void builtin_jobs(){
	for (int i = 1; i < MAXJOBS; i++) {
		if (jobs[i] == NULL) continue;
		printf("[%d]  %s\t\t\t%s", i, JobState_str[jobs[i]->state], jobs[i]->cmdline);
	}
}

void builtin_fg(char* arg) {
	if (arg == NULL) {
		printf("fg: %s: No such job\n", arg);
		return;
	}

    int job_id = atoi(arg);
	Job* job = jobs[job_id];

	if (job == NULL || job->state == TERMINATED) printf("fg: %s: No such job\n", arg);
	else {
		tcsetpgrp(STDIN_FILENO, job->pgid);

		job->state = RUNNING;
		kill(-job->pgid, SIGCONT);

		printf("%s", job->cmdline);

		waitpid(-job->pgid, NULL, WUNTRACED);
		job->state = TERMINATED;

		tcsetpgrp(STDIN_FILENO, shell_pgid);
	}

	return;
}

void builtin_bg(char* arg) {
	if (arg == NULL) {
		printf("bg: %s: No such job\n", arg);
		return;
	}

	int job_id = atoi(arg);
	Job* job = jobs[job_id];

	if (job == NULL || job->state == TERMINATED) printf("bg: %s: No such job\n", arg);
	else {
		kill(-job->pgid, SIGCONT);
		job->state = RUNNING;
		tcsetpgrp(STDIN_FILENO, shell_pgid);
	}
	return;
}

void builtin_kill(char* arg) {
	if (arg == NULL || arg[0] != '%') {
		printf("kill: usage: kill %[job_id]\n");
		return;
	}

    int job_id = atoi(arg + 1);
	Job* job = jobs[job_id];

	if (job == NULL || job->state == TERMINATED) printf("kill: %s: No such job\n", arg);
	else {
		kill(-job->pgid, SIGINT);
		job->state = TERMINATED;
	}

	return;
}