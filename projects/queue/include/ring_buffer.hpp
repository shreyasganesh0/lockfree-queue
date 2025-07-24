#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <cstddef>
#include <atomic>

#define ELES 250000
template<typename T, size_t SIZE>
class RingBuffer {

	T buffer[SIZE];
	std::atomic<size_t> read_end;
	std::atomic<size_t> write_end;
	bool empty_flag = true;

public:
	RingBuffer() {

		read_end = 0;
		write_end = 0;
		empty_flag = true;
	}
	bool push(const T& item); // false if full
	bool pop(T& item); // false if empty
};

template<typename T, size_t SIZE>
struct thread_send_t {
	int t_id;
	RingBuffer<T, SIZE> *buf;
}; 
#endif
