# Lock-Free Queue Benchmark

[![Performance Benchmarks](https://github.com/shreyaganesh0/lockfree-queue/actions/workflows/benchmark.yml/badge.svg)](https://github.com/shreyaganesh0/lockfree-queue/actions/workflows/benchmark.yml)

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


### Experiment 2 - CPU Instruction and Memory Reordering Experiment

    ### Hypothesis

    This experiment aims to implement Dekkers algorithmn to prove that memory reordering 
    of instructions may cause deadlocks that are subtle and hard to reproduce
    Hardware Specifications - Ryzen 9 7940HS single 8-core CCX

    ### Results
    Memory Reordering Detection Statistics
    ========================================
    Runs analyzed: 10
    Minimum iterations: 9,992 (0.01M)
    Maximum iterations: 102,408,891 (102.41M)
    Mean iterations: 80,790,652 (80.79M)
    Median iterations: 100,409,264 (100.41M)
    Standard deviation: 40,208,990 (40.21M)
    Coefficient of variation: 49.8%

[Plot](experiments/memory_ordering/results/reordering_detection_distribution.png)
