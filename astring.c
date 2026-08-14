#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "contracts.h"
#include "astring.h"

void *xmalloc(size_t size){
    void *p = malloc(size);
    if (p == NULL) {abort();}
    return p;
}
void *xcalloc(size_t num_elements, size_t size){
    void *p = calloc(num_elements, size);
    if (p == NULL) {abort();}
    return p;
}

typedef struct astring_header {
    struct astring_header *left;
    struct astring_header *right;
    size_t size;
    char data;
} astring_t;

bool is_astring (astring_t *a){
    if (a == NULL) {return true;}

    size_t leftsize = 0;
    size_t rightsize = 0;

    if (a->left != NULL) {
        leftsize = a->left->size;
    }
    if (a->right != NULL){
        rightsize = a->right->size;
    }

    if (a->size != rightsize + leftsize + 1) {return false;}
    size_t diff = (leftsize > rightsize) ? (leftsize - rightsize) : (rightsize - leftsize);
    if (diff > 1) {return false;}

    return is_astring(a->left) && is_astring(a->right);

}

// O(1)
size_t astring_size(astring_t *a){
    REQUIRES(is_astring(a));
    // NULL is the empty string
    if (a == NULL) {return 0;}
    return a->size;
}

// O(size)
astring_t *string_to_astring_helper(char *str, size_t size){
    if (size == 0) {return NULL;}
    ASSERT(str != NULL);
    size_t mid = size/2;
    astring_t *a = xmalloc(sizeof(astring_t));
    a->size = size;
    a->data = *(str + mid);
    a->left = string_to_astring_helper(str, mid);
    a->right = string_to_astring_helper((str + mid + 1), (size-mid-1));
    ENSURES(is_astring(a));
    return a;
}

// O(strlen(str))
astring_t *string_to_astring(char *str){
    REQUIRES(str != NULL);
    size_t len = strlen(str);
    astring_t *a = string_to_astring_helper(str, len);
    ENSURES(is_astring(a));
    return a;
}

// O(a->size)
void free_astring(astring_t *a){
    if(a == NULL) {(void)0;}
    else if (a->size == 1) {free(a);}
    else{
        free_astring(a->left);
        free_astring(a->right);
        free(a);
    }
}

// O(a->size)
void print_astring(astring_t *a){
    REQUIRES(is_astring(a));
    if (a == NULL) {(void)0;}
    else{
        print_astring(a->left);
        printf("%c", a->data);
        print_astring(a->right);
    }
}

size_t astring_to_string_helper(astring_t *a, char *str, size_t index){
    REQUIRES(is_astring(a));
    if (a == NULL) {return index;}
    else{
        index = astring_to_string_helper(a->left, str, index);
        str[index] = a->data;
        index++;
        index = astring_to_string_helper(a->right, str, index);
        return index;
    }

}

char *astring_to_string(astring_t *a){
    REQUIRES(is_astring(a));
    char *str = xmalloc((a->size + 1) * sizeof(char));
    astring_to_string_helper(a, str, 0);
    str[a->size] = '\0';
    return str;
}

