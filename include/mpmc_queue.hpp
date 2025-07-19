#ifndef MPMC_QUEUE_H
#define MPMC_QUEUE_H

#include <atomic>
#include <cstddef>

namespace lockfree {

static constexpr size_t CACHE_LINE_SIZE = 64;

// Metrics structure - each metric on its own cache line to prevent false sharing
struct alignas(CACHE_LINE_SIZE) QueueMetrics {
    std::atomic<uint64_t> enqueue_total{0};
    char padding1[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>)];
    
    std::atomic<uint64_t> dequeue_total{0};
    char padding2[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>)];
    
    std::atomic<uint64_t> enqueue_failures{0};
    char padding3[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>)];
    
    std::atomic<uint64_t> dequeue_empty{0};
    char padding4[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>)];
    
    std::atomic<uint64_t> contention_events{0};
    char padding5[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>)];
};

// Configuration for queue construction
struct QueueConfig {
    size_t capacity;                    // Max elements in queue
    size_t hazard_pointer_per_thread;   // HP slots per thread 
};

// Multi-Producer Multi-Consumer Lock-Free Queue
// Based on Michael & Scott algorithm with hazard pointers for safe memory reclamation
template<typename T>
class alignas(CACHE_LINE_SIZE) MPMCQueue {
public:
    // Constructor - allocates bounded queue
    // Time: O(capacity) for node pool allocation
    // Progress: wait-free
    explicit MPMCQueue(const QueueConfig& config);
    
    // Destructor - cleans up all resources
    // Time: O(capacity)  
    // Progress: wait-free
    ~MPMCQueue();
    
    // Attempt to add element to back of queue
    // Returns: true if successful, false if queue full
    // Time: O(1) amortized, O(threads) worst case under contention
    // Progress: lock-free (guaranteed system-wide progress)
    // Memory order: sequential consistency for queue operations
    bool enqueue(const T& item);
    
    // Attempt to remove element from front of queue  
    // Returns: true if successful and item is populated, false if queue empty
    // Time: O(1) amortized, O(threads) worst case under contention
    // Progress: lock-free (guaranteed system-wide progress)
    // Memory order: sequential consistency for queue operations
    bool dequeue(T& item);
    
    // Get current queue metrics
    // Time: O(1)
    // Progress: wait-free
    const QueueMetrics& metrics() const { return metrics_; }
    
    // Approximate number of elements currently in queue
    // Note: This is an estimate in lock-free structures
    // Time: O(1)
    // Progress: wait-free
    size_t size_approx() const;
    
    // Prevent copying - lock-free structures should not be copied
    MPMCQueue(const MPMCQueue&) = delete;
    MPMCQueue& operator=(const MPMCQueue&) = delete;
    MPMCQueue(MPMCQueue&&) = delete;
    MPMCQueue& operator=(MPMCQueue&&) = delete;
    
private:
    // Internal node structure (implementation detail)
    struct Node;
    
    // Queue state (details hidden)
    alignas(CACHE_LINE_SIZE) std::atomic<Node*> head_;
    char padding1[CACHE_LINE_SIZE - sizeof(std::atomic<Node*>)];
    
    alignas(CACHE_LINE_SIZE) std::atomic<Node*> tail_;  
    char padding2[CACHE_LINE_SIZE - sizeof(std::atomic<Node*>)];
    
    // Metrics
    mutable QueueMetrics metrics_;
    
    // Node pool for bounded queue (implementation detail)
    // Hazard pointer management (implementation detail)
};

} // namespace lockfree

#endif // MPMC_QUEUE_H
