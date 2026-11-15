#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>

#include "contracts.h"

#ifndef ASTRING_H
#define ASTRING_H
typedef struct astring_header astring_t;
typedef struct astring_iterator_header astring_iterator_t;

size_t astring_size(astring_t *a);
astring_t *string_to_astring(char *str); // Threaded
void free_astring(astring_t *a);
void print_astring(astring_t *a);
char *astring_to_string(astring_t *a);
bool astring_eq(astring_t *a1, astring_t *a2); // Threaded
char astring_index_at(astring_t *a, size_t i);
void astring_update_char(astring_t *a, size_t index, char new_val);
astring_t *astring_insert(astring_t *a, size_t i, const char *str); // Threaded
astring_t *astring_concat(astring_t *a1, astring_t *a2);
char astring_char(astring_t *a);


astring_iterator_t *new_iterator(astring_t *a); // lazy implementation of an iterator taking up o(h) space
astring_t *iterator_next(astring_iterator_t *iterator);
bool iterator_has_next(astring_iterator_t *iterator);
void free_iterator(astring_iterator_t *iterator);
#endif