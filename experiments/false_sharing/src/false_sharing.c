#include <sched.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "false_sharing.h"


void *cache_coherence(void *thread_counter) {

    thread_counter_t *counter_thread = (thread_counter_t *)thread_counter;

    good_counter_t *good_counter;
    bad_counter_t *bad_counter;
    false_good_counter_t *false_good_counter;
    struct timespec start, end;
    volatile long *counter;

    switch (counter_thread->counter_type) {

        case 0:
            bad_counter = (bad_counter_t*)counter_thread->counter;
            if (counter_thread->thread_id == 0) {

                counter = &bad_counter->thread_1_counter;
            } else {

                counter = &bad_counter->thread_2_counter;
            }
            break;
        case 1:
            good_counter = (good_counter_t*)counter_thread->counter;
            if (counter_thread->thread_id == 0) {

                counter = &good_counter->thread_1_counter;
            } else {

                counter = &good_counter->thread_2_counter;
            }
            break;
        case 2:
            false_good_counter = (false_good_counter_t*)counter_thread->counter;
            if (counter_thread->thread_id == 0) {

                counter = &false_good_counter->thread_1_counter;
            } else {

                counter = &false_good_counter->thread_2_counter;
            }
            break;
        default:
            perror("Fatal error");
            exit(1);
    }


    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(counter_thread->thread_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < ITERS; i++) {
        long temp = *counter; 
        __asm__ volatile("nop; nop; nop; nop;");
        (*counter) = temp + 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec + (end.tv_nsec / 1000000000.0)) 
        - (start.tv_sec + (start.tv_nsec / 1000000000.0));

    printf("Time taken for thread %c to complete = %.6f\n", 
            counter_thread->thread_id, elapsed); 

    return NULL;
}

int main(int argc, char *argv[]) {

    thread_counter_t thread_counter_1;
    thread_counter_t thread_counter_2;

    int counter_flag = 0;
    thread_counter_1.thread_id = 0;
    thread_counter_2.thread_id = 1;

    if (argc == 2) {

        if (strcmp(argv[1], "good") == 0) {

                thread_counter_1.counter_type = 1;
                thread_counter_2.counter_type = 1;
            printf("Size of good struct = %zu bytes\n", sizeof(good_counter_t));
        } else if (strcmp(argv[1], "bad") == 0) {

                thread_counter_1.counter_type = 0;
                thread_counter_2.counter_type = 0;
            printf("Size of bad struct = %zu bytes\n", sizeof(bad_counter_t));
        } else {

            printf("Invalid args");
            exit(1);
        }

    } else if (argc == 3) {

        thread_counter_1.counter_type = 2;
        thread_counter_2.counter_type = 2;

        if (strcmp(argv[2], "cross-ccx") == 0) {

            thread_counter_2.thread_id = 8; //to put it into a different ccx
            printf("Size of good struct = %zu bytes\n", sizeof(false_good_counter_t));

        } else if (strcmp(argv[2], "same-ccx") == 0) {

            printf("Size of struct = %zu bytes\n", sizeof(false_good_counter_t));
        } else {

            printf("Invalid args");
            exit(1);
        }

    } else {

        printf("Invalid args");
        exit(1);
    }


    switch(thread_counter_1.counter_type) {

        case 0:
            {
                thread_counter_1.counter = malloc(sizeof(bad_counter_t));
                thread_counter_2.counter = thread_counter_1.counter;

                if (thread_counter_1.counter == NULL) {
                    perror("Failed to allocate to counter");
                    exit(1);
                }
                printf("Address of counter 1 is %p\n",
                    &((bad_counter_t*)thread_counter_1.counter)->thread_1_counter);

                printf("Address of counter 2 is %p\n",
                     &((bad_counter_t*)thread_counter_1.counter)->thread_2_counter);

                printf("Distance between counter 1 and counter 2 is %ld bytes\n",
                    (char*)&((bad_counter_t*)thread_counter_1.counter)->thread_2_counter 
                   - (char*)&((bad_counter_t*)thread_counter_1.counter)->thread_1_counter);

                pthread_t bad_thread_1, bad_thread_2;

                if(pthread_create(&bad_thread_1, NULL, cache_coherence, &thread_counter_1) != 0) {

                    perror("Failed to create thread 1");
                    exit(1);
                }

                if(pthread_create(&bad_thread_2, NULL, cache_coherence, &thread_counter_2) != 0) {

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
                thread_counter_1.counter = malloc(sizeof(good_counter_t));
                thread_counter_2.counter = thread_counter_1.counter;

                if (thread_counter_1.counter == NULL) {
                    perror("Failed to allocate to counter");
                    exit(1);
                }

                printf("Address of counter 1 is %p\n",
                        &((good_counter_t*)thread_counter_1.counter)->thread_1_counter);

                printf("Address of counter 2 is %p\n",
                        &((good_counter_t*)thread_counter_1.counter)->thread_2_counter);

                printf("Distance between counter 1 and counter 2 is %ld bytes\n",
                (char*)&((good_counter_t*)thread_counter_1.counter)->thread_2_counter 
                - (char*)&((good_counter_t*)thread_counter_1.counter)->thread_1_counter);

                pthread_t good_thread_1, good_thread_2;

                if(pthread_create(&good_thread_1, NULL, cache_coherence, &thread_counter_1) != 0) {

                    perror("Failed to create thread 1");
                    exit(1);
                }

                if(pthread_create(&good_thread_2, NULL, cache_coherence, &thread_counter_2) != 0) {

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

        case 2:

            {
                thread_counter_1.counter = malloc(sizeof(false_good_counter_t));
                thread_counter_2.counter = thread_counter_1.counter;

                if (thread_counter_1.counter == NULL) {
                    perror("Failed to allocate to counter");
                    exit(1);
                }

                printf("Address of counter 1 is %p\n", 
                     &((false_good_counter_t*)thread_counter_1.counter)->thread_1_counter);

                printf("Address of counter 2 is %p\n",
                    &((false_good_counter_t*)thread_counter_1.counter)->thread_2_counter);

                printf("Distance between counter 1 and counter 2 is %ld bytes\n", 
            (char*)&((false_good_counter_t*)thread_counter_1.counter)->thread_2_counter 
            - (char*)&((false_good_counter_t*)thread_counter_1.counter)->thread_1_counter);

                pthread_t good_thread_1, good_thread_2;

                if(pthread_create(&good_thread_1, NULL, cache_coherence, &thread_counter_1) != 0) {

                    perror("Failed to create thread 1");
                    exit(1);
                }

                if(pthread_create(&good_thread_2, NULL, cache_coherence, &thread_counter_2) != 0) {

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
