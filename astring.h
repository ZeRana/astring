#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "contracts.h"

#ifndef ASTRING_H
#define ASTRING_H
typedef struct astring_header astring_t;

size_t astring_size(astring_t *a);
astring_t *string_to_astring(char *str);
void free_astring(astring_t *a);
void print_astring(astring_t *a);
char *astring_to_string(astring_t *a);

#endif