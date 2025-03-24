#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "list.h"
#include "hash.h"
#include "bitmap.h"

#define MAX_BUFFER_SIZE 1024
#define MAX_ARG_SIZE 32

// Maximum number of data structures (assumption #6)
#define MAX_STRUCTURE_COUNT 11

// A variable that determines to stop the program
bool QUIT = false;

// Simple data structure manager
struct list lists[MAX_STRUCTURE_COUNT];
struct hash hashes[MAX_STRUCTURE_COUNT];

// TODO: Include bitmap
//struct bitmap bitmaps[MAX_STRUCTURE_COUNT];

static inline bool
is_command_equal(char* input, char* command) {
	return (strlen(input) == strlen(command)) &&
		   (strncmp(input, command, strlen(command)) == 0);
}

static inline bool
is_list_command(char* input) {
	return strncmp(input, "list_", 5) == 0;
}

static inline bool
is_hash_command(char* input) {
	return strncmp(input, "hash_", 5) == 0;
}

static inline bool
is_bitmap_command(char* input) {
	return strncmp(input, "bitmap_", 7) == 0;
}

static inline int
get_ds_index(char* arg) {
	return atoi(&arg[strlen(arg) - 1]);
}

bool compare_elements(const struct list_elem *a, const struct list_elem *b, void *aux) {
	struct list_item *item_a = list_entry(a, struct list_item, list_elem);
	struct list_item *item_b = list_entry(b, struct list_item, list_elem);

	return item_a->data < item_b->data;
}

void shell_loop();
void execute_command_from_buffer(char* buffer);

int main() {
	srand(time(NULL));

	shell_loop();
	return 0;
}

void shell_loop() {
	char buffer[MAX_BUFFER_SIZE];

	while (!QUIT) {
		fgets(buffer, MAX_BUFFER_SIZE, stdin);
		execute_command_from_buffer(buffer);
	}

	return;
}

void execute_command_from_buffer(char* buffer) {
	// Parse the buffer
	char cmd[MAX_ARG_SIZE], arg1[MAX_ARG_SIZE], arg2[MAX_ARG_SIZE], arg3[MAX_ARG_SIZE], arg4[MAX_ARG_SIZE], arg5[MAX_ARG_SIZE];
	int scan_count = sscanf(buffer, "%s %s %s %s %s %s", cmd, arg1, arg2, arg3, arg4, arg5);
	int args_count = scan_count - 1;

	int data_structure_index;  // Use this as an index variable for all ds - list, hash and bitmap

	// The only condition to quit the program normally
	if (is_command_equal(cmd, "quit"))
		QUIT = true;

	// Create data strcuture
	else if (is_command_equal(cmd, "create")) {
		data_structure_index = get_ds_index(arg2);

		if (strncmp(arg1, "list", 4) == 0) list_init (&lists[data_structure_index]);
		else if (strncmp(arg1, "hashtable", 9) == 0) {}
		else if (strncmp(arg1, "bitmap", 6) == 0) {}
	}

	// Delete data strcuture
	else if (is_command_equal(cmd, "delete")) {
		if (strncmp(arg1, "list", 4) == 0) {}
		else if (strncmp(arg1, "hash", 4)) {}
		else if (strncmp(arg1, "bm", 2)) {}
	}

	// Show data structure
	else if (is_command_equal(cmd, "dumpdata")) {
		data_structure_index = get_ds_index(arg1);
		if (strncmp(arg1, "list", 4) == 0) {
			if (&lists[data_structure_index] != NULL) {
				struct list_elem *e;
				for (e = list_begin (&lists[data_structure_index]); e != list_end (&lists[data_structure_index]); e = list_next (e)) {
					struct list_item *i = list_entry (e, struct list_item, list_elem);
					printf("%d ", i->data);
				}
				printf("\n");
			}
		}
		else if (strncmp(arg1, "hash", 4)) {}
		else if (strncmp(arg1, "bm", 2)) {}
	}

	// List related operations
	else if (is_list_command(cmd)) {
		data_structure_index = get_ds_index(arg1);
		// Insert
		if (is_command_equal(cmd, "list_insert")) {
			int index = atoi(arg2);
			int data = atoi(arg3);

			struct list_elem *e = list_begin (&lists[data_structure_index]);
			while (index) {e = list_next (e); index--;}

			struct list_item* item_to_insert = init_list_item(data);
			list_insert (e, &item_to_insert->list_elem);
		}
		else if (is_command_equal(cmd, "list_insert_ordered")) {
			int data = atoi(arg2);
			struct list_item* item_to_insert = init_list_item(data);
			list_insert_ordered (&lists[data_structure_index], &item_to_insert->list_elem, compare_elements, NULL);
		}
		else if (is_command_equal(cmd, "list_push_front")) {
			int data = atoi(arg2);
			struct list_item* item_to_insert = init_list_item(data);
			list_push_front (&lists[data_structure_index], &item_to_insert->list_elem);
		}
		else if (is_command_equal(cmd, "list_push_back")) {
			int data = atoi(arg2);
			struct list_item* item_to_insert = init_list_item(data);
			list_push_back (&lists[data_structure_index], &item_to_insert->list_elem);
		}
		// Remove
		else if (is_command_equal(cmd, "list_pop_front")) {
			list_pop_front (&lists[data_structure_index]);
		}
		else if (is_command_equal(cmd, "list_pop_back")) {
			list_pop_back (&lists[data_structure_index]);
		}
		else if (is_command_equal(cmd, "list_remove")) {
			int index = atoi(arg2);
			int data = atoi(arg3);

			struct list_elem *e = list_begin (&lists[data_structure_index]);
			while (index) {e = list_next (e); index--;}

			list_remove (e);
		}
		// Peek
		else if (is_command_equal(cmd, "list_front")) {
			struct list_item *front = list_entry (list_front (&lists[data_structure_index]), struct list_item, list_elem);
			printf("%d\n", front->data);
		}
		else if (is_command_equal(cmd, "list_back")) {
			struct list_item *back = list_entry (list_back (&lists[data_structure_index]), struct list_item, list_elem);
			printf("%d\n", back->data);
		}
		else if (is_command_equal(cmd, "list_empty")) {
			bool is_empty = list_empty (&lists[data_structure_index]);
			printf("%s\n", is_empty ? "true" : "false");
		}
		else if (is_command_equal(cmd, "list_size")) {
			size_t size = list_size (&lists[data_structure_index]);
			printf("%zu\n", size);
		}
		else if (is_command_equal(cmd, "list_max")) {
			struct list_item* max_item = list_entry (list_max (&lists[data_structure_index], compare_elements, NULL), struct list_item, list_elem);
			printf("%d\n", max_item->data);
		}
		else if (is_command_equal(cmd, "list_min")) {
			struct list_item* min_item = list_entry (list_min (&lists[data_structure_index], compare_elements, NULL), struct list_item, list_elem);
			printf("%d\n", min_item->data);
		}
		// Sort
		else if (is_command_equal(cmd, "list_reverse")) {
			list_reverse (&lists[data_structure_index]);
		}
		else if (is_command_equal(cmd, "list_sort")) {
			list_sort (&lists[data_structure_index], compare_elements, NULL);
		}
		else if (is_command_equal(cmd, "list_unique")) {
			struct list* duplicates = NULL;
			if (args_count >= 2) {
				int duplicates_index = get_ds_index(arg2);
				duplicates = &lists[duplicates_index];
			}

			list_unique (&lists[data_structure_index], duplicates, compare_elements, NULL);
		}
		else if (is_command_equal(cmd, "list_swap")) {
			struct list_elem* first = NULL;
			struct list_elem* second = NULL;

			int first_index = atoi(arg2);
			int second_index = atoi(arg3);

			int curr = 0;
			struct list_elem *e =list_begin (&lists[data_structure_index]);
			while (first == NULL || second == NULL) {
				if (curr == first_index) first = e;
				else if (curr == second_index) second = e;
				curr++;
				e = list_next (e);
			}
			list_swap(first, second);
		}
		else if (is_command_equal(cmd, "list_splice")) {
			int list_index_to_in = get_ds_index(arg1);
			int list_index_to_slice = get_ds_index(arg3);

			int index_to_in = atoi(arg2);
			int index_to_slice_from = atoi(arg4);
			int index_to_slice_to = atoi(arg5);

			struct list_elem *before = list_begin (&lists[list_index_to_in]);
			while (index_to_in) {before = list_next (before); index_to_in--;}

			struct list_elem *slice_from = list_begin (&lists[list_index_to_slice]);
			while (index_to_slice_from) {slice_from = list_next (slice_from); index_to_slice_from--;}

			struct list_elem *slice_to = list_begin (&lists[list_index_to_slice]);
			while (index_to_slice_to) {slice_to = list_next (slice_to); index_to_slice_to--;}

			list_splice(before, slice_from, slice_to);
		}
		else if (is_command_equal(cmd, "list_shuffle")) {
			list_suffle(&lists[data_structure_index]);
		}
	}

	// Hash related operations
	else if (is_hash_command(cmd)) {}

	// Bitmap related operations
	else if (is_bitmap_command(cmd)) {}
	else {
		printf("Unknown command: %s\n", buffer);
		exit(1);
	}

	return;
}