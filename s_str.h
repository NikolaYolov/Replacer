#if !defined SIZED_STRING
#define SIZED_STRING

#include "defs.h"

struct s_str {
	int size;
	lett_t * str_mp;
};

static inline int str_is_init(const struct s_str * s) {
	return (int)s->str_mp;
}

#endif /* SIZED_STRING */

