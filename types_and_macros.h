#ifndef _CHESS_MACROS_H
#define _CHESS_MACROS_H

#include <stdint.h>

typedef uint_fast64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

static const char pieces[] = "PNBRQKpnbrqk";

#define FOR_RANGE(var, from, to, jumps) for (int var = (from); var < (to); var+=jumps)
#define FOR(var, till) FOR_RANGE(var, 0, till, 1)

#define REP(n, exp) FOR(_tmp, n) { exp }
#define IS_EVEN(num) ((num)&1)

#define ASSERT(a, msg) if(a) {printf("ERROR: %s\n", msg); exit(1);}
#define IS_PIECE(p) ((bool)p && (bool)strchr(pieces, tolower(p)))

#define IN_BOUNDRY(n, n1, n2) (n1 <= n && n <= n2)

#define PRINT_VAR(x) _Generic((x), \
	int: printf("%s = %d\n", #x, x),\
	float: printf("%s = %d\n", #x, x),\
	char: printf("%s = %c\n", #x, x),\
	u8: printf("%s = %d\n", #x, x),\
	bool: printf("%s = %d\n", #x, x),\
	short: printf("%s = %d\n", #x, x),\
	size_t: printf("%s = %d\n", #x, x),\
	unsigned int: printf("%s = %u\n", #x, x))

#endif
