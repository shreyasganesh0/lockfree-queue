# ADR-001: Mandatory 64-byte Cache Line Alignment for All Concurrent Data Structures

## Status: Accepted

## Decision
    All data accessed by multiple threads MUST be aligned to 64 byte cache boundaries using alignas(64) in C++ or explicit padding in C. Potentially 1 byte padding between shared threads may be sufficient in the future.

## Consequences
    Positives:
        - Eliminates false sharing
        - Prevents cache contention
        - Predictable performance 
    Disadvantages:
        - Requires extra memory (possible to amortize in actual systems)
        - Has to be architecture specific

## Measurements

         Command     | Structure Size (bytes) | Counter byte gap | Cache miss rate (%) |  
---------------------|------------------------|------------------|---------------------|
./false_sharing bad  |        64              |      8           |    99.99            |
                     |                        |                  |                     |
./false_sharing good |        128             |      72          |     41.64           |
                     |                        |                  |                     |
./false_sharing good |        72              |       8          |      46.12          |
cross-ccx            |                        |                  |                     |
                     |                        |                  |                     |
./false_sharing good |        72              |       8          |     39.98           |
same-ccx             |                        |                  |                     |
_____________________|________________________|__________________|_____________________|

## Production Impact Calculation

Netflix DAU ~ 270  Million
 -  1 % affected = 2.7 Million streams
 - 3 Mbps per stream = 3 x 10^3 x 2.7 10^6 = 8.1 Gbps of potential degraded traffic

## Verification

-verify struct alignment to check if structure is cache aligncd using 
```
static_assert(alignof(YourStruct) == 64, "Cache line alignment violated");
static_assert(offsetof(YourStruct, field) % 64 == 0, "Field not cache aligned");
