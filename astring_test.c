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

void test_index_at(char *s){
    astring_t *a = string_to_astring(s);
    for (size_t i = 0; i < strlen(s); i++){
        assert(astring_index_at(a, i) == s[i]);
    }

    free_astring(a);
}

// len(new_vals) <= len(s)
void test_update(char *s, char *new_vals){
    astring_t *a = string_to_astring(s);
    for (size_t i = 0; i < strlen(new_vals); i++){
        astring_update_char(a, i, new_vals[i]);
        assert(astring_index_at(a, i) == new_vals[i]);
        for (size_t j = i + 1; j < strlen(new_vals); j++){
            assert(astring_index_at(a, j) == s[j]);
        }
    }

    free_astring(a);
}

// index <= len(s)
void test_insert(char *s, char* t, size_t index){
    size_t res_len = strlen(s) + strlen(t) + 1;
    char *res = malloc(sizeof(char) * res_len);
    if (res == NULL) {
        return;
    }
    
    memcpy(res, s, index);
    memcpy(res + index, t, strlen(t));
    memcpy(res + index + strlen(t), s + index, strlen(s) - index);
    res[res_len - 1] = '\0';

    astring_t *a = string_to_astring(s);
    a = astring_insert(a, index, t);
    char *str = astring_to_string(a);
    assert(strcmp(str, res) == 0);

    free(res);
    free_astring(a);
    free(str);
}

void test_concat(char *s1, char *s2){
    size_t res_len = strlen(s1) + strlen(s2) + 1;
    char *res = malloc(sizeof(char) * res_len);
    memcpy(res, s1, strlen(s1));
    memcpy(res + strlen(s1), s2, strlen(s2));
    res[res_len - 1] = '\0';

    astring_t *a1 = string_to_astring(s1);
    astring_t *a2 = string_to_astring(s2);
    astring_t *astring_res = astring_concat(a1, a2);
    char *str_res = astring_to_string(astring_res);
    
    assert(strcmp(res, str_res) == 0);
    
    free(res);
    free(str_res);
    free_astring(astring_res);
}

void test_iterator(char *s){
    astring_t *a = string_to_astring(s);
    astring_iterator_t *iterator = new_iterator(a);
    astring_t *cur;
    size_t i = 0;
    while(iterator_has_next(iterator)){
        cur = iterator_next(iterator);
        assert(astring_char(cur) == s[i]);
        i++;
    }

    free_astring(a);
    free_iterator(iterator);
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

    test_index_at("Hello World");
    test_index_at("Silly Goose");

    test_update("Hello World", "Goodbye");

    test_insert("Hello", "World", 5);
    test_insert("Herld", "llo Wo", 2);

    test_concat("Hello", " World");
    test_concat("Meow", "");

    test_iterator("Hello world");

    

    printf("All tests passed!\n");
    return 0;
}