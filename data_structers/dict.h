/* Autor (whiteapolo)
 *
 * implemented by open hash table to a linear linked list
 *
 *
 * USAGE:
 *
 * dict_t dict;
 * dict_init(&dict, 10);
 * // size matters how much memory will be takes, to how efficient the finding will be
 *
 * dict_insert(&dict, "key", strdup(str));
 * // you dont need to alocate space for the key, but you do for the value. see strdup
 * // you can store any thing that is a pointer
 *
 * dict_find(&dict, "key");
 * // if vlaue doesnt exists for this key, NULL will be returned
 *
 * // after you done, free the memory
 * dict_destroy(&dict, free);
 * // or
 * dict_destroy(&dict, NULL);
 *
 *
 * // the function to free info can be NULL.
 */

#ifndef DICT_H
#define DICT_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lll.h"


typedef struct {
	char *key;
	void *info;
} Val;

typedef struct {
	lll_t **managers;
	unsigned int size;
} dict_t;

void dict_init(dict_t *dict, int size);

// key is automaticly duplicated, info is not.
void dict_insert(dict_t *dict, char *key, void *info);

// remove a value by key
void dict_remove(dict_t *dict, char *key, void (*free_info)(void *));

// if vlaue doesnt exists for this key, NULL will be returned
void *dict_find(dict_t *dict, const char *key);

// free the memory, if you want to free the info, pass a function to do so
// you can pass libc `free` function
void dict_destroy(dict_t *dict, void (*free_info)(void *));

// pass a function that prints the info you stored, the key is automaticly printed
void dict_print(dict_t *dict, void (*print_info)(void *));

unsigned long hash(const char *str);

#endif
