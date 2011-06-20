#if !defined AUTOMATA_TABLE_H
#define AUTOMATA_TABLE_H

#include <assert.h>

#include "defs.h"
#include "s_str.h"

/**
 * Structure
 */

typedef int state_t;
#define init_state  0
#define inval_state  -1

struct field {
	state_t to_m;
	struct s_str lambda_m;
};

struct row {
	struct field transit_mp[alpha_s];
	struct s_str phi_mp;
	int is_final_m;
};

struct a_table {
	struct row * tbl_mp;
	int size_m;
	int space_m;
};

/**
 * Interface
 */

void a_create_n(struct a_table * res, int n);
void a_free(struct a_table * t);
void inc_size(struct a_table * t);

#if !defined NDEBUG
void print_state(struct a_table * t, int s);
#endif

/**
 * Inline implementation
 */

static inline struct field * get_trans(struct a_table * a, state_t s, lett_t l) {
	assert(s < a->size_m);
	return &(a->tbl_mp[s].transit_mp[(int)l]);
}

static inline const struct field * get_trans_c(const struct a_table * a, state_t s, lett_t l) {
	assert(s < a->size_m);
	return &(a->tbl_mp[s].transit_mp[(int)l]);
}

/* inval_state for fail */
static inline state_t follow_trans(const struct a_table * a, state_t s, lett_t l) {
	assert(s < a->size_m);
	const struct field * fld = get_trans_c(a, s, l);
	return str_is_init(&fld->lambda_m)? fld->to_m : inval_state;
}

/* all states are not final by default */
static inline void set_final(const struct a_table * a, state_t s) {
	assert(s < a->size_m);
	a->tbl_mp[s].is_final_m = 1;
}

static inline int is_final(const struct a_table * a, state_t s) {
	assert(s < a->size_m);
	return a->tbl_mp[s].is_final_m;
}

#endif /* AUTOMATA_TABLE_H */

