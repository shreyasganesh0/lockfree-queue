#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define ITERS 100000000

volatile int thread_flag_0 = 0;
volatile int thread_flag_1 = 0;
volatile long iteration_counter = 0;

typedef struct {

    int id;
    volatile int reorder_flag;
    volatile int running_flag;
} reorder_t;

void *set_flag(void *id_ret) {

    reorder_t *reorder = (reorder_t *)id_ret;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(reorder->id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    for (long i = 0; i < ITERS; i++) {

        __sync_fetch_and_add(&iteration_counter, 1);
        if (i % 1000 == 0) sched_yield();


        if (reorder->id == 0) {

            thread_flag_0 = 1;
            if (thread_flag_1 == 0) {
                // allowed to enter critical section if other thread isnt in it.
                if (thread_flag_1 == 1) {

                    reorder->reorder_flag = 1;
                    break;
                }
                 // double check if other thread raised flag (it raised its flag after thread 0 check)
            }
            thread_flag_0 = 0;
        } else if (reorder->id == 7) {

            thread_flag_1 = 1;
            if (thread_flag_0 == 0) {

                if (thread_flag_0 == 1) {

                    reorder->reorder_flag = 1;
                    break;
                }
            }
            thread_flag_1 = 0;
        }
    }

    reorder->running_flag = 0;
    return NULL;

}

int main(int argc, char *argv[]) {

    pthread_t thread_0, thread_1;

    reorder_t reorder_t0, reorder_t1;
    reorder_t0.id = 0;
    reorder_t0.reorder_flag = 0;
    reorder_t0.running_flag = 1;
    reorder_t1.id = 7;
    reorder_t1.reorder_flag = 0;
    reorder_t1.running_flag = 1;

    if (pthread_create(&thread_0, NULL, set_flag, &reorder_t0) != 0) {

        perror("Failed to create thread 0");
        exit(1);
    }

    if (pthread_create(&thread_1, NULL, set_flag, &reorder_t1) != 0) {

        perror("Failed to create thread 1");
        exit(1);
    }

    while (reorder_t0.running_flag && reorder_t1.running_flag) {
        
        printf("Program is running...\n");
        printf("Number of iterations run until now: %ld\n", iteration_counter);
        sleep(1);
    }

    if (pthread_join(thread_0, NULL) != 0) {

        perror("Failed to join thread 0");
        exit(1);
    }

    if (pthread_join(thread_1, NULL) != 0) {

        perror("Failed to join thread 1");
        exit(1);
    }

    if (reorder_t0.reorder_flag || reorder_t1.reorder_flag) {

	int reorder_threadid = reorder_t0.reorder_flag == 1 ? 0 : 1;


	printf("\n=== MEMORY REORDERING DETECTED ===\n"
"Iteration: %ld (%.4f%% of maximum)\n\n"
"What happened (chronological order):\n"
"1. Thread %d@Core0: Set flag0 = 1\n"
"2. Thread %d@Core0: Read flag1 = 0 (appears safe to enter)\n"
"3. Thread %d@Core7: Set flag1 = 1\n"  
"4. Thread %d@Core7: Read flag0 = 0 (also appears safe!)\n\n"
"Both threads entered the critical section!\n\n"
"CPU Explanation:\n"
"- Thread %d's read of flag1 executed BEFORE its write to flag0 was visible\n"
"- Thread %d's read of flag0 executed BEFORE its write to flag1 was visible\n"
"- This is Store-Load reordering in action\n\n"
"Without reordering, at least one thread would have seen the other's flag\n",
                iteration_counter, (iteration_counter/(2.0*ITERS)) * 100.0,
				reorder_threadid, reorder_threadid, 
				1 - reorder_threadid, 1 -reorder_threadid,
				reorder_threadid, 1 -reorder_threadid); 
    } else {
        printf("Completed %ld iterations, no reordering occured\n",
                iteration_counter); 
    }
    return 0;
}
