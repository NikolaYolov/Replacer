#include "s_alloc.h"

#include <stdlib.h>

#include "const.h"

struct storage {
	struct storage * next_mp;
	lett_t memory_mp[0];
};

static struct storage * start;
static struct storage * head;
static int cur_size;
static int cur_ptr;

void mem_init() {
	cur_size = INITIAL_ALLOC_MEMORY;
	cur_ptr = 0;

	start = head = (struct storage * )malloc(sizeof *head + sizeof start->memory_mp[0] * cur_size);
	head->next_mp = 0;
}

lett_t * alloc_n(int n) {
	int old;
	if (cur_ptr + n > cur_size) {
		cur_size *= 2;
		cur_ptr = 0;
		head->next_mp = (struct storage * )malloc(sizeof *head + sizeof start->memory_mp[0] * cur_size);
		head = head->next_mp;
		head->next_mp = 0;
	}
	old = cur_ptr;
	cur_ptr += n;
	return head->memory_mp + old;
}

void mem_fin() {
	while (start) {
		head = start;
		start = start->next_mp;
		free(head);
	}
}

