#ifndef MPMC_QUEUE_H
#define MPMC_QUEUE_H

alignas(64) struct QueueMetrics {

	std::atomic<uint64_t> enqueue_total;
	std::atomic<uint64_t> dequeue_total;
	std::atomic<uint64_t> enqueue_failures;
	std::atomic<uint64_t> dequeue_event;
	std::atomic<uint64_t> contention_events; 
	char padding[32];
};

template<typename T>
class alignas(64) MPMCQueue {

	bool enqueue(const T &item);
	bool dequeue(T &item);

	const QueueMetrics &metrics() const;

	size_t queue_size() const;
};
#endif
