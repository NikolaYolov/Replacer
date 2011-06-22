#include "translate.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "aut_table.h"
#include "s_alloc.h"
#include "const.h"
/* #include "log.h" */

static struct s_str eps = { 0, (lett_t *)"" };

static inline struct s_str concat(const struct s_str l, const struct s_str r) {
	int len_sum = l.size + r.size;
	lett_t * s_concat = alloc_n(len_sum + 1);
	memcpy(s_concat, l.str_mp, l.size * sizeof s_concat[0]);
	memcpy(s_concat + l.size, r.str_mp, r.size * sizeof s_concat[0]);
	s_concat[len_sum] = '\0';
	return (struct s_str){ len_sum, s_concat };
}

void create_trie(struct a_table * t, FILE * inp) {
	struct field * fld;
	lett_t word_buf[WORD_MAX_LEN]; // 4KB is the max len of a word
	lett_t tran_buf[WORD_MAX_LEN];
	int i, s_len;
	state_t q, next;

	while (fscanf(inp, "%s %s\n", word_buf, tran_buf) == 2) {
		/* LOG("new dict pair = %s -> %s\n", word_buf, tran_buf); */

		q = INITIAL_STATE;
		for (i = 0; word_buf[i] != '\0'; ++i) {
			next = a_follow_trans(t, q, word_buf[i]);
			if (next == INVALID_STATE)
				break;
			else
				q = next;
		}

		/* LOG("\tmatched len = %d\n", i); */
		/* LOG("\tstarted from state = %d\n", q); */
		for ( ; word_buf[i] != '\0'; ++i) {
			/* next will be a new state */
			if ((next = t->size++) == t->space)
				a_a_a_inc_size(t);

			fld = a_get_trans(t, q, word_buf[i]);
			fld->to = next;
			fld->lambda = eps;

			q = next;
		}

		/* LOG("\ttotal len = %d\n", i); */
		/* LOG("\tfinished in state = %d, rem states = %d\n", q, t->space - q); */
		assert(is_final(t, q) == 0);
		a_set_final(t, q);

		s_len = strlen((char *)tran_buf);
		t->tbl[q].phi.str_mp = alloc_n(s_len + 1);
		memcpy(t->tbl[q].phi.str_mp, tran_buf, (s_len + 1) * sizeof ""[0]);
		t->tbl[q].phi.size = s_len;
	}
}

/* a constructed trie is a precondition */
void add_fail_links(struct a_table * t) {
	lett_t c;
	int * queue_cur, * queue_prev, q_head = 0, q_tail = 0;
	struct s_str * queue_gamma;
	int prev, cur;
	const int states = t->size;
	struct s_str gamma;
	struct field * cur_trans;
	struct field * prev_trans;

	assert((lett_t)0 == '\0'); /* this should be a compile-time assert */

	queue_cur = (int *)malloc(states * sizeof queue_cur[0]);
	queue_prev = (int *)malloc(states * sizeof queue_prev[0]);
	queue_gamma = (struct s_str *)malloc(states * sizeof queue_gamma[0]);

#if !defined NDEBUG
	/* a_a_print_state(t, INITIAL_STATE); */
#endif

	for (c = 1 /* skip the null terminator */; c != 0; ++c) {
		cur_trans = a_get_trans(t, INITIAL_STATE, c);
		gamma.str_mp = alloc_n(1 + 1);
		gamma.str_mp[0] = c;
		gamma.str_mp[1] = '\0';
		gamma.size = 1;
		if (!str_is_init(&cur_trans->lambda)) {
			assert(cur_trans->to == INITIAL_STATE);
			cur_trans->lambda = gamma;
		}
		else {
			queue_cur[q_tail] = cur_trans->to;
			queue_prev[q_tail] = INITIAL_STATE;
			queue_gamma[q_tail] = gamma;
			++q_tail;
		}
	}

	a_set_final(t, INITIAL_STATE);
	t->tbl[INITIAL_STATE].phi = eps;

	while (q_head < states - 1) {
		/* LOG("q_head = %d, q_tail = %d, size = %d\n", q_head, q_tail, states); */
		assert(q_head < q_tail);

		cur = queue_cur[q_head];
		if (is_final(t, cur)) {
			prev = INITIAL_STATE;
			gamma = t->tbl[cur].phi;
		} else {
			prev = queue_prev[q_head];
			gamma = queue_gamma[q_head];
			a_set_final(t, cur);
			t->tbl[cur].phi = concat(gamma, t->tbl[prev].phi);
		}


		for (c = 1 /* skip the null terminator */; c != 0; ++c) {
			cur_trans = a_get_trans(t, cur, c);
			prev_trans = a_get_trans(t, prev, c);
			if (!str_is_init(&cur_trans->lambda)) {
				cur_trans->to = prev_trans->to;
				cur_trans->lambda = concat(gamma, prev_trans->lambda);
			} else {
				queue_cur[q_tail] = cur_trans->to;
				queue_prev[q_tail] = prev_trans->to;
				queue_gamma[q_tail] = concat(gamma, prev_trans->lambda);
				++q_tail;
			}
		}

		++q_head;
	}

	/* LOG("loop endted\nq_head = %d, q_tail = %d, size = %d\n", q_head, q_tail, states); */
	assert(q_head == states - 1 && q_tail == states - 1);

	free(queue_gamma);
	free(queue_prev);
	free(queue_cur);
}

void travers(const struct a_table * t, FILE * in, FILE * out) {
	lett_t * const read_buffer = (lett_t *)malloc(CHUNK_SIZE * sizeof read_buffer[0]);
	lett_t * const write_buffer = (lett_t *)malloc(CHUNK_SIZE * sizeof write_buffer[0]);
	int read_ptr, write_ptr = 0;
	int items;
	int f_end = 1;
	int state = INITIAL_STATE;
	const struct field * fld;
	struct s_str phi; /* used in the end */

	while (f_end) {
		items = fread(read_buffer, sizeof read_buffer[0], CHUNK_SIZE, in);
		f_end = items == CHUNK_SIZE;

		for (read_ptr = 0; read_ptr < items; ++read_ptr) {
			fld = a_get_trans_c(t, state, read_buffer[read_ptr]);

			/* write the lambda */
			if (write_ptr + fld->lambda.size > CHUNK_SIZE) {
				fwrite(write_buffer, sizeof write_buffer[0], write_ptr, out);
				write_ptr = 0;
			}
			memcpy(write_buffer + write_ptr, fld->lambda.str_mp, fld->lambda.size + sizeof write_buffer[0]);
			write_ptr += fld->lambda.size;

			/* change the state */
			state = fld->to;
		}
	}

	/* write the phi */
	phi = t->tbl[state].phi;
	if (write_ptr + phi.size > CHUNK_SIZE) {
		fwrite(write_buffer, sizeof write_buffer[0], write_ptr, out);
		write_ptr = 0;
	}
	memcpy(write_buffer + write_ptr, phi.str_mp, phi.size * sizeof write_buffer[0]);
	write_ptr += phi.size;
	fwrite(write_buffer, sizeof write_buffer[0], write_ptr, out);

	free(read_buffer);
	free(write_buffer);
}

