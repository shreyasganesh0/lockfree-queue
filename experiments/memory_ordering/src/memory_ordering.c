#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define ITERS 1000000

volatile int thread_flag_0 = 0;
volatile int thread_flag_1 = 0;

typedef struct {

    int id;
    int reorder_flag;
} reorder_t;

void *set_flag(void *id_ret) {

    reorder_t *reorder = (reorder_t *)id_ret;

    for (int i = 0; i < ITERS; i++) {
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
        } else if (reorder->id == 1) {

            thread_flag_1 = 1;
            if (thread_flag_0 == 0) {

                if (thread_flag_0 == 1) {

                    printf("Instruction has been reordered!\n");
                    reorder->reorder_flag = 1;
                    break;
                }
            }
            thread_flag_1 = 0;
        }
    }
    return NULL;

}

int main(int argc, char *argv[]) {

    pthread_t thread_0, thread_1;

    reorder_t reorder_t0, reorder_t1;
    reorder_t0.id = 0;
    reorder_t0.reorder_flag = 0;
    reorder_t1.id = 1;
    reorder_t1.reorder_flag = 0;

    if (pthread_create(&thread_0, NULL, set_flag, &reorder_t0) != 0) {

        perror("Failed to create thread 0");
        exit(1);
    }

    if (pthread_create(&thread_1, NULL, set_flag, &reorder_t1) != 0) {

        perror("Failed to create thread 1");
        exit(1);
    }

    if (pthread_join(thread_0, NULL) != 0) {

        perror("Failed to create thread 0");
        exit(1);
    } 

    if (pthread_join(thread_1, NULL) != 0) {

        perror("Failed to create thread 1");
        exit(1);
    } 

    if (reorder_t0.reorder_flag || reorder_t1.reorder_flag) {

        printf("Instruction has been reordered!\n");
    } else {
        printf("Completed 1000000 iterations, no reordering occured\n");
    }
    return 0;
}
