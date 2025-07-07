# lockfree-queue

## Description
Production grade lock-free queue implementation acheiving 50M ops/second. designed for high frequesncy operations where microsecond latency matters.

## Experiments

### Experiment 1 - Cache Line Bouncing

    ### Part A - Shared V/S Seperate Cache Lines

    Two threads incrementing seperate variables can be upto 15x slower than one thread doing both
    this experiment demonstrates why understanding CPU cache coherence is ciritical for production
    systems serving millions of users.

    ### Part B - Odd Cache Timing 

    Even after adding only 1 byte of padding between the two counters the same speed up is still witnessed, 
    despite being on the same cache line.lI tried to investigate this further

    I first tried to print out the distance between the actual counters in memory to check if the reason
    was due to some compiler optimization.


