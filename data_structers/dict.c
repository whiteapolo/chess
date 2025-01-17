#include "dict.h"
#include "lll.h"

static void (*g_free_info)(void *) = NULL;
static void (*g_print_info)(void *) = NULL;

void free_val(void *val)
{
	if (g_free_info)
		g_free_info(((Val*)val)->info);
	free(((Val*)val)->key);
	free((Val*)val);
}

void print_val(void *val)
{
	printf("KEY: %s, VALUE: ", ((Val*)val)->key);
	if (g_print_info)
		g_print_info(((Val*)val)->info);
	putchar('\n');
}

void dict_init(dict_t *dict, int size)
{
	dict->size = size;
	dict->managers = calloc(sizeof(lll_t*), dict->size);
}

void dict_insert(dict_t *dict, char *key, void *info)
{
	int h = hash(key) % dict->size;
	Val *val = malloc(sizeof(Val));
	val->key = strdup(key);
	val->info = info;
	lll_push(&dict->managers[h], val);
}

void dict_remove(dict_t *dict, char *key, void (*free_info)(void *))
{
	g_free_info = free_info;
	int h = hash(key) % dict->size;
	lll_t *curr = dict->managers[h];
	lll_t *pre = NULL;
	while (curr != NULL && strcmp(curr->info, key)) {
		pre = curr;
		curr = curr->next;
	}

	if (curr != NULL) {
		if (pre == NULL)
			free_val(lll_pop(&dict->managers[h]));
		else
			free_val(lll_remove_after(pre));
	}
}

void *dict_find(dict_t *dict, const char *key)
{
	int h = hash(key) % dict->size;
	lll_t *curr = dict->managers[h];
	while (curr != NULL && strcmp(((Val*)(curr->info))->key, key)) {
		curr = curr->next;
	}

	if (curr != NULL)
		return ((Val*)curr->info)->info;
	else
		return NULL;
}

void dict_print(dict_t *dict, void (*print_info)(void *))
{
	g_print_info = print_info;
	for (int i = 0; i < dict->size; i++)
		if (dict->managers[i])
			lll_print(dict->managers[i], print_val);
		else
			puts("----------------");
}

void dict_destroy(dict_t *dict, void (*free_info)(void *))
{
	g_free_info = free_info;
	for (int i = 0; i < dict->size; i++)
		if (dict->managers[i])
			lll_clear(&dict->managers[i], free_val);
	free(dict->managers);
}

unsigned long hash(const char *str)
{
	int c;
	unsigned long hash = 5381;
	while ((c = *(str++)))
		hash = ((hash << 5) + hash) + c;
	return hash;
}
