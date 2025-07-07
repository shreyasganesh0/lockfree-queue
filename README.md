# lockfree-queue

## Description
Production grade lock-free queue implementation acheiving 50M ops/second. designed for high frequesncy operations where microsecond latency matters.

## Experiments

### Experiment 1 - Cache Line Bouncing

    Two threads incrementing seperate variables can be upto 176x slower than one thread doing both
    this experiment demonstrates why understanding CPU cache coherence is ciritical for production
    systems serving millions of users.

    ### Setup
    The experiment is run on a Ryzen 9 7000 series AMD x86_64 CPU which has a 32KB L1 cache, 1MB L2 cache
    per core and shared L3 cache. 

    ### How to run the experiment
    
    - compile the program
    ```
    gcc -pthread experiments/false_sharing -o false_sharing
    ```

    - run the program using flags
    ```
    ./false_sharing good

            OR

    ./false_sharing bad
    ```

    ### Explanation

    The good and bad cases demonstrate how moving data in and out of L1 cache in the 64 byte cache lines can cause
    high latency with upto a 65x decrease even with shared threads.

    The bad version of the counter is a struct with memory aligned struct containing 2 adjacent 8 byte longs.
    The cache coherence MESI policy dictates that the cache lines be moved in 64 bytes at a time and be cleared
    when another thread needs to access the cache.
    since a single cache line will contain both counters, everytime the other thread has to access the counter the 
    cache line has to be cleared and re populated which causes huge latency

    In the good case we add 64 bytes of padding to seperate the two counters and have them on different cache lines.
    This way the threads do not have to clear the cache line in cache on which the other threads counter exists thus
    causing a massive speed up in the cache line case.

    ### Initial results

    $ perf stat -e cache-misses,cache-references,bus-cycles ./false_sharing good
    Size of good struct = 128 bytes
    Address of counter 1 is 0x5cc2bc8d96b0
    Address of counter 2 is 0x5cc2bc8d96f8
    Distance between counter 1 and counter 2 is 72 bytes
    Time taken for thread  to complete = 0.009699
    Time taken for thread  to complete = 0.009851

    Performance counter stats for './false_sharing good':

             8,280      cache-misses:u                   #   49.95% of all cache refs
            16,576      cache-references:u
       <not supported>      bus-cycles:u

       0.011505820 seconds time elapsed

       0.020153000 seconds user
       0.001014000 seconds sys


    $ perf stat -e cache-misses,cache-references,bus-cycles ./false_sharing bad
    Size of bad struct = 64 bytes
    Address of counter 1 is 0x55c6aee8a6b0
    Address of counter 2 is 0x55c6aee8a6b8
    Distance between counter 1 and counter 2 is 8 bytes
    Time taken for thread  to complete = 20.777588
    Time taken for thread  to complete = 20.829665

     Performance counter stats for './false_sharing bad':

           957,937,632      cache-misses:u                   #   99.99% of all cache refs
           958,075,263      cache-references:u
       <not supported>      bus-cycles:u

      20.831486289 seconds time elapsed

      41.512073000 seconds user
       0.001000000 seconds sys

    The difference in cache miss rate goes from 50 to almost a 100 percent in the bad case which aligns with my hypothesis.

    ### Side affects

    Even after adding only 1 byte of padding between the two counters the same speed up is still witnessed, 
    despite being on the same cache line.lI tried to investigate this further

    I first tried to print out the distance between the actual counters in memory to check if the reason
    was due to some compiler optimization.

    $ ./false_sharing bad
    Size of bad struct = 64 bytes
    Address of counter 1 is 0x588358a456b0
    Address of counter 2 is 0x588358a456b8
    Distance between counter 1 and counter 2 is 8 bytes
    Time taken for thread  to complete = 20.769536
    Time taken for thread  to complete = 20.852941

    $ ./false_sharing good
    Size of good struct = 64 bytes
    Address of counter 1 is 0x5ab65662b6b0
    Address of counter 2 is 0x5ab65662b6c0
    Distance between counter 1 and counter 2 is 16 bytes
    Time taken for thread  to complete = 0.009216
    Time taken for thread  to complete = 0.009827

    despite there not being any differences in the address that shows cache line differences it still shows a great speedup
