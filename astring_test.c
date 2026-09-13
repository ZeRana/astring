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

bool test_eq(char *s1, char *s2){
    astring_t *a1 = string_to_astring(s1);
    astring_t *a2 = string_to_astring(s2);

    bool astringres = astring_eq(a1, a2);
    int strres = strcmp(s1, s2);

    free_astring(a1);
    free_astring(a2);

    return (strres == 0) == (astringres);
}

int main (){
    assert(test_convert("Hello World"));
    assert(test_convert("Skibidi Gyatt"));
    assert(test_convert(""));
    assert(test_convert("amougus"));

    assert(test_eq("Hello World", "Hello World"));
    assert(test_eq("", "Hello World"));
    assert(test_eq("", ""));
    assert(test_eq("Silly", "Goose"));
    assert(test_eq("Silly", "Sille"));

    printf("All tests passed!\n");
    return 0;
}