#if !defined AUTOMATA_TABLE_H
#define AUTOMATA_TABLE_H

#include <assert.h>

#include "defs.h"
#include "s_str.h"

/**
 * Structure
 */

typedef int state_t;
#define INITIAL_STATE 0
#define INVALID_STATE -1

struct field {
	state_t to;
	struct s_str lambda;
};

struct row {
	struct field transit[alphabet_size];
	struct s_str phi;
	int is_final;
};

struct a_table {
	struct row * tbl;
	int size;
	int space;
};

/**
 * Interface
 */

void a_create_n(struct a_table * res, int n);
void a_free(struct a_table * t);
void a_a_a_inc_size(struct a_table * t);

#if !defined NDEBUG
void a_a_print_state(struct a_table * t, int s);
#endif

/**
 * Inline implementation
 */

static inline struct field * a_get_trans(struct a_table * a, state_t s, lett_t l) {
	assert(s < a->size);
	return &(a->tbl[s].transit[(int)l]);
}

static inline const struct field * a_get_trans_c(const struct a_table * a, state_t s, lett_t l) {
	assert(s < a->size);
	return &(a->tbl[s].transit[(int)l]);
}

/* INVALID_STATE for fail */
static inline state_t a_follow_trans(const struct a_table * a, state_t s, lett_t l) {
	assert(s < a->size);
	const struct field * fld = a_get_trans_c(a, s, l);
	return str_is_init(&fld->lambda)? fld->to : INVALID_STATE;
}

/* all states are not final by default */
static inline void a_set_final(const struct a_table * a, state_t s) {
	assert(s < a->size);
	a->tbl[s].is_final = 1;
}

static inline int is_final(const struct a_table * a, state_t s) {
	assert(s < a->size);
	return a->tbl[s].is_final;
}

#endif /* AUTOMATA_TABLE_H */

