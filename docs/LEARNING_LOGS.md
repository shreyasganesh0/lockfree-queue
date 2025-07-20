# Learnings From Benchmark Queue

## Std library use
- std::chrono::high_resolution_clock::now();
    - this gives us a time for now which we store in start and stop to measure
    - std::chrono::duration<double> time_done{stop - start};
        - we can calculate the time taken with duration

    - .count() function retruns the raw .double duration from chrono::duration
- std::lock_guard<std::mutex> guard{mutex};
    - this can be use to do RAII with mutexes so we dont have to explicitlly freee
      lock and resources
