#if !defined LOG_H
#define LOG_H

#if defined NDEBUG
#	define LOG(s, ...)
#else
#	include <stdio.h>
#	define LOG printf
#endif

#endif /* LOG_H */

