#if !defined CONST_H
#define CONST_H

#define init_size             0x00080000 /* 2^19 states initially */
#define chunk_size            0x00100000 /* 1MB per I/O buffer */
#define INITIAL_ALLOC_MEMORY  0x00100000 /* 1MB for string allocations */
#define WORD_MAX_LEN          0x00001000 /* 4KB is the max word len */

#endif /* CONST_H */
 
