#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

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

	hazard_pointers[1][0] = node[2];
	hazard_pointers[1][1] = node[1];
	hazard_pointers[2][0] = node[0];

}

void *simulate_reading(void *id) {

	int t_id = *(int *)id;
	node_t *t_node = NULL;

	switch (t_id) {

		case 0:

			t_node = node[0];
			printf("Thread 0: currently reading node 0 with value %d\n", t_node->data);
			break;

		case 1:

			t_node = node[2];
			printf("Thread 1: currently reading node 2 with value %d\n", t_node->data);
			break;

		case 2:

			printf("Thread 2: not reading anything (idle)\n");
			break;

		default:
			printf("invalid thread id");
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
		node_p_t curr_node_ids[2];
		for (int j = 0; j < 2; j++) {

			if (hazard_pointers[i][j] == NULL) {

				null_count = j + 1;
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

		switch (null_count) {

			case 1:
				printf("Thread %d: [%ld, NULL] (protecting node %d\n", i,
						curr_node_ids[0].data,  
						curr_node_ids[0].id);
				break;
			case 2:
				printf("Thread %d: [NULL, %ld] (protecting node %d\n", i,
						curr_node_ids[1].data,  
						curr_node_ids[1].id);
				break;
			case 3:
				printf("Thread %d: [%ld, %ld] (protecting node %d and %d\n", i,
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

	return;

}

int main(int argc, char *argv[]) {

	init_nodes();

	pthread_t thread_0, thread_1, thread_2;
	int id0 = 0;
	int id1 = 1;
	int id2 = 2;

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

	print_snapshot();

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
}
