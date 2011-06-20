#if !defined STRING_ALLOCATOR_H
#define STRING_ALLOCATOR_H

#include "defs.h"

void       mem_init();

lett_t *   alloc_n(int n);

void       mem_fin();

#endif /* STRING_ALLOCATOR_H */

