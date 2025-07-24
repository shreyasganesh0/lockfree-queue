#include "ring_buffer.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <atomic>
#include <thread>

template<typename T, size_t SIZE>
bool RingBuffer<T, SIZE>::pop(T& item) {
	
	size_t curr_read_end, next_read_end;

	do {

		curr_read_end = read_end.load();
		next_read_end = (curr_read_end + 1) % SIZE;

		if (curr_read_end == write_end.load()) {

			return false;
		}
	} while (!read_end.compare_exchange_weak(curr_read_end, next_read_end));

	item = buffer[curr_read_end];
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
	} while (!(write_end.compare_exchange_weak(curr_tail, next_tail)));

	buffer[curr_tail] = item;

	return true;
}

template<typename T, size_t SIZE>
void push_elements(thread_send_t<T, SIZE> *thread_state) {

	for (int i = thread_state->t_id * ELES; i < (thread_state->t_id + 1) *ELES; i++) {

		thread_state->buf->push(i);
	}

	printf("Thread %d finished inserting\n", thread_state->t_id);
}

int main(void) {

	RingBuffer<int, 1000001> ring_buf;

	thread_send_t<int, 1000001> t0{0, &ring_buf};
	thread_send_t<int, 1000001> t1{1, &ring_buf};
	thread_send_t<int, 1000001> t2{2, &ring_buf};
	thread_send_t<int, 1000001> t3{3, &ring_buf};

	std::jthread thread0(push_elements<int, 1000001>, &t0);
	std::jthread thread1(push_elements<int, 1000001>, &t1);
	std::jthread thread2(push_elements<int, 1000001>, &t2);
	std::jthread thread3(push_elements<int, 1000001>, &t3); 

	int tmp = 0;
	int count = 0;
	do {
	while (ring_buf.pop(tmp)) {count++; printf("Popped %d ", tmp);}
	} while (count < 4 * ELES);

	printf("\nTotal Popped %d", count);

	thread0.join();
	thread1.join();
	thread2.join();
	thread3.join();
}
