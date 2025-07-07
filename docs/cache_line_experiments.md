# Cache Line Bouncing Experiment

## Hypothesis
I expect to see a high cache miss rate when mutliple threads try to increment a shared counter due to the cache line invalidation between cores.

### Setup
The experiment is run on a Ryzen 9 7000 series AMD x86_64 CPU which has a 32KB L1 cache, 1MB L2 cache
per core and shared L3 cache. 

## Part A - Shared V/S Seperate Cache Lines

    Two threads incrementing seperate variables can be upto 15x slower than one thread doing both
    this experiment demonstrates why understanding CPU cache coherence is ciritical for production
    systems serving millions of users.

### How to run the experiment

    - compile the program
    ```
    ./build.sh
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

    Showing stats for same cache line counters

    Size of bad struct = 64 bytes
    Address of counter 1 is 0x5f7fc65306b0
    Address of counter 2 is 0x5f7fc65306b8
    Distance between counter 1 and counter 2 is 8 bytes
    Time taken for thread  to complete = 3.296375
    Time taken for thread  to complete = 3.301566

    Performance counter stats for 'bin/false_sharing bad':

           168,196,763      cache-misses:u                   #   99.98% of all cache refs
           168,235,316      cache-references:u
       <not supported>      LLC-load-misses:u
       <not supported>      LLC-store-misses:u
           168,189,991      L1-dcache-load-misses:u
       <not supported>      L1-dcache-store-misses:u

           3.303788228 seconds time elapsed

           6.581418000 seconds user
           0.002000000 seconds sys


    - Showing stats for cross cache line counter

    Size of good struct = 128 bytes
    Address of counter 1 is 0x5d4cc04ac6b0
    Address of counter 2 is 0x5d4cc04ac6f8
    Distance between counter 1 and counter 2 is 72 bytes
    Time taken for thread  to complete = 0.202983
    Time taken for thread  to complete = 0.210147

    Performance counter stats for 'bin/false_sharing good':

                 8,979      cache-misses:u                   #   41.64% of all cache refs
                21,564      cache-references:u
       <not supported>      LLC-load-misses:u
       <not supported>      LLC-store-misses:u
                 2,348      L1-dcache-load-misses:u
       <not supported>      L1-dcache-store-misses:u

           0.211990259 seconds time elapsed

           0.409449000 seconds user
           0.001979000 seconds sys

### Conclusion

The difference in cache miss rate goes from 26 to almost a 100 percent in the bad case which aligns with my hypothesis.

## Part B - Odd Cache Timing 

Even after adding only 1 byte of padding between the two counters the same speed up is still witnessed, 
despite being on the same cache line.lI tried to investigate this further

I first tried to print out the distance between the actual counters in memory to check if the reason
was due to some compiler optimization.

## Setup
    - compile the program
    ```
    ./build.sh
    ```

    - run the program using flags
    ```
    ./false_sharing good cross-ccx

            OR

    ./false_sharing good same-ccx

### Evidence

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

    Despite there not being any differences in the address that shows cache line differences it still shows a great speedup

    Further exploration made me come to the conclusion that it could be because in the AMD architecture, cores form a shared group called the Core Complex (CCX). Cores in a CCX share the L3 cache more efficiently. Thus even a small padding might be prefetched by the hardware and causing the speed up despite being on the same cache line.

### Results

    Showing stats where core affinity is for adjacent cores so they share CCX
    Invalid args
    Performance counter stats for 'bin/false_sharing same-ccx':

             4,609      cache-misses:u                   #   46.12% of all cache refs
             9,993      cache-references:u
    <not supported>      LLC-load-misses:u
    <not supported>      LLC-store-misses:u
             1,393      L1-dcache-load-misses:u
    <not supported>      L1-dcache-store-misses:u

       0.001196320 seconds time elapsed

       0.000000000 seconds user
       0.001211000 seconds sys


    Showing stats where cores dont belong to the same CCX
    Size of good struct = 64 bytes
    Address of counter 1 is 0x5bbb247ee6b0
    Address of counter 2 is 0x5bbb247ee6c0
    Distance between counter 1 and counter 2 is 16 bytes
    Time taken for thread  to complete = 0.409357
    Time taken for thread to complete = 0.409417

    Performance counter stats for 'bin/false_sharing good cross-ccx':

             7,892      cache-misses:u                   #   32.75% of all cache refs
            24,095      cache-references:u
    <not supported>      LLC-load-misses:u
    <not supported>      LLC-store-misses:u
             2,599      L1-dcache-load-misses:u
    <not supported>      L1-dcache-store-misses:u

       0.410921332 seconds time elapsed

       0.802727000 seconds user
       0.002971000 seconds sys

The results show that when threads are pinned to cores not in the same CCX there is 2X slow down on accesses. I even added a more comprehensive read-modify-write operation for the counter to force the CPU to read the current value along with using nops with inline assembly to prevent compiler optimizations to make this work. 
The CCX seems to be part of the reason of the speedup but doesnt fully explain why just one byte is enough to cause a speed up equal to a cache line jump.
This could be useful in saving memory in optimization cases where we previously assumed that we might have to use an entire 64 bytes where now only 1 byte might be sufficient.
