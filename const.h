#if !defined CONST_H
#define CONST_H

#if defined NDEBUG

#define INIT_SIZE             0x00010000 /* 2^16 states initially */
#define CHUNK_SIZE            0x00100000 /* 1MB per I/O buffer */
#define INITIAL_ALLOC_MEMORY  0x00100000 /* 1MB for string allocations */
#define WORD_MAX_LEN          0x00001000 /* 4KB is the max word len */

#else

#define INIT_SIZE             0x00000001 /* 2^0 states initially */
#define CHUNK_SIZE            0x00001000 /* 8KB per I/O buffer */
#define INITIAL_ALLOC_MEMORY  0x00000001 /* 1MB for string allocations */
#define WORD_MAX_LEN          0x00001000 /* 4KB is the max word len */

#endif /* NDEUBG */

#endif /* CONST_H */
 
