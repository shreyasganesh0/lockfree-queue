# Lock-Free MPMC Queue in C++

This repository contains an implementation of a lock-free Multi-Producer Multi-Consumer (MPMC) queue using C++ atomics. The primary goal of this project was to gain a deep, practical understanding of lock-free algorithms, the challenges of concurrent memory access on modern CPUs, and techniques for performance optimization.

## Why Build This?

Lock-free data structures promise higher throughput and avoid problems like deadlock and priority inversion common with mutex-based approaches. However, they introduce subtle complexities related to:
* **Memory Ordering:** Ensuring operations appear in the correct order across different CPU cores.
* **Atomicity:** Performing multi-step operations (like enqueue/dequeue) safely without locks, often using Compare-and-Swap (CAS).
* **ABA Problem:** Avoiding issues where a memory location is read, modified by another thread, then modified back, fooling a CAS loop. (Solved here using sequence numbers).
* **Memory Reclamation:** Safely freeing memory in a concurrent environment (explored via Hazard Pointers).
* **Performance Pitfalls:** Understanding issues like cache contention and false sharing.

This project includes not only the queue implementation but also targeted experiments and benchmarks to explore these concepts hands-on.

## Features

* **Lock-Free MPMC Queue:** A queue implementation (`projects/queue/include/mpmc_queue.hpp`) allowing multiple threads to enqueue and dequeue items concurrently without using mutexes, primarily relying on C++ `std::atomic` operations and sequence numbers to manage slots.
* **Ring Buffer Base:** Built upon a fixed-size ring buffer (`projects/queue/include/ring_buffer.hpp`) optimized with cache-line alignment to mitigate false sharing [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/docs/decisions/ADR-001-cache-line-alignment.md].
* **Memory Ordering Experiments:** Code (`experiments/memory_ordering/`) to demonstrate potential memory reordering issues by the compiler or CPU and the necessity of memory barriers/atomic operations [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/experiments/memory_ordering/src/memory_ordering.c]. Includes stress testing and analysis [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/experiments/memory_ordering/scripts/stress_memory_ordering.sh, shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/experiments/memory_ordering/results/stress_test_analysis.png].
* **False Sharing Experiments:** Code (`experiments/false_sharing/`) demonstrating the performance impact of false sharing and mitigation techniques [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/experiments/false_sharing/src/false_sharing.c].
* **Hazard Pointers Exploration:** Initial exploration and visualization tools (`experiments/hazard_pointers/`) for understanding the Hazard Pointers algorithm for safe memory reclamation [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/experiments/hazard_pointers/tools/hazard_pointers_viz.c].
* **Benchmarking:** Includes a baseline mutex-based queue (`projects/queue/benchmarks/include/baseline_queue.hpp`) for performance comparison and benchmark results [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/projects/queue/benchmarks/results/]. Benchmarks are automated via GitHub Actions [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/.github/workflows/benchmark.yml].
* **Detailed Documentation:** Extensive learning logs, design decisions (ADRs), and reading notes are available in the `/docs` directory [cite: shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/docs/LEARNING_LOGS.md, shreyasganesh0/lockfree-queue/lockfree-queue-392905297f2f1986913ccdd3ccf09ab709ff3aa7/docs/RUNBOOK.md, etc.].

## Technical Deep Dive: Lock-Free Enqueue with Sequence Numbers

The core logic uses atomic operations on the `head_` (dequeue) and `tail_` (enqueue) indices. To coordinate producers and consumers safely without locks and avoid the ABA problem, each slot in the underlying ring buffer also stores a sequence number (`seq_`).

A producer wishing to enqueue performs these steps:
1.  Loads the current `tail_` index.
2.  Checks the sequence number (`seq_`) at that `tail_` slot in the ring buffer. It should equal the `tail_` index itself, indicating the slot is free.
3.  Checks if the queue is full by comparing `tail_` to a cached `head_` value.
4.  If the slot is free and the queue isn't full, it attempts to atomically increment `tail_` using `compare_exchange_weak`.
5.  If the CAS succeeds, it means this producer successfully claimed the `tail_` slot. It can now write the data into `buffer_[tail_]` and, critically, update the sequence number at that slot to `tail_ + 1` using `store` with `memory_order_release`. This signals to consumers that the data is ready.
6.  If any check fails or the CAS fails (meaning another producer raced ahead), the producer retries the entire loop.

Here is the actual `enqueue` implementation from `projects/queue/include/mpmc_queue.hpp`:

```c++
// From: projects/queue/include/mpmc_queue.hpp
template <typename T>
bool MPMCQueue<T>::enqueue(const T &data)
{
    size_t tail = tail_.load(std::memory_order_relaxed);
    for (;;) // Start CAS loop
    {
        Slot &slot = buffer_[tail & mask_]; // Get slot using bitwise AND for wrap-around
        size_t seq = slot.seq_.load(std::memory_order_acquire);
        intptr_t dif = (intptr_t)seq - (intptr_t)tail; // Calculate difference

        // Check if slot is free (sequence matches tail index)
        if (dif == 0)
        {
            // Try to atomically claim the slot by incrementing tail_
            if (tail_.compare_exchange_weak(tail, tail + 1, std::memory_order_relaxed))
            {
                // Successfully claimed slot, write data and update sequence number
                slot.data_ = data;
                slot.seq_.store(tail + 1, std::memory_order_release); // Signal data is ready
                return true;
            }
            // CAS failed, another producer got here, retry loop
        }
        else if (dif < 0) // Check if queue is full
        {
            // seq should be tail + 1 if slot is occupied by a previous enqueue
            // If seq < tail, it means the head has wrapped around and caught up
            // Check against a potentially stale head_cache first for performance
            size_t head = head_cache_.load(std::memory_order_relaxed);
            if(tail - head >= capacity_) {
                // Cache indicates full, load actual head and re-check
                 head = head_.load(std::memory_order_acquire);
                 if(tail - head >= capacity_) {
                     // Queue is definitively full
                     return false;
                 }
                 head_cache_.store(head, std::memory_order_relaxed); // Update cache
            }

             // Reload tail as it might have changed
             tail = tail_.load(std::memory_order_relaxed);
        } else {
             // Sequence number is ahead of tail; indicates stale tail, reload
             tail = tail_.load(std::memory_order_relaxed);
        }
    }
}
(Note: A similar pattern with sequence numbers and CAS is used in the dequeue method for consumers.)
```
## How to Build & Run
1. Build the baseline benchmark (mutex-based queue)
```
cd projects/queue/benchmarks
./build.sh 
```

2. Run the baseline benchmark
```
./baseline --threads=4 --items=1000000 > results/baseline_result_4t_1m.txt
```

3. Build and run the false sharing experiment
```
cd ../../../experiments/false_sharing
./scripts/build_and_test.sh
```

4. Build and run memory ordering stress test
```
cd ../memory_ordering
./scripts/build.sh
./scripts/stress_memory_ordering.sh # Generates report in results/
```

## Further Reading
See the /docs directory for detailed learning logs, architectural decisions, and notes on relevant academic papers and resources used during development.
