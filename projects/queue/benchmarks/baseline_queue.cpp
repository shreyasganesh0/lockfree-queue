#include <chrono>
#include <queue>
#include <mutex>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "baseline_queue.hpp"

void TestQueue::push(int val) {

	const std::lock_guard<std::mutex> guard(m);
	q.push(val);

	return;
}

bool TestQueue::pop(int &val) {


	const std::lock_guard<std::mutex> guard(m);
	if (q.empty()) return false;
	val = q.front();
	q.pop();

	return true;
}


int main(void) {

	TestQueue benchmark_queue = {};

	const auto start_push{std::chrono::high_resolution_clock::now()};
	for (int i = 0; i < ITERS; i++) {

		benchmark_queue.push(i);
	}
	const auto stop_push{std::chrono::high_resolution_clock::now()};

	int tmp_val;
	const auto start_pop{std::chrono::high_resolution_clock::now()};
	while (benchmark_queue.pop(tmp_val)) {}
	const auto stop_pop{std::chrono::high_resolution_clock::now()};

	const std::chrono::duration<double> push_time{stop_push - start_push};
	const std::chrono::duration<double> pop_time{stop_pop - start_pop};

	printf("Time to insert 1000000 elements: %.6f seconds\n", push_time.count());
	printf("Time to pop 1000000 elements: %.6f seconds\n", pop_time.count());
	printf("Total Time on 2000000 elements: %.6f seconds\n", (pop_time + push_time).count());
	printf("Rate: %.6f ops/seconds\n", (2.0 * ITERS)/(pop_time + push_time).count());

}
