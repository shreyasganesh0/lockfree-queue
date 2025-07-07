#ifndef FALSE_SHARING_H
#define FALSE_SHARING_H


#define ITERS 1000000000


struct BadSharedCounter {

    long thread_1_counter;
    long thread_2_counter;

}__attribute__((aligned(64))); 

struct GoodSharedCounter {

    long thread_1_counter;
    char padding[64];
    long thread_2_counter;

}__attribute__((aligned(64))); 

struct FalseGoodSharedCounter {

    long thread_1_counter;
    char padding;
    long thread_2_counter;

}__attribute__((aligned(64))); 

typedef struct BadSharedCounter bad_counter_t;
typedef struct GoodSharedCounter good_counter_t;
typedef struct FalseGoodSharedCounter false_good_counter_t;

typedef struct ThreadCounter {

    void* counter;
    int thread_id;
    int counter_type;
} thread_counter_t;

void *cache_coherence(void *thread_counter);

#endif
