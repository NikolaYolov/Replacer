#include "aut_table.h"

#include <stdlib.h>
#include <memory.h>

#include "log.h"

void a_create_n(struct a_table * res, int n) {
	res->tbl = (struct row *)calloc(n, sizeof res->tbl[0]);
	/* LOG("alloc initial memory = %d, ptr = %p\n", n, res->tbl); */
	res->size = 1;
	res->space = n;
}

void a_free(struct a_table * t) {
	free(t->tbl);
}

void a_a_a_inc_size(struct a_table * t) {
	const int bytes = t->space * sizeof t->tbl[0];
	struct row * new;
	t->space *= 2;
	new = (struct row *)calloc(t->space, sizeof t->tbl[0]);
	/* LOG("alloc memory = %d, new ptr = %p, old ptr = %p\n", t->space, new, t->tbl); */
	memcpy(new, t->tbl, bytes);
	free(t->tbl);
	t->tbl = new;
}

#if !defined NDEBUG

void a_a_print_state(struct a_table * t, int s) {
	LOG("state %d description:\n\t"
	    "is final = %d\n\t"
	    "phi = %s\n\t", s, t->tbl[s].is_final, t->tbl[s].phi.str_mp);
	for (int ch = 1; ch < alphabet_size; ++ch) {
		const struct field * fld = a_get_trans_c(t, s, (lett_t)ch);
		if (str_is_init(&fld->lambda)) {
			LOG("%d -> %d, a = %s\t", (int)ch, fld->to, fld->lambda.str_mp);
			if (ch % 5 == 0)
				LOG("\n\t");
		}
	}
	LOG("\n");
}

#endif /* NDEBUG */

