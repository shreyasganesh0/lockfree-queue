# Memory Ordering Issues Troubleshooting Guide

## Symptom: Sporadic data corruption under high load.
1. Check: if multiple threads access shared data
2. Verify: if data is cache aligned
3. Test: Run memory ordering detector on target hardware
4. Fix: Add memory barriers or use atomic operations with sequential consistency

## Symptom Performance degradation with multiple cores
1. Measure: Cache misses using perf stat -e cache-misses
2. Identify: False sharing using experimental approach
3. Fix: Align data structures to cache line boundaries

## Prevention Checklist
1. All concurrent data structures are cache-line aligned
2. Memory barriers are used for lock free algorithms
3. Performance regression tests include multi-core testing.
4. Code is tested on all production hardware types.
