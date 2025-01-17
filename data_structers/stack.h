#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdbool.h>

#ifndef STK_TYPE
  #define STK_TYPE void*
#endif

#ifndef STK_IS_PTR
  #define STK_IS_PTR 1
#endif

typedef struct {
	STK_TYPE *vec;
	int top;
} stk_t;

void stk_init(stk_t *s);

bool stk_is_empty(stk_t *s);
void stk_push(stk_t *s, STK_TYPE item);
STK_TYPE stk_pop(stk_t *s);
STK_TYPE stk_peak(stk_t *s);

void stk_free(stk_t *s, void (*free_info)(void *));
int stk_size(stk_t *s);

void stk_print(stk_t *s, void (*print_info)(STK_TYPE));

#endif
