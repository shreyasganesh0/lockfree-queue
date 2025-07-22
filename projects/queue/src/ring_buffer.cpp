#include "ring_buffer.hpp"
#include <stdio.h>
#include <stdlib.h>

template<typename T, size_t SIZE>
bool RingBuffer<T, SIZE>::pop(T& item) {
	
	if ((read_end != write_end) || (read_end == write_end && empty_flag == false)) {
		if (read_end == write_end) empty_flag = true;
		item = buffer[read_end];
		read_end++;
		read_end %= SIZE;
		return true;
	}

	return false;
}

template<typename T, size_t SIZE>
bool RingBuffer<T, SIZE>::push(const T& item) {


	if (((write_end % SIZE) != read_end) || 
			((write_end % SIZE) == read_end && (empty_flag == true))) {
		if (read_end == write_end) empty_flag = false;
		buffer[write_end] = item;
		write_end++;
		write_end %= SIZE;
		return true;
	}

	return false;
}


int main(void) {

	RingBuffer<int, 10> ring_buf;

	printf("Pushing 0-9\n");

	printf("Pushed: ");
	for (int i = 0; i < 11; i++) {

		if (ring_buf.push(i)) {
			printf("%d ", i);
		} else {

			printf("Failed to push %d, buffer full", i);
		}
	}

	printf("\n");

	printf("Popping 5 elements\n");

	printf("Popped: ");
	int tmp = 0;
	for (int i = 0; i < 5; i++) {

		ring_buf.pop(tmp);
		printf("%d ", tmp);
	}
	printf("\n");

	printf("Pushing 10-12\n");
	printf("Pushed: ");
	for (int i = 0; i < 3; i++) {

		ring_buf.push(i + 10);
		printf("%d ", i + 10);
	}

	printf("\n");
	printf("Popping remaining...\n");

	printf("Popped: ");
	while (ring_buf.pop(tmp)) {printf("%d ", tmp);}

	printf("\n");
	printf("Buffer is now empty\n");

}
