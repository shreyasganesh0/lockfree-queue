#include "ring_buffer.hpp"
#include <stdio.h>
#include <stdlib.h>

template<typename T, size_t SIZE>
bool RingBuffer<T, SIZE>::pop(T& item) {
	
	size_t curr_read_end, next_read_end;

	do {

		curr_read_end = read_end.load():
		next_read_end = (curr_read_end + 1) % SIZE;

		if (next_read_end == write_end.load()) {

			return false;
		}
	} while (!read_end.compare_exchange(curr_read_end, next_read_end));

	return true;
}

template<typename T, size_t SIZE>
bool RingBuffer<T, SIZE>::push(const T& item) {

	size_t curr_tail, next_tail;

	do {
		curr_tail = write_end.load();
		next_tail = (curr_tail + 1) % SIZE;

		if (next_tail == read_end.load()) {

			return false;
		}
	} while (!(write_end.compare_exchange_weak(curr_tail, next_tail));

	item = buffer[curr_read_end];

	return true;
}

void push_element(RingBuffer &buf) {

	for (int i = id * ELES; i < (id + 1) * ELES; i++) {

		printf("%d ", i)
	}
}

int main(void) {

	RingBuffer<int, 10> ring_buf;

	int id0 = 0;
	int id1 = 1;
	int id2 = 2;
	int id3 = 3;
	int id4 = 4;
	int id5 = 5;
	std::jthread t0(push_elements, ring_buf);
	std::jthread t1(push_elements, ring_buf);
	std::jthread t2(push_elements, ring_buf);
	std::jthread t3(push_elements, ring_buf);

	std::jthread t4(pop_elements, ring_buf);
	std::jthread t5(pop_elements, ring_buf);

	t0.join();
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
}
