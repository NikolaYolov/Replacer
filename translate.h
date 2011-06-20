#if !defined TRANSLATE_H
#define TRANSLATE_H

#include <stdio.h>

struct a_table;

void create_trie(struct a_table * t, FILE * inp);

void add_fail_links(struct a_table * t);

void travers(const struct a_table * t, FILE * in, FILE * out);

#endif /* TRANSLATE_H */

