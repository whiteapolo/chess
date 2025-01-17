#ifndef LLL_H
#define LLL_H

#include <stdio.h>
#include <stdlib.h>

// Declerations
// ------------
typedef struct lll_t{
	struct lll_t *next;
	void *info;
} lll_t;

void lll_init(lll_t **m);
void lll_push(lll_t **m, void *info);
void *lll_pop(lll_t **m);

void lll_insert_after(lll_t *l, void *info);
void *lll_remove_after(lll_t *l);

void lll_clear(lll_t **m, void (*free_info)(void *));
void lll_print(lll_t *l, void(*print_info)(void*));

lll_t *lll_get_last(lll_t *l);

#endif
