#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>

#include "contracts.h"
#include "astring.h"

#define THREAD_DEPTH_MAX 8

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
    size_t height;
    char data;
} astring_t;

bool is_astring (astring_t *a){
    if (a == NULL) {return true;}

    size_t left_size = 0;
    size_t right_size = 0;
    size_t left_height = 0;
    size_t right_height = 0;

    if (a->left != NULL) {
        left_size = a->left->size;
    }
    if (a->right != NULL){
        right_size = a->right->size;
    }

    if (a->size != right_size + left_size + 1) {return false;}
    size_t diff = (left_size > right_size) ? (left_size - right_size) : (right_size - left_size);
    if (diff > 1) {return false;}

    size_t hdiff = (left_height > right_height) ? (left_height - right_height) : (right_height - left_height);
    if (hdiff > 1) {return false;}
    if (a->height != 1 + ((left_height > right_height) ? left_height : right_height)) {return false;}

    return is_astring(a->left) && is_astring(a->right);

}

// O(1)
inline size_t astring_size(astring_t *a){
    REQUIRES(is_astring(a));
    // NULL is the empty string
    if (a == NULL) {return 0;}
    return a->size;
}

size_t astring_height(astring_t *a){
    REQUIRES(is_astring(a));
    if (a == NULL) {return 0;}
    return a->height;
}

void fix_stats(astring_t *a){
    if (a != NULL){
        size_t lh = astring_height(a->left);
        size_t rh = astring_height(a->right);
        a->size = 1 + astring_size(a->left) + astring_size(a->right);
        a->height = 1 + (lh > rh ? lh : rh);
    }
}

typedef struct {
    char *str;
    size_t size;
    thread_limiter_t *tl;
    astring_t *result;
} to_string_helper_t;

astring_t *string_to_astring_helper(char *str, size_t size, thread_limiter_t *tl);

void* string_to_astring_thread(void *args){
    to_string_helper_t *arg = (to_string_helper_t*)args;
    arg->result = string_to_astring_helper(arg->str, arg->size, arg->tl);
    return (void*)arg;
}

// O(size)
astring_t *string_to_astring_helper(char *str, size_t size, thread_limiter_t *tl){
    if (size == 0) {return NULL;}
    ASSERT(str != NULL);
    size_t mid = size/2;
    astring_t *a = xmalloc(sizeof(astring_t));
    a->data = *(str + mid);

    // Checking to insure we don't over utilize resources
    pthread_mutex_lock(&tl->lock);
    bool spawn = (tl->num_threads < THREAD_DEPTH_MAX);
    if (spawn) {tl->num_threads++;}
    pthread_mutex_unlock(&tl->lock);

    if(!spawn){
        a->left = string_to_astring_helper(str, mid, tl);
        a->right = string_to_astring_helper((str + mid + 1), (size-mid-1), tl);
        fix_stats(a);
        ENSURES(is_astring(a));
        return a;
    } else {
        to_string_helper_t left_args = {str, mid, tl, NULL};
        pthread_t tid;
        int success = pthread_create(&tid, NULL, string_to_astring_thread, &left_args);
        if (success == 0){
            // Thread created
            pthread_join(tid, NULL);
            a->left = left_args.result;
        } else {
            // Thread not created
            a->left = string_to_astring_helper(str, mid, tl);
        }
        a->right = string_to_astring_helper((str + mid + 1), (size-mid-1), tl);
        pthread_mutex_lock(&tl->lock);
        tl->num_threads--;
        pthread_mutex_unlock(&tl->lock);
        fix_stats(a);
        ENSURES(is_astring(a));
        return a;
    }
}

// O(strlen(str))
astring_t *string_to_astring(char *str){
    thread_limiter_t tl;
    new_thread_limiter(&tl);
    REQUIRES(str != NULL);
    size_t len = strlen(str);
    astring_t *a = string_to_astring_helper(str, len, &tl);
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
// O(log n)
astring_t *astring_index_at_helper(astring_t *a, size_t i){
    REQUIRES(is_astring(a));
    REQUIRES(i < astring_size(a));
    if (a == NULL){
        return NULL;
    } 
    size_t left_size = astring_size(a->left);
    if (i == left_size) {
        return a;
    }
    else if (i < left_size) {
        return astring_index_at_helper(a->left, i);
    } else {
        return astring_index_at_helper(a->right, i - left_size - 1);
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

void astring_update_char(astring_t *a, size_t i, char new_val){
    REQUIRES(is_astring(a));
    astring_t *elem = astring_index_at_helper(a, i);
    if (elem != NULL){
        elem->data = new_val;
    }
}

astring_t* rotate_left(astring_t *a){
    astring_t *temp = a->right;
    a->right = a->right->left;
    temp->left = a;
    fix_stats(a);
    fix_stats(temp);
    return temp;
}

astring_t *rotate_right(astring_t *a){
    astring_t *temp = a->left;
    a->left = a->left->right;
    temp->right = a;
    fix_stats(a);
    fix_stats(temp);
    return temp;
}

astring_t *rebalance_right(astring_t *a){
    if (astring_height(a->right) - astring_height(a->left) == 2){
        if (astring_height(a->right->right) >= astring_height(a->right->left)){
            a = rotate_left(a);
        } else {
            a->right = rotate_right(a->right);
            a = rotate_left(a);
        }
    } else {
        fix_stats(a);
    }
    return a;
}

astring_t *rebalance_left(astring_t *a){
    if (astring_height(a->left) - astring_height(a->right) == 2) {
        if (astring_height(a->left->left) >= astring_height(a->left->right)) {
            a = rotate_right(a);
        } else {
            a->left = rotate_left(a->left);
            a = rotate_right(a);
        }
    } else {
        fix_stats(a);
    }
    return a;
}


astring_t *astring_pop_last(astring_t *a, astring_t **out_node);


astring_t *astring_join(astring_t *L, astring_t *x, astring_t *R){
    REQUIRES(is_astring(L));
    REQUIRES(is_astring(R));

    if (x == NULL){
        if (L == NULL) {return R;}
        astring_t *pivot;
        astring_t *rest = astring_pop_last(L, &pivot);
        astring_t *result = astring_join(rest, pivot, R);
        ENSURES(is_astring(result));
        return result;
    }

    size_t left_height = astring_height(L);
    size_t right_height = astring_height(R);

    if (left_height > right_height + 1){
        L->right = astring_join(L->right, x, R);
        fix_stats(L);
        astring_t *result = rebalance_right(L);
        ENSURES(is_astring(result));
        return result;
    } else if (right_height > left_height + 1){
        R->left = astring_join(L, x, R->left);
        fix_stats(R);
        astring_t *result = rebalance_left(R);
        ENSURES(is_astring(result));
        return result;
    } else {
        x->left = L;
        x->right = R;
        fix_stats(x);
        ENSURES(is_astring(x));
        return x;
    }
}

typedef struct { 
    astring_t *left; 
    astring_t *right; 
} astring_pair_t;

// L = positions [0, i), R = positions [i, size(a)). O(log(size(a)))
astring_pair_t astring_split(astring_t *a, size_t i){
    REQUIRES(is_astring(a));
    REQUIRES(i <= astring_size(a));

    if (a == NULL) {astring_pair_t r = {NULL, NULL}; return r;}

    size_t left_size = astring_size(a->left);

    if (i <= left_size){
        astring_pair_t sub = astring_split(a->left, i);
        astring_t *right_piece = astring_join(sub.right, a, a->right);
        astring_pair_t result = {sub.left, right_piece};
        return result;
    } else {
        astring_pair_t sub = astring_split(a->right, i - left_size - 1);
        astring_t *left_piece = astring_join(a->left, a, sub.left);
        astring_pair_t result = {left_piece, sub.right};
        return result;
    }
}

//O(log n)
// Need some way to get a joiner node for astring_join
astring_t *astring_pop_last(astring_t *a, astring_t **out_node){
    REQUIRES(a != NULL);
    if (a->right == NULL){
        *out_node = a;
        return a->left;
    }
    a->right = astring_pop_last(a->right, out_node);
    fix_stats(a);
    astring_t *result = rebalance_left(a);
    ENSURES(is_astring(result));
    return result;
}


typedef struct {
    const char *str; 
    size_t size; 
    thread_limiter_t *tl; 
    astring_t *result;
} build_helper_t;

astring_t *astring_build_helper(const char *str, size_t size, thread_limiter_t *tl);

void *astring_build_thread(void *args){
    build_helper_t *arg = (build_helper_t*)args;
    arg->result = astring_build_helper(arg->str, arg->size, arg->tl);
    return NULL;
}

astring_t *astring_build_helper(const char *str, size_t size, thread_limiter_t *tl){
    if (size == 0) {return NULL;}
    size_t mid = size / 2;
    astring_t *a = xmalloc(sizeof(astring_t));
    a->data = str[mid];   // size/height set by fix_stats below, not here

    pthread_mutex_lock(&tl->lock);
    bool spawn = (tl->num_threads < THREAD_DEPTH_MAX);
    if (spawn) {tl->num_threads++;}
    pthread_mutex_unlock(&tl->lock);

    if (!spawn){
        a->left  = astring_build_helper(str, mid, tl);
        a->right = astring_build_helper(str + mid + 1, size - mid - 1, tl);
    } else {
        build_helper_t left_args = {str, mid, tl, NULL};
        pthread_t tid;
        int success = pthread_create(&tid, NULL, astring_build_thread, &left_args);
        if (success == 0){
            a->right = astring_build_helper(str + mid + 1, size - mid - 1, tl);
            pthread_join(tid, NULL);
            a->left = left_args.result;
        } else {
            a->left  = astring_build_helper(str, mid, tl);
            a->right = astring_build_helper(str + mid + 1, size - mid - 1, tl);
        }
        pthread_mutex_lock(&tl->lock);
        tl->num_threads--;
        pthread_mutex_unlock(&tl->lock);
    }
    fix_stats(a);
    ENSURES(is_astring(a));
    return a;
}

astring_t *astring_build(const char *str, size_t n){
    if (n == 0) {return NULL;}
    thread_limiter_t tl;
    new_thread_limiter(&tl);
    astring_t *a = astring_build_helper(str, n, &tl);
    free_thread_limiter(&tl);
    return a;
}

// O(log(size(a)) + len(str))
// str must be NULL terminated
astring_t *astring_insert(astring_t *a, size_t i, const char *str){
    REQUIRES(is_astring(a));
    REQUIRES(i <= astring_size(a));
    if (str == NULL) {
        return a;
    }
    size_t n = strlen(str);

    astring_pair_t halves = astring_split(a, i);
    astring_t *mid = astring_build(str, n);
    astring_t *result = astring_join(astring_join(halves.left, NULL, mid), NULL, halves.right);

    ENSURES(is_astring(result));
    return result;
}




