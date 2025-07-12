# Primer on Memory Consistency and Cache Coherence

## Sequential Consistency

- Single core SC
    - Result of the execution is the same as if the operations were executed in order
- Multi core SC
    - Result is the same as if all processors executed in some sequential order
      and the operations of a single core appeared in sequence as its programs
      flow
- total order of operations is called Memory order
    - in SC each core respects the program order
    - op1 <p op2 then op1 <m op2 (where <p is program order and <m is memory order)
- in TSO exactly one type of reordering is allowed - 
  load can bypass an earlier stores to differnt address
- X86_64 allows this to happen

