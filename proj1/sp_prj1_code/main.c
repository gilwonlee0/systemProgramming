#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "list.h"
#include "hash.h"
#include "bitmap.h"

#define MAX_BUFFER_SIZE 1024
#define MAX_ARG_SIZE 32

bool QUIT = false;

void shell_loop();
void execute_command_from_buffer(char* buffer);

int main(int argc, char* argv[]) {
	shell_loop();

	return 0;
}

void shell_loop() {
	char buffer[MAX_BUFFER_SIZE];

	while (!QUIT) {
		fgets(buffer, MAX_BUFFER_SIZE, stdin);
		printf("%s\n", buffer);
		execute_command_from_buffer(buffer);
	}

	return;
}

void execute_command_from_buffer(char* buffer) {
	// Parse the buffer
	char cmd[MAX_ARG_SIZE], arg1[MAX_ARG_SIZE], arg2[MAX_ARG_SIZE], arg3[MAX_ARG_SIZE];
	sscanf(buffer, "%s %s %s %s", cmd, arg1, arg2, arg3);

	printf("cmd: %s\n", cmd);
	printf("arg1: %s\n", arg1);
	printf("arg2: %s\n", arg2);
	printf("arg3: %s\n", arg3);

	if (strncmp(cmd, "quit", 4) == 0) QUIT = true;  // The only condition to quit the program normally
	else if (strncmp(cmd, "create", 6) == 0) {
		if (strncmp(arg1, "list", 4) == 0) {}
		else if (strncmp(arg1, "hashtable", 9) == 0) {}
		else if (strncmp(arg1, "bitmap", 6) == 0) {}
	}
	else if (strncmp(cmd, "delete", 6) == 0) {
		if (strncmp(arg1, "list", 4) == 0) {}
		else if (strncmp(arg1, "hashtable", 9)) {}
		else if (strncmp(arg1, "bitmap", 6)) {}
	}
	else if (strncmp(cmd, "dumpdata", 8) == 0) {
		if (strncmp(arg1, "list", 4) == 0) {}
		else if (strncmp(arg1, "hashtable", 9)) {}
		else if (strncmp(arg1, "bitmap", 6)) {}
	}
	else if (strncmp(cmd, "list_", 5) == 0) {}
	else if (strncmp(cmd, "hash_", 5) == 0) {}
	else if (strncmp(cmd, "bitmap_", 7) == 0) {}
	else {
		printf("Unknown command: %s\n", buffer);
		exit(1);
	}

	return;
}