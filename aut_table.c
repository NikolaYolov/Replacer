#include "aut_table.h"

#include <stdlib.h>
#include <memory.h>

void a_create_n(struct a_table * res, int n) {
	res->tbl_mp = (struct row *)calloc(n, sizeof res->tbl_mp[0]);
	res->size_m = 1;
	res->space_m = n;
}

void a_free(struct a_table * t) {
	free(t->tbl_mp);
}

void inc_size(struct a_table * t) {
	int bytes = t->space_m * sizeof *t->tbl_mp;
	struct row * new = (struct row *)calloc(t->space_m * 2, sizeof *t->tbl_mp);
	memcpy(new, t->tbl_mp, bytes);
	free(t->tbl_mp);
	t->tbl_mp = new;
	t->space_m *= 2;
}

#if !defined NDEBUG

#include "log.h"

void print_state(struct a_table * t, int s) {
	LOG("state %d description:\n\t"
	    "is final = %d\n\t"
	    "phi = %s\n\t", s, t->tbl_mp[s].is_final_m, t->tbl_mp[s].phi_mp.str_mp);
	for (int ch = 1; ch < alpha_s; ++ch) {
		const struct field * fld = get_trans_c(t, s, (lett_t)ch);
		if (str_is_init(&fld->lambda_m)) {
			LOG("%d -> %d, a = %s\t", (int)ch, fld->to_m, fld->lambda_m.str_mp);
			if (ch % 5 == 0)
				LOG("\n\t");
		}
	}
	LOG("\n");
}

#endif

