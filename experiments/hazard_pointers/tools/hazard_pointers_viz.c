#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>

typedef struct {

	int id;
	int data;
} node_t;

typedef struct {

	int id;
	size_t data;
} node_p_t;

node_t *hazard_pointers[3][2] = {NULL};
node_t *node[4];

#include "delete_nodes.c"

void init_nodes() {


	node[0] = (node_t*)malloc(sizeof(node_t));
	node[0]->id = 0;
	node[0]->data = 10;

	node[1] = (node_t*)malloc(sizeof(node_t));
	node[1]->id = 1;
	node[1]->data = 40;

	node[2] = (node_t*)malloc(sizeof(node_t));
	node[2]->id = 2;
	node[2]->data = 110;

	node[3] = (node_t*)malloc(sizeof(node_t));
	node[3]->id = 3;
	node[3]->data = 34;

}

void *simulate_reading(void *id) {

	int t_id = *(int *)id;
	node_t *t_node = NULL;
	int hp_idx = 0;

	for (int sim = 0; sim < 4; sim++) {

		int curr_node = rand() % 4;
		int curr_sleep_time = (rand() % 3) + 1;

		t_node = node[curr_node];
		while (!__sync_bool_compare_and_swap(&hazard_pointers[t_id][hp_idx], NULL, t_node)) {};

		sleep(curr_sleep_time);
		
		__sync_lock_release(&hazard_pointers[t_id][hp_idx]);
	}

	return NULL;
}

void print_snapshot() {

	printf("\n=== HAZARD POINTER SNAPSHOT==\n\n");

	printf("Node State:\n");
	for (int i = 0; i < 4; i++) {

		printf("Node %d: addr=%ld, data=%d\n", node[i]->id, (size_t)node[i], node[i]->data);
	}

	int prot_arr[4] = {};
	printf("Hazard Pointer State:\n");
	for (int i = 0; i < 3; i++) {

		int null_count = 0;
		int right = 0;
		node_p_t curr_node_ids[2] = {};
		for (int j = 0; j < 2; j++) {

			if (hazard_pointers[i][j] == NULL) {

				null_count += 1;
				if (j == 1) right = 1;
			} else{

				switch (hazard_pointers[i][j]->id) {

					case 0:
						curr_node_ids[j].id = 0;   
						prot_arr[0] += 1;
						break;
					case 1: 
						curr_node_ids[j].id = 1;   
						prot_arr[1] += 1;
						break;
					case 2: 
						curr_node_ids[j].id = 2;   
						prot_arr[2] += 1;
						break;
					case 3:
						curr_node_ids[j].id = 3;   
						prot_arr[3] += 1;
						break;
					default:
						curr_node_ids[j].id = -1;   
				}

				curr_node_ids[j].data = (size_t)hazard_pointers[i][j]; 
			}
		}		

		if (right == 1 && null_count == 1) null_count++;

		switch (null_count) {

			case 1:
				printf("Thread %d: [%ld, NULL] (protecting node %d)\n", i,
						curr_node_ids[0].data,  
						curr_node_ids[0].id);
				break;
			case 2:
				printf("Thread %d: [NULL, %ld] (protecting node %d)\n", i,
						curr_node_ids[1].data,  
						curr_node_ids[1].id);
				break;
			case 3:
				printf("Thread %d: [%ld, %ld] (protecting node %d and %d)\n", i,
						curr_node_ids[0].data, curr_node_ids[1].data, 
						curr_node_ids[0].id,curr_node_ids[1].id);
				break;
			case 0:
				printf("Thread %d: [NULL, NULL] (not reading anything)\n", i);
				break;
			default:
				printf("Invalid case dont know how null count %d, can be more than 3\n"
						, null_count);
		}

	}

	printf("Protection Summary:\n");
	for (int i = 0; i < 4; i++) {

		if (prot_arr[i] >= 1) {

			printf("Node %d: PROTECTED (%d threads)\n", i, prot_arr[i]);
		} else {

			printf("Node %d: FREE\n", i);
		}
	}

	printf("Deletion Statistics\n");
	printf ("- Blocked: %d\n", deletion_blocked);
	printf ("- Successful: %d\n", deletion_success);

	return;

}

void sleep_ms(int millisecs) {

	struct timespec ts;

	ts.tv_sec = millisecs/1000;
	ts.tv_nsec = (millisecs % 1000) * 1000000000;

	while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

int main(int argc, char *argv[]) {

	srand(time(NULL));
	init_nodes();

	pthread_t thread_0, thread_1, thread_2, thread_3;
	int id0 = 0;
	int id1 = 1;
	int id2 = 2;
	int id3 = 3;

	if (pthread_create(&thread_0, NULL, simulate_reading, &id0) != 0) {

		perror("Failed to create thread 0");
		exit(1);
	} 
	if (pthread_create(&thread_1, NULL, simulate_reading, &id1) != 0) {

		perror("Failed to create thread 1");
		exit(1);
	} 
	if (pthread_create(&thread_2, NULL, simulate_reading, &id2) != 0) {

		perror("Failed to create thread 2");
		exit(1);
	} 
	if (pthread_create(&thread_3, NULL, delete_nodes, &id3) != 0) {

		perror("Failed to create thread 2");
		exit(1);
	} 

	for (int i = 1; i <= 10; i++) {

		printf("\n=== ITERATION %d ===\n\n", i);

		print_snapshot();
		sleep_ms(500);

	}

	if (pthread_join(thread_0, NULL) != 0) {

		perror("Failed to create thread 0");
		exit(1);
	}
	if (pthread_join(thread_1, NULL) != 0) {

		perror("Failed to create thread 1");
		exit(1);
	}
	if (pthread_join(thread_2, NULL) != 0) {

		perror("Failed to create thread 2");
		exit(1);
	}
	if (pthread_join(thread_3, NULL) != 0) {

		perror("Failed to create thread 3");
		exit(1);
	}
}
