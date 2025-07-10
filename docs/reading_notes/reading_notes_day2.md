# Key Insights for Dekker's Algorithm for Instruction Reordering 

## Solution of a Problem in Concurrent Programming Control
- Setup
    - Consider N processes in a cyclic process (some loop)
    - The loop contains a "critical section" which only one process can access at a time
        - to make this happen processes communicate using some global state (flag)
        - reading and writing to the global variables are atomic
        - so when 2 processes try to do these operations they must occur one after
          the other in some order. (.eg thread 1 write to v1 and thread 2 reads from v1)

- Problem Statement
    - Solution should be symmetrical (cant order processes)
    - processor speeds cant be assumed
    - processors stopped outside of the critical section should not stop other processes
    - if 2 processes contend for the critical section atleast one of them must go first

## AOMP - Section 2.3
- Explain peterson's deadlock free locking algorithm
- Setup
    - There are two flags, 1 for each thread A and B
    - CSa = critical section  of A and CSb is critical section of B
    - threads A and B read and write flags to implement lock and unlock methods
- Combines 2 locks
    - Lock 1 Algo
     int flag[2] = {0, 0}; 
     lock() {
        int i = t_id; // thread id
        int j = 1 - i; // either 0 or 1
        flag[i] = true; // i set to true by A
        while (flag[j]) {}; // busy wait till j is set to 0 by B
     }

     unlock() {
         int i = t_id;
         flag[i] = false; // set the flag to false when exiting critical section
     }
     - Problem 
        - with this lock and unlock if flag[i] = true happens in A then swaps to B
          and happens in B there will be a deadlock since both will be waiting on each 
          other to release and we will be deadlocked on the lock method.

    - Lock 2 Algo
        lock() {
            int i = t_id;
            victim = i;
            while (victim == i) {}
        }
        unlock() {}

    - Problem
     - if thread A or B gets scheduled to run completely before the other thread
       there will be a deadlock.
     - this method relies on the fact that victim = i; will be changed by the other thread
     - to avoid the deadlock of interleaving in case 1. Since the problem with case 1 was
       if at any point it interleaved it would cause a deadlock so we tried 
       to build a lock that handles that edge case but it 
       pushes the problem to the end of the cycle.
- Solution
    - Petersons Lock
    - combines the interleaving solution via a victim that can be changed and the complete
      execution problem solved by case 1 where if j is not set it will continue executing

    lock() {
        int i = t_id;
        int j = 1 - i;
        flag[i] = true; // says it wants to execute
        victim = i; // but lets the other one go first if it needs to
        while (flag[j] && victime == i) {} // check if the other one is went (victim != i)         and is still running or not (flag[j])
    }
    unlock() {
        int i = t_id;
        flag[i] = false;
    }

    - this works because each node sets its flag to true when it calls lock
    - it also sets the victim = i expecting it to change if the other thread has run
    - if the other thread see that the other thread called lock and wants the lock 
      (flag[i] = true) and didnt set the victim to itself yet meaning we are letting it
      go (victim = i) then we have to wait

        
