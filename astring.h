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

size_t astring_size(astring_t *a);
astring_t *string_to_astring(char *str); // Threaded
void free_astring(astring_t *a);
void print_astring(astring_t *a);
char *astring_to_string(astring_t *a);
bool astring_eq(astring_t *a1, astring_t *a2); // Threaded
char astring_index_at(astring_t *a, size_t i);

#endif