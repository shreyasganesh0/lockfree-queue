# CPU Instruction and Memory Reordering Experiment

## Hypothesis
This experiment aims to implement Dekkers algorithmn to prove that memory reordering 
of instructions may cause deadlocks that are subtle and hard to reproduce
Hardware Specifications - Ryzen 9 7940HS single 8-core CCX

## Setup
    - We have 2 Threads running simlultaneously on the Ryzen 9 7000 series CPU.
    - Each thread has a volatile global flag that it sets before trying to enter
    the critical setion of the program.
    - Threads only enter the critical section if  they check (read) that the other
    threads flag is not set i.e trying to enforce 1 thread in the critical section
    at a time.
    - The thread then double checks the other threads flag while inside the
      critical section to see if it changed while it is inside.
        - this will only occur if the setting of the flag and the check gets reordered
          such that the value of the register used to check is still 0 (unset) for 
          the global thread flag which should not be the case despite the other thread 
          already having set the flag for the critical section

## Part A - Initial experiment

### Usage
```
cd experiment/memory_ordering

scripts/build.sh

bin/memory_ordering
```

### Conclusion
- Threads writes and reads may not be instantaneously visible to the other thread
- if Thread 0 sets value to 1 and Thread 1 checks for thread_flag_0 to be == 0
  before entering the critical section then the function will run as expected
    - since only the runs where the thread is able to change the flag and do 
      the flag check on the other threads flag simultaneously will work
    - In cases where thread 0 sets and thread 1 swaps in and sets both will skip 
      the critical section and reset the flag values and repeat the loop
    - if thread 0 sets the flag and then checks the value of thread 1
    -> then the cpu swaps to thread 1 which sets its flag and checks thread 0
    flag, the expected thing  will happen where thread 0 will enter the cs (since
    it did the check first) and thread 1 will wait till it exits

- The Pathalogical case:
    - If the order of execution is thread 0 set -> thread 1 set -> thread 0 check; 
      (original critical section skip scenario) but the instructions get 
      reorderd by the CPU 
    - thread 0 will see thread 1's flag as 0 despite it being set to 1 by thre
        - the thread 0 checks thread 1 instruction will see flag 1 as 0 despite
        thread 1 having changed the value which will cause it to see 1 on flag 1
        when double checking while in the critical section.
    - this breaks our locking mechanism and is the implementationof Dekkers algorithm.

## Part B - Updated Conditions to force reordering

### Setup
- Add thread pinning to force the cpu to assign threads to a particular core
- this assignment allows us to stress the cpu core exactly using the stress-ng tool
    - stress-ng --cpu 7 --io 4
    - The higher the stress the cpu core is under the more likely hood of thread
      swapping and instruction reordering is to occur
    - the cacheline swapping will also increase the likelihood of seeing instruction
      reordering
- Atomic counter increments were also added using __sync_fetch_and_add(&counter, 1);
    - this counter was used to display progress every second via the main thread
    - some iterations may be long running and this was an atomic way for the multiple
      threads to update the counter with the number of iterations they completed
- flags to check if the threads completed execution were added
    - this was important for the async notification of progress via the main thread
        - if we used pthread_join it would be blocking
        - but if we had an infinte while loop we could have a blocking join in it
        - so we set a flag on exit which the while loop checks before exiting.
- sched_yeild() was also added every 1000 iterations to add more chaos and increase
  the chances of seeing reordering.

### Conclusion
Instruction reordering was witnessed 
    ```
    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 101739127 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 101338235 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 10624
    Instruction has been reordered! Completed 100270916 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 761688 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 102408891 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 9992 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 100595764 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 100142033 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 100547611 iterations

    [shreyas@arch-tiling memory_ordering]$ bin/memory_reordering
    Program is running...
    Number of iterations run until now: 0
    Instruction has been reordered! Completed 100092265 iterations
    ```
Race conditions galore!...

## Experimental Results

### Summary of 10 Runs
- Reordering detection range 9992 - 102408891 iterations
- Average detection point = 80790642 iterations
- Standard deviation = 40208990 iterations (large variance)

### Observation
There is a large variance since due to outliers caused by the non deterministic
nature of reordering
Reordering can be caused by.
    - Store buffer availabilty at the moment of writing
    - Cache line state (Modified/Exclusive/Shared/Invalid)
    - Background processes causing thread swapping.
    - CPU frequency and thremal stress

### x86_64 TSO Model Implications
Despite the X86_64 arch supporting a strong memory model (TSO)
reordering of instructions is still possible as proven by this experiment.
We must therefore be cautious about any race conditions that may occur 
when implementing Store-Load opertions since it only garuntees Load-Load and
Store-Store operations ordering to be maintained.

## Production Impact
Services like data bases that using locking while performing transactions
must not make assumptions about memory ordering when locking and unlocking
is involved.
If we are to implement any home grown applications this may be something to 
consider while executing critical sections of the code to avoid hard to 
detect race conditions.

### Final Results
Program is running...
Number of iterations run until now: 0

=== MEMORY REORDERING DETECTED ===
Iteration: 100074573 (50.0373% of maximum)

What happened (chronological order):
1. Thread 1@Core0: Set flag0 = 1
2. Thread 1@Core0: Read flag1 = 0 (appears safe to enter)
3. Thread 0@Core7: Set flag1 = 1
4. Thread 0@Core7: Read flag0 = 0 (also appears safe!)

Both threads entered the critical section!

CPU Explanation:
- Thread 1's read of flag1 executed BEFORE its write to flag0 was visible
- Thread 0's read of flag0 executed BEFORE its write to flag1 was visible
- This is Store-Load reordering in action
