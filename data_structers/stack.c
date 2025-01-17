#include "stack.h"

void stk_init(stk_t *s)
{
	s->vec = NULL;
	s->top = 0;
}

bool stk_is_empty(stk_t *s)
{
	return s->top == 0;
}

void stk_push(stk_t *s, STK_TYPE item)
{
	s->vec = realloc(s->vec, sizeof(STK_TYPE) * ++s->top);
	s->vec[s->top - 1] = item;
}

STK_TYPE stk_pop(stk_t *s)
{
	STK_TYPE item = s->vec[s->top - 1];
	s->vec = realloc(s->vec, sizeof(STK_TYPE) * --s->top);
	return item;
}

void stk_free(stk_t *s, void (*free_info)(void *))
{
#if STK_IS_PTR == 1
	if (free_info != NULL) {
		for (int i = s->top -1; i >= 0; i--) {
			free(s->vec[i]);		
		}
	}
#endif
	free(s->vec);
}

STK_TYPE stk_peak(stk_t *s)
{
	return s->vec[s->top - 1];
}

int stk_size(stk_t *s)
{
	return s->top;
}

void stk_print(stk_t *s, void (*print_info)(STK_TYPE))
{
	for (int i = s->top - 1; i >= 0; i--) {
		print_info(s->vec[i]);
	}
}
