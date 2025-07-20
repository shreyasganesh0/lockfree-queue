#ifndef BASELINE_QUEUE_HPP
#define BASELINE_QUEUE_HPP

#define ITERS 1000000

class TestQueue {

	std::queue<int> q;
	std::mutex m;

public:

	void push(int val);
	bool pop(int &val);
};
#endif
