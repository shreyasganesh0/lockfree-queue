int deletion_blocked = 0;
int deletion_success = 0;

void  *delete_nodes(void *id) {

	int node_to_delete = rand() % 4;
	int s_flag = 0;

	printf("\n[DELETER NODE]: Trying to delete node %d...", node_to_delete);

	while (1) {
		s_flag = 0;
		for (int i = 0; i < 3; i++) {

			for (int j = 0; j < 2; j++) {
			
				if (hazard_pointers[i][j] == node[node_to_delete]) {

					deletion_blocked++;
					printf("[DELETER NODE]: BLOCKED (thread %d is using it)\n", i);
					s_flag = 1;
					break;
				}

			}

			if (s_flag == 1) {

				deletion_success++;
				break;
			}
		}

		sleep(1);
	}

	return NULL;
}
