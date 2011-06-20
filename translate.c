#include "translate.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "aut_table.h"
#include "s_alloc.h"
#include "log.h"

static struct s_str eps = { 0, (lett_t *)"" };

static inline struct s_str concat(const struct s_str l, const struct s_str r) {
	int len_sum = l.size_m + r.size_m;
	lett_t * s_concat = alloc_n(len_sum + 1);
	memcpy(s_concat, l.str_mp, l.size_m);
	memcpy(s_concat + l.size_m, r.str_mp, r.size_m);
	s_concat[len_sum] = '\0';
	return (struct s_str){ len_sum, s_concat };
}

void create_trie(struct a_table * t, FILE * inp) {
	struct field * fld;
	lett_t word_buf[0x00001000]; // 4KB is the max len of a word
	lett_t tran_buf[0x00001000];
	int i, s_len;
	state_t q, next;

	while (fscanf(inp, "%s %s\n", word_buf, tran_buf) == 2) {
		LOG("new dict pair = %s -> %s\n", word_buf, tran_buf);

		q = init_state;
		for (i = 0; word_buf[i] != '\0'; ++i) {
			next = follow_trans(t, q, word_buf[i]);
			if (next == inval_state)
				break;
			else
				q = next;
		}

		LOG("\tmatched len = %d\n", i);
		LOG("\tstarted from state = %d\n", q);
		for ( ; word_buf[i] != '\0'; ++i) {
			/* next will be a new state */
			if ((next = t->size_m++) == t->space_m)
				inc_size(t);

			fld = get_trans(t, q, word_buf[i]);
			fld->to_m = next;
			fld->lambda_m = eps;

			q = next;
		}

		LOG("\ttotal len = %d\n", i);
		LOG("\tfinished in state = %d, rem states = %d\n", q, t->space_m - q);
		assert(is_final(t, q) == 0);
		set_final(t, q);

		s_len = strlen((char *)tran_buf);
		t->tbl_mp[q].phi_mp.str_mp = alloc_n(s_len + 1);
		memcpy(t->tbl_mp[q].phi_mp.str_mp, tran_buf, (s_len + 1) * sizeof ""[0]);
		t->tbl_mp[q].phi_mp.size_m = s_len;
	}
}

/* a constructed trie is a precondition */
void add_fail_links(struct a_table * t) {
	lett_t c;
	int * queue_cur, * queue_prev, q_head = 0, q_tail = 0;
	struct s_str * queue_gamma;
	int prev, cur;
	const int states = t->size_m;
	struct s_str gamma;
	struct field * cur_trans;
	struct field * prev_trans;

	assert((lett_t)0 == '\0'); /* this should be a compile-time assert */

	queue_cur = (int *)malloc(states * sizeof queue_cur[0]);
	queue_prev = (int *)malloc(states * sizeof queue_prev[0]);
	queue_gamma = (struct s_str *)malloc(states * sizeof queue_gamma[0]);

#if !defined NDEBUG
	/* print_state(t, init_state); */
#endif

	for (c = 1 /* skip the null terminator */; c != 0; ++c) {
		cur_trans = get_trans(t, init_state, c);
		gamma.str_mp = alloc_n(1 + 1);
		gamma.str_mp[0] = c;
		gamma.str_mp[1] = '\0';
		gamma.size_m = 1;
		if (!str_is_init(&cur_trans->lambda_m)) {
			assert(cur_trans->to_m == init_state);
			cur_trans->lambda_m = gamma;
		}
		else {
			queue_cur[q_tail] = cur_trans->to_m;
			queue_prev[q_tail] = init_state;
			queue_gamma[q_tail] = gamma;
			++q_tail;
		}
	}

	set_final(t, init_state);
	t->tbl_mp[init_state].phi_mp = eps;

	while (q_head < states - 1) {
		LOG("q_head = %d, q_tail = %d, size = %d\n", q_head, q_tail, states);
		assert(q_head < q_tail);

		cur = queue_cur[q_head];
		if (is_final(t, cur)) {
			prev = init_state;
			gamma = t->tbl_mp[cur].phi_mp;
		} else {
			prev = queue_prev[q_head];
			gamma = queue_gamma[q_head];
			set_final(t, cur);
			t->tbl_mp[cur].phi_mp = concat(gamma, t->tbl_mp[prev].phi_mp);
		}


		for (c = 1 /* skip the null terminator */; c != 0; ++c) {
			cur_trans = get_trans(t, cur, c);
			prev_trans = get_trans(t, prev, c);
			if (!str_is_init(&cur_trans->lambda_m)) {
				cur_trans->to_m = prev_trans->to_m;
				cur_trans->lambda_m = concat(gamma, prev_trans->lambda_m);
			} else {
				queue_cur[q_tail] = cur_trans->to_m;
				queue_prev[q_tail] = prev_trans->to_m;
				queue_gamma[q_tail] = concat(gamma, prev_trans->lambda_m);
				++q_tail;
			}
		}

		++q_head;
	}

	LOG("loop endted\nq_head = %d, q_tail = %d, size = %d\n", q_head, q_tail, states);
	assert(q_head == states - 1 && q_tail == states - 1);

	free(queue_gamma);
	free(queue_prev);
	free(queue_cur);
}

void travers(const struct a_table * t, FILE * in, FILE * out) {
	static const int chunk_size = 0x00100000; // 1MB
	lett_t * const read_buffer = (lett_t *)malloc(chunk_size * sizeof read_buffer[0]);
	lett_t * const write_buffer = (lett_t *)malloc(chunk_size * sizeof write_buffer[0]);
	int read_ptr, write_ptr = 0;
	int items;
	int f_end = 1;
	int state = init_state;
	const struct field * fld;
	struct s_str phi; /* used in the end */

	while (f_end) {
		items = fread(read_buffer, sizeof read_buffer[0], chunk_size, in);
		f_end = items == chunk_size;

		for (read_ptr = 0; read_ptr < items; ++read_ptr) {
			fld = get_trans_c(t, state, read_buffer[read_ptr]);

			/* write the lambda */
			if (write_ptr + fld->lambda_m.size_m > chunk_size) {
				fwrite(write_buffer, sizeof write_buffer[0], write_ptr, out);
				write_ptr = 0;
			}
			memcpy(write_buffer + write_ptr, fld->lambda_m.str_mp, fld->lambda_m.size_m + sizeof write_buffer[0]);
			write_ptr += fld->lambda_m.size_m;

			/* change the state */
			state = fld->to_m;
		}
	}

	/* write the phi */
	phi = t->tbl_mp[state].phi_mp;
	if (write_ptr + phi.size_m > chunk_size) {
		fwrite(write_buffer, sizeof write_buffer[0], write_ptr, out);
		write_ptr = 0;
	}
	memcpy(write_buffer + write_ptr, phi.str_mp, phi.size_m * sizeof write_buffer[0]);
	write_ptr += phi.size_m;
	fwrite(write_buffer, sizeof write_buffer[0], write_ptr, out);

	free(read_buffer);
	free(write_buffer);
}

