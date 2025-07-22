#include <chrono>
#include <thread>
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

void push_func(TestQueue &queue) {

	const auto start_push{std::chrono::high_resolution_clock::now()};
	for (int i = 0; i < ITERS; i++) {

		queue.push(i);
	}
	const auto stop_push{std::chrono::high_resolution_clock::now()};
	const std::chrono::duration<double> push_time{stop_push - start_push};
	
	printf("Time to insert 1000000 elements: %.6f seconds\n", push_time.count());
}

void pop_func(TestQueue &queue) {

	int tmp_val;
	int pop_count = 0;
	const auto start_pop{std::chrono::high_resolution_clock::now()};

	while (pop_count < ITERS) {
		while (queue.pop(tmp_val)) {pop_count++;}
	}

	const auto stop_pop{std::chrono::high_resolution_clock::now()};
	const std::chrono::duration<double> pop_time{stop_pop - start_pop};

	printf("Time to pop 1000000 elements: %.6f seconds\n", pop_time.count());
}

int main(void) {

	TestQueue benchmark_queue = {};

	std::jthread t0(push_func, std::ref(benchmark_queue));
	std::jthread t1(pop_func, std::ref(benchmark_queue));

	const auto start_threads{std::chrono::high_resolution_clock::now()};
	t0.join();
	t1.join();
	const auto stop_threads{std::chrono::high_resolution_clock::now()};
	const std::chrono::duration<double> thread_time{stop_threads - start_threads};

	int tmp_val;
	printf("Total Time on 2000000 elements: %.6f seconds\n", thread_time.count());
	printf("Rate: %.6f ops/seconds\n", (2.0 * ITERS)/thread_time.count());

}
