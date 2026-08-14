#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "contracts.h"
#include "astring.h"

bool test_convert(char *str){
    astring_t *a = string_to_astring(str);
    char *s = astring_to_string(a);
    bool same = (strcmp(s, str)) == 0;
    free(s);
    free_astring(a);
    return same;
}

int main (){
    assert(test_convert("Hello World"));


    return 0;
}