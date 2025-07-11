# CPU Instruction and Memory Reordering Experiment

## Hypothesis
This experiment aims to implement Dekkers algorithmn to prove that memory reordering 
of instructions may cause deadlocks that are subtle and hard to reproduce

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
    - this breaks our locking mechanism and is the implementationof Dekkers algorithmn


