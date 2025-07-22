#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <cstddef>
template<typename T, size_t SIZE>
class RingBuffer {

	T buffer[SIZE];
	size_t read_end = 0;
	size_t write_end = 0;
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
#endif
