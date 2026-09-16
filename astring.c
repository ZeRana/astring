#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>

#include "contracts.h"
#include "astring.h"

#define THREAD_DEPTH_MAX 8

//@TODO add index tracking to fix a lot of big O bounds

// Limiting recursive functions thread making abilities
typedef struct {
    uint8_t num_threads;
    pthread_mutex_t lock;
} thread_limiter_t;

// This is ideal for stack based creation
// If not being used on the stack then malloc an object before passing into
void new_thread_limiter (thread_limiter_t *tl){
    tl->num_threads = 0;
    pthread_mutex_init(&tl->lock, NULL);
}

// Assumes stack based creation
// Call free on tl if this was heap based
void free_thread_limiter (thread_limiter_t *tl) {
    pthread_mutex_destroy(&tl->lock);
}


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
    size_t index;
    char data;
} astring_t;

bool is_astring (astring_t *a){
    if (a == NULL) {return true;}

    size_t leftsize = 0;
    size_t rightsize = 0;

    if (a->left != NULL) {
        leftsize = a->left->size;
        if (a->index <= a->left->index) {return false;}
    }
    if (a->right != NULL){
        rightsize = a->right->size;
        if (a->index >= a->right->index) {return false;}
    }

    if (a->size != rightsize + leftsize + 1) {return false;}
    size_t diff = (leftsize > rightsize) ? (leftsize - rightsize) : (rightsize - leftsize);
    if (diff > 1) {return false;}

    return is_astring(a->left) && is_astring(a->right);

}

// O(1)
inline size_t astring_size(astring_t *a){
    REQUIRES(is_astring(a));
    // NULL is the empty string
    if (a == NULL) {return 0;}
    return a->size;
}

typedef struct {
    char *str;
    size_t size;
    size_t offset;
    thread_limiter_t *tl;
    astring_t *result;
} to_string_helper_t;

astring_t *string_to_astring_helper(char *str, size_t size, size_t offset, thread_limiter_t *tl);

void* string_to_astring_thread(void *args){
    to_string_helper_t *arg = (to_string_helper_t*)args;
    arg->result = string_to_astring_helper(arg->str, arg->size, arg->offset, arg->tl);
    return (void*)arg;
}

// O(size)
astring_t *string_to_astring_helper(char *str, size_t size, size_t offset, thread_limiter_t *tl){
    if (size == 0) {return NULL;}
    ASSERT(str != NULL);
    size_t mid = size/2;
    astring_t *a = xmalloc(sizeof(astring_t));
    a->size = size;
    a->data = *(str + mid);
    a->index = offset + mid;

    // Checking to insure we don't over utilize resources
    pthread_mutex_lock(&tl->lock);
    bool spawn = (tl->num_threads < THREAD_DEPTH_MAX);
    if (spawn) {tl->num_threads++;}
    pthread_mutex_unlock(&tl->lock);

    if(!spawn){
        a->left = string_to_astring_helper(str, mid, offset, tl);
        a->right = string_to_astring_helper((str + mid + 1), (size-mid-1), (offset + mid + 1), tl);
        ENSURES(is_astring(a));
        return a;
    } else {
        to_string_helper_t left_args = {str, mid, offset, tl, NULL};
        pthread_t tid;
        int success = pthread_create(&tid, NULL, string_to_astring_thread, &left_args);
        if (success == 0){
            // Thread created
            pthread_join(tid, NULL);
            a->left = left_args.result;
        } else {
            // Thread not created
            a->left = string_to_astring_helper(str, mid, offset, tl);
        }
        a->right = string_to_astring_helper((str + mid + 1), (size-mid-1), (offset + mid + 1), tl);
        pthread_mutex_lock(&tl->lock);
        tl->num_threads--;
        pthread_mutex_unlock(&tl->lock);
        return a;
    }


}

// O(strlen(str))
astring_t *string_to_astring(char *str){
    thread_limiter_t tl;
    new_thread_limiter(&tl);
    REQUIRES(str != NULL);
    size_t len = strlen(str);
    astring_t *a = string_to_astring_helper(str, len, 0, &tl);
    free_thread_limiter(&tl);
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
    char *str = xmalloc((astring_size(a) + 1) * sizeof(char));
    astring_to_string_helper(a, str, 0);
    str[astring_size(a)] = '\0';
    return str;
}

typedef struct {
    astring_t *a1;
    astring_t *a2;
    thread_limiter_t *tl;
    bool result;
} eq_helper_t;

bool astring_eq_helper(astring_t *a1, astring_t *a2, thread_limiter_t *tl);

void* astring_eq_thread(void *args){
    eq_helper_t *arg = (eq_helper_t*)args;
    arg->result = astring_eq_helper(arg->a1, arg->a2, arg->tl);
    return (void*)arg;
}

bool astring_eq_helper(astring_t *a1, astring_t *a2, thread_limiter_t *tl){
    REQUIRES(is_astring(a1));
    REQUIRES(is_astring(a2));
    if (a1 == NULL){return a2 == NULL;}
    else if (a2 == NULL) {return false;}
    if (a1->data != a2->data) {return false;}

    // Checking to insure we don't over utilize resources
    pthread_mutex_lock(&tl->lock);
    bool spawn = (tl->num_threads < THREAD_DEPTH_MAX);
    if (spawn) {tl->num_threads++;}
    pthread_mutex_unlock(&tl->lock);
    if (!spawn){
        return astring_eq_helper(a1->left, a2->left, tl) & astring_eq_helper(a1->right, a2->right, tl);
    } else {
        eq_helper_t left_args = { a1->left, a2->left, tl, false };
        pthread_t tid;
        int success = pthread_create(&tid, NULL, astring_eq_thread, &left_args);
        bool left_result = false;
        if (success == 0){
            // Thread created
            pthread_join(tid, NULL);
            left_result = left_args.result;
        } else {
            // Thread not created
            left_result = astring_eq_helper(a1->left, a2->left, tl);
        }

        bool right_result = astring_eq_helper(a1->right, a2->right, tl);

        pthread_mutex_lock(&tl->lock);
        tl->num_threads--;
        pthread_mutex_unlock(&tl->lock);
        return left_result & right_result;
    }
}

bool astring_eq(astring_t *a1, astring_t *a2){
    thread_limiter_t tl;
    new_thread_limiter(&tl);
    bool res = astring_eq_helper(a1, a2, &tl);
    free_thread_limiter(&tl);
    return res;  
}

// Returns a pointer to the node for future inserting use
astring_t *astring_index_at_helper(astring_t *a, size_t i){
    REQUIRES(is_astring(a));
    if (a == NULL){
        return NULL;
    } else if (a->index == i) { 
        return a;
    } else if (a->index > i) {
        return astring_index_at_helper(a->left, i);
    } else {
        return astring_index_at_helper(a->right, i);
    }
}

char astring_index_at(astring_t *a, size_t i){
    REQUIRES(is_astring(a));
    astring_t *elem = astring_index_at_helper(a, i);
    if (elem == NULL){
        return '\0';
    } else {
        return elem->data;
    }

}


// Insertion logic was very heavily inspired by CMU's 15122 lecture slides
// inline void fix_size(astring_t *a){
//     if (a != NULL){
//         a->size = 1 + max(astring_size(a->left), astring_size(a->right));
//     }
// }

// astring_t* rotate_left(astring_t *a){
//     astring_t *temp = a->right;
//     a->right = a->right->left;
//     temp->left = a;
//     fix_size(a);
//     fix_size(temp);
//     return temp;
// }

// astring_t *rotate_right(astring_t *a){
//     astring_t *temp = a->left;
//     a->left = a->left->right;
//     temp->right = a;
//     fix_size(a);
//     fix_size(temp);
//     return temp;
// }

// astring_t *rebalance_right(astring_t *a){
//     if (astring_size(a->right) - astring_size(a->left) == 2){
//         if (astring_size(a->right->right) > astring_size(a->right->left)){
//             a = rotate_left(a);
//         } else {
//             a->right = rotate_right(a->right);
//             a = rotate_left(a);
//         }
//     } else {
//         fix_size(a);
//     }
//     return a;
// }

// astring_t *rebalance_left(astring_t *a){
//     if (astring_size(a->left) - astring_size(a->right) == 2) {
//         if (astring_size(a->left->left) > astring_size(a->left->right)) {
//             a = rotate_right(a);
//         } else {
//             a->left = rotate(a->left);
//             a = rotate_right(a);
//         }
//     } else {
//         fix_size(a);
//     }

//     return a;
// }





