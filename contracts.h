#include <stdlib.h>
#include <stdio.h>

#ifndef CONTRACTS_H
#define CONTRACTS_H
#ifdef DEBUG

#define ASSERT(COND) \
    do { \
        if (!(COND)) { \
            fprintf(stderr, "ASSERT failed! Condition (%s) failed on line %d in file %s\n", \
                    #COND, __LINE__, __FILE__); \
            abort(); \
        } \
    } while(0)

#define REQUIRES(COND) \
    do { \
        if (!(COND)) { \
            fprintf(stderr, "REQUIRES failed! Condition (%s) failed on line %d in file %s\n", \
                    #COND, __LINE__, __FILE__); \
            abort(); \
        } \
    } while(0)
    
#define ENSURES(COND) \
    do { \
        if (!(COND)) { \
            fprintf(stderr, "ENSURES failed! Condition (%s) failed on line %d in file %s\n", \
                    #COND, __LINE__, __FILE__); \
            abort(); \
        } \
    } while(0)

#define DPRINT(...) (printf(__VA_ARGS__))
#else

#define ASSERT(COND) ((void)0)
#define REQUIRES(COND) ((void)0)
#define ENSURES(COND) ((void)0)
#define DPRINT(...) ((void)0)

#endif
#endif