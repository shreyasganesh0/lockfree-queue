int deletion_blocked = 0;
int deletion_success = 0;

void  *delete_nodes(void *id) {


	while (1) {

	int node_to_delete = rand() % 4;
	printf("\n[DELETER NODE]: Trying to delete node %d...\n", node_to_delete);
	int	s_flag = -1;

		for (int i = 0; i < 3; i++) {

			for (int j = 0; j < 2; j++) {
			
				if (hazard_pointers[i][j] == node[node_to_delete]) {

					s_flag = i;
					break;
				}

			}
			if (s_flag == -1) break;

		}

		if (s_flag != -1) {
			deletion_success++;
			printf("[DELETER NODE]: Successfully delete node %d\n", node_to_delete );
		} else {

			deletion_blocked++;
			printf("[DELETER NODE]: BLOCKED (thread %d is using it)\n", s_flag);
		}
		sleep(1);
	}

	return NULL;
}
