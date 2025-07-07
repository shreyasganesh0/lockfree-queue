#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

struct BadSharedCounter {

    long thread_1_counter;
    long thread_2_counter;

}__attribute((aligned(64))); 

struct GoodSharedCounter {

    long thread_1_counter;
    char padding[64];
    long thread_2_counter;

}__attribute((aligned(64))); 

typedef BadSharedCounter bad_counter_t;
typedef GoodSharedCounter good_counter_t;

bad_counter_t *bad_shared_counter;
good_counter_t *good_shared_counter;

void *cache_line_battle(void *id) {

    int *t_id = (int *)id;
    struct timespec start, end;
    volatile long *counter;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (*t_id == 1) {

        counter = &bad_shared_counter->thread_1_counter;
    } else {

        counter = &bad_shared_counter->thread_2_counter;
    }

    for (long i = 0; i < 10000000000; i++) {
        (*counter)++;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec + (end.tv_nsec / 1000000000.0)) - (start.tv_sec + (start.tv_nsec / 1000000000.0));
    printf("Time taken for thread %c to complete = %.6f\n", *(char *)id, elapsed); 

    return NULL;
}

void *good_cache_coherence(void *id) {

    int *t_id = (int *)id;

    struct timespec start, end;
    volatile long *counter;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (*t_id == 1) {

        counter = &good_shared_counter->thread_1_counter;
    } else {

        counter = &good_shared_counter->thread_2_counter;
    }

    for (long i = 0; i < 10000000; i++) {
        (*counter)++;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec + (end.tv_nsec / 1000000000.0)) - (start.tv_sec + (start.tv_nsec / 1000000000.0));
    printf("Time taken for thread %c to complete = %.6f\n", *(char *)id, elapsed); 

    return NULL;
}

int main(int argc, char *argv[]) {

    int counter_flag = 0;
    if (argc == 2) {
        if (strcmp(argv[1], "good") == 0) {

            counter_flag = 1;
            printf("Size of good struct = %zu bytes\n", sizeof(good_counter_t));
        } else if (strcmp(argv[1], "bad") == 0) {

            counter_flag = 0;
            printf("Size of bad struct = %zu bytes\n", sizeof(bad_counter_t));
        }
    } else {

        printf("Invalid args");
        exit(1);
    }


    switch(counter_flag) {

        case 0:
            {
                bad_shared_counter = (bad_counter_t*)malloc(sizeof(bad_counter_t));
                if (bad_shared_counter == NULL) {
                    perror("Failed to allocate to counter");
                    exit(1);
                }
                bad_shared_counter->thread_1_counter = 0;
                bad_shared_counter->thread_2_counter = 0;
                printf("Address of counter 1 is %p\n", &bad_shared_counter->thread_1_counter);
                printf("Address of counter 2 is %p\n", &bad_shared_counter->thread_2_counter);
                printf("Distance between counter 1 and counter 2 is %ld bytes\n", (char*)&bad_shared_counter->thread_2_counter - (char*)&bad_shared_counter->thread_1_counter);

                pthread_t bad_thread_1, bad_thread_2;
                int id1 = 1;
                int id2 = 2;

                if(pthread_create(&bad_thread_1, NULL, cache_line_battle, &id1) != 0) {

                    perror("Failed to create thread 1");
                    exit(1);
                }

                if(pthread_create(&bad_thread_2, NULL, cache_line_battle, &id2) != 0) {

                    perror("Failed to create thread 2");
                    exit(1);
                }

                if (pthread_join(bad_thread_1, NULL) != 0) {

                    perror("Failed to join thread 1");
                    exit(1);
                }

                if (pthread_join(bad_thread_2, NULL) != 0) {

                    perror("Failed to join thread 2");
                    exit(1);
                }
            }
            break;

        case 1:

            {
                good_shared_counter = (good_counter_t*)malloc(sizeof(good_counter_t));
                if (good_shared_counter == NULL) {
                    perror("Failed to allocate to counter");
                    exit(1);
                }
                good_shared_counter->thread_1_counter = 0;
                good_shared_counter->thread_2_counter = 0;

                printf("Address of counter 1 is %p\n", &good_shared_counter->thread_1_counter);
                printf("Address of counter 2 is %p\n", &good_shared_counter->thread_2_counter);
                printf("Distance between counter 1 and counter 2 is %ld bytes\n", (char*)&good_shared_counter->thread_2_counter - (char*)&good_shared_counter->thread_1_counter);

                pthread_t good_thread_1, good_thread_2;
                int id1 = 1;
                int id2 = 2;

                if(pthread_create(&good_thread_1, NULL, good_cache_coherence, &id1) != 0) {

                    perror("Failed to create thread 1");
                    exit(1);
                }

                if(pthread_create(&good_thread_2, NULL, good_cache_coherence, &id2) != 0) {

                    perror("Failed to create thread 2");
                    exit(1);
                }

                if (pthread_join(good_thread_1, NULL) != 0) {

                    perror("Failed to join thread 1");
                    exit(1);
                }

                if (pthread_join(good_thread_2, NULL) != 0) {

                    perror("Failed to join thread 2");
                    exit(1);
                }
            }
            break;

        default :

            perror("Something went wrong");
            exit(1);
    }

    return 0;
}

