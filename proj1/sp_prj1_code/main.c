#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "bitmap.h"
#include "list.h"
#include "hash.h"

#define MAX_BUFFER_SIZE 1024
#define MAX_ARG_SIZE 32

// Maximum number of data structures (assumption #6)
#define MAX_STRUCTURE_COUNT 11

// A variable that determines to stop the program
bool QUIT = false;

// Simple data structure manager
struct list* lists[MAX_STRUCTURE_COUNT];
struct hash* hashes[MAX_STRUCTURE_COUNT];
struct bitmap* bitmaps[MAX_STRUCTURE_COUNT];

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

static inline bool
parse_boolean(char* arg) {
	return strncmp(arg, "true", 4) == 0;
}

bool compare_elements(const struct list_elem *, const struct list_elem *, void *);
unsigned hash_func(const struct hash_elem *, void *);
void hash_square_apply(struct hash_elem *, void *);
void hash_triple_apply(struct hash_elem *, void *);
bool compare_hash_elements(const struct hash_elem *, const struct hash_elem *, void *);
void execute_command_from_buffer(char*);
void shell_loop();

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

bool compare_elements(const struct list_elem *a, const struct list_elem *b, void *aux) {
	struct list_item *item_a = list_entry(a, struct list_item, list_elem);
	struct list_item *item_b = list_entry(b, struct list_item, list_elem);

	return item_a->data < item_b->data;
}

unsigned hash_func(const struct hash_elem *e, void *aux) {
	return hash_int(e->data);
}

void hash_square_apply(struct hash_elem *e, void *aux) {
	e->data = e->data * e->data;
}

void hash_triple_apply(struct hash_elem *e, void *aux) {
	e->data = e->data * e->data * e->data;
}

bool compare_hash_elements(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
	return a->data < b->data;
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
		if (strncmp(arg1, "list", 4) == 0) {
			lists[data_structure_index] = malloc(sizeof(struct list));
			list_init (lists[data_structure_index]);
		}
		else if (strncmp(arg1, "hashtable", 9) == 0) {
			hashes[data_structure_index] = malloc(sizeof(struct hash));
			hash_init (hashes[data_structure_index], hash_func, compare_hash_elements, NULL);
		}
		else if (strncmp(arg1, "bitmap", 6) == 0) {
			size_t bit_cnt = (size_t) atoi(arg3);
			bitmaps[data_structure_index] = bitmap_create(bit_cnt);
		}
	}

	// Delete data strcuture
	else if (is_command_equal(cmd, "delete")) {
		data_structure_index = get_ds_index(arg1);
		if (strncmp(arg1, "list", 4) == 0) {
			struct list* list = lists[data_structure_index];
			while (!list_empty (list)) list_pop_front(list);
			free (list);
		}
		else if (strncmp(arg1, "hash", 4) == 0) {
			struct hash* hash = hashes[data_structure_index];
			hash_destroy (hash, NULL);
			free (hash);
		}
		else if (strncmp(arg1, "bm", 2) == 0) {
			struct bitmap* bitmap = bitmaps[data_structure_index];
			bitmap_destroy(bitmap);
		}
	}

	// Show data structure
	else if (is_command_equal(cmd, "dumpdata")) {
		data_structure_index = get_ds_index(arg1);

		if (strncmp(arg1, "list", 4) == 0) {
			struct list* list = lists[data_structure_index];
			for (struct list_elem *e = list_begin (list); e != list_end (list); e = list_next (e)) {
				struct list_item *i = list_entry (e, struct list_item, list_elem);
				printf("%d ", i->data);
			}
		}
		else if (strncmp(arg1, "hash", 4) == 0) {
			struct hash* hash = hashes[data_structure_index];
			struct hash_iterator* i = malloc(sizeof(struct hash_iterator));

			hash_first (i, hash);
			while (hash_next (i)) printf("%d ", hash_cur(i)->data);

			free(i);
		}
		else if (strncmp(arg1, "bm", 2) == 0) {
			struct bitmap* bitmap = bitmaps[data_structure_index];
			for (size_t i = 0; i < bitmap_size(bitmap); i++) printf("%d", bitmap_test(bitmap, i) ? 1 : 0);
		}
		printf("\n");
	}

	// List related operations
	else if (is_list_command(cmd)) {
		data_structure_index = get_ds_index(arg1);
		// Insert
		if (is_command_equal(cmd, "list_insert")) {
			int index = atoi(arg2);
			int data = atoi(arg3);

			struct list_elem *e = list_begin (lists[data_structure_index]);
			while (index) {e = list_next (e); index--;}

			struct list_item* item_to_insert = new_list_item(data);
			list_insert (e, &item_to_insert->list_elem);
		}
		else if (is_command_equal(cmd, "list_insert_ordered")) {
			int data = atoi(arg2);
			struct list_item* item_to_insert = new_list_item(data);
			list_insert_ordered (lists[data_structure_index], &item_to_insert->list_elem, compare_elements, NULL);
		}
		else if (is_command_equal(cmd, "list_push_front")) {
			int data = atoi(arg2);
			struct list_item* item_to_insert = new_list_item(data);
			list_push_front (lists[data_structure_index], &item_to_insert->list_elem);
		}
		else if (is_command_equal(cmd, "list_push_back")) {
			int data = atoi(arg2);
			struct list_item* item_to_insert = new_list_item(data);
			list_push_back (lists[data_structure_index], &item_to_insert->list_elem);
		}
		// Remove
		else if (is_command_equal(cmd, "list_pop_front")) {
			list_pop_front (lists[data_structure_index]);
		}
		else if (is_command_equal(cmd, "list_pop_back")) {
			list_pop_back (lists[data_structure_index]);
		}
		else if (is_command_equal(cmd, "list_remove")) {
			int index = atoi(arg2);

			struct list_elem *e = list_begin (lists[data_structure_index]);
			while (index) {e = list_next (e); index--;}

			list_remove (e);
		}
		// Peek
		else if (is_command_equal(cmd, "list_front")) {
			struct list_item *front = list_entry (list_front (lists[data_structure_index]), struct list_item, list_elem);
			printf("%d\n", front->data);
		}
		else if (is_command_equal(cmd, "list_back")) {
			struct list_item *back = list_entry (list_back (lists[data_structure_index]), struct list_item, list_elem);
			printf("%d\n", back->data);
		}
		else if (is_command_equal(cmd, "list_empty")) {
			bool is_empty = list_empty (lists[data_structure_index]);
			printf("%s\n", is_empty ? "true" : "false");
		}
		else if (is_command_equal(cmd, "list_size")) {
			size_t size = list_size (lists[data_structure_index]);
			printf("%zu\n", size);
		}
		else if (is_command_equal(cmd, "list_max")) {
			struct list_item* max_item = list_entry (list_max (lists[data_structure_index], compare_elements, NULL), struct list_item, list_elem);
			printf("%d\n", max_item->data);
		}
		else if (is_command_equal(cmd, "list_min")) {
			struct list_item* min_item = list_entry (list_min (lists[data_structure_index], compare_elements, NULL), struct list_item, list_elem);
			printf("%d\n", min_item->data);
		}
		// Sort
		else if (is_command_equal(cmd, "list_reverse")) {
			list_reverse (lists[data_structure_index]);
		}
		else if (is_command_equal(cmd, "list_sort")) {
			list_sort (lists[data_structure_index], compare_elements, NULL);
		}
		else if (is_command_equal(cmd, "list_unique")) {
			struct list* duplicates = NULL;
			if (args_count >= 2) {
				int duplicates_index = get_ds_index(arg2);
				duplicates = lists[duplicates_index];
			}

			list_unique (lists[data_structure_index], duplicates, compare_elements, NULL);
		}
		else if (is_command_equal(cmd, "list_swap")) {
			struct list_elem* first = NULL;
			struct list_elem* second = NULL;

			int first_index = atoi(arg2);
			int second_index = atoi(arg3);

			int curr = 0;
			struct list_elem *e =list_begin (lists[data_structure_index]);
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

			struct list_elem *before = list_begin (lists[list_index_to_in]);
			while (index_to_in) {before = list_next (before); index_to_in--;}

			struct list_elem *slice_from = list_begin (lists[list_index_to_slice]);
			while (index_to_slice_from) {slice_from = list_next (slice_from); index_to_slice_from--;}

			struct list_elem *slice_to = list_begin (lists[list_index_to_slice]);
			while (index_to_slice_to) {slice_to = list_next (slice_to); index_to_slice_to--;}

			list_splice(before, slice_from, slice_to);
		}
		else if (is_command_equal(cmd, "list_shuffle")) list_suffle(lists[data_structure_index]);
	}

	// Hash related operations
	else if (is_hash_command(cmd)) {
		data_structure_index = get_ds_index(arg1);
		struct hash* hash = hashes[data_structure_index];

		// Insert
		if (is_command_equal(cmd, "hash_insert")) {
			int data = atoi(arg2);
			struct hash_elem* hash_elem = new_hash_elem(data);

			hash_insert (hash, hash_elem);
		}
		// Delete
		else if (is_command_equal(cmd, "hash_delete")) {
			int data = atoi(arg2);

			struct hash_elem* to_free = hash_delete (hash, new_hash_elem(data));
			if (to_free) free(to_free);
		}
		// Replace
		else if (is_command_equal(cmd, "hash_replace")) {
			int data = atoi(arg2);
			struct hash_elem* hash_elem = new_hash_elem(data);

			hash_replace (hash, hash_elem);
		}
		// Find
		else if (is_command_equal(cmd, "hash_find")) {
			int data = atoi(arg2);

			struct hash_elem* found = hash_find (hash, new_hash_elem(data));
			if (found) printf("%d\n", found->data);
		}
		// Miscellaneous
		else if (is_command_equal(cmd, "hash_apply")) {
			if (is_command_equal(arg2, "square")) hash_apply (hash, hash_square_apply);
			else if (is_command_equal(arg2, "triple")) hash_apply (hash, hash_triple_apply);
		}
		else if (is_command_equal(cmd, "hash_empty")) printf("%s\n", hash_empty (hash) ? "true" : "false");
		else if (is_command_equal(cmd, "hash_size")) printf("%zu\n", hash_size (hash));
		else if (is_command_equal(cmd, "hash_clear")) hash_clear (hash, NULL);
	}

	// Bitmap related operations
	else if (is_bitmap_command(cmd)) {
		data_structure_index = get_ds_index(arg1);
		struct bitmap* bitmap = bitmaps[data_structure_index];
		size_t idx, cnt;

		// Update
		if (is_command_equal(cmd, "bitmap_mark")) {
			idx = (size_t) atoi(arg2);

			bitmap_mark (bitmap, idx);
		}
		else if (is_command_equal(cmd, "bitmap_set")) {
			idx = (size_t) atoi(arg2);
			bool value = parse_boolean(arg3);

			bitmap_set (bitmap, idx, value);
		}
		else if (is_command_equal(cmd, "bitmap_set_multiple")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool value = parse_boolean(arg4);

			bitmap_set_multiple (bitmap, idx, cnt, value);
		}
		else if (is_command_equal(cmd, "bitmap_set_all")) {
			bool value = parse_boolean(arg2);

			bitmap_set_all (bitmap, value);
		}
		else if (is_command_equal(cmd, "bitmap_reset")) {
			idx = (size_t) atoi(arg2);
			bitmap_reset (bitmap, idx);
		}
		else if (is_command_equal(cmd, "bitmap_flip")) {
			idx = (size_t) atoi(arg2);
			bitmap_flip (bitmap, idx);
		}
		else if (is_command_equal(cmd, "bitmap_scan_and_flip")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool value = parse_boolean(arg4);

			size_t scanned_idx = bitmap_scan_and_flip (bitmap, idx, cnt, value);
			printf("%zu\n", scanned_idx);
		}
		else if (is_command_equal(cmd, "bitmap_expand")) {
			cnt = (size_t) atoi(arg2);
			bitmap_expand (bitmap, cnt);
		}
		// Read
		else if (is_command_equal(cmd, "bitmap_all")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool is_all = bitmap_all (bitmap, idx, cnt);
			printf("%s\n", is_all ? "true" : "false");
		}
		else if (is_command_equal(cmd, "bitmap_any")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool is_any = bitmap_any (bitmap, idx, cnt);

			printf("%s\n", is_any ? "true" : "false");
		}
		else if (is_command_equal(cmd, "bitmap_contains")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool value = parse_boolean(arg4);
			bool is_contains = bitmap_contains (bitmap, idx, cnt, value);

			printf("%s\n", is_contains ? "true" : "false");
		}
		else if (is_command_equal(cmd, "bitmap_none")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool is_none = bitmap_none (bitmap, idx, cnt);

			printf("%s\n", is_none ? "true" : "false");
		}
		else if (is_command_equal(cmd, "bitmap_scan")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool value = parse_boolean(arg4);

			size_t scanned_idx = bitmap_scan (bitmap, idx, cnt, value);
			printf("%zu\n", scanned_idx);
		}
		else if (is_command_equal(cmd, "bitmap_test")) {
			idx = (size_t) atoi(arg2);
			bool is_set = bitmap_test (bitmap, idx);

			printf("%s\n", is_set ? "true" : "false");
		}
		else if (is_command_equal(cmd, "bitmap_count")) {
			idx = (size_t) atoi(arg2);
			cnt = (size_t) atoi(arg3);
			bool value = parse_boolean(arg4);
			size_t bit_cnt = bitmap_count (bitmap, idx, cnt, value);

			printf("%zu\n", bit_cnt);
		}
		else if (is_command_equal(cmd, "bitmap_size")) printf("%zu\n", bitmap_size (bitmap));
		else if (is_command_equal(cmd, "bitmap_dump")) bitmap_dump(bitmap);
	}
	else {
		printf("Unknown command: %s\n", buffer);
		exit(1);
	}

	return;
}
