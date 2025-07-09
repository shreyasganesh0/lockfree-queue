#!/bin/bash

 echo "Showing stats for same cache line counters"
 perf stat -e cache-misses,cache-references,LLC-load-misses,LLC-store-misses,L1-dcache-load-misses,L1-dcache-store-misses bin/false_sharing bad

 echo "Showing stats for cross cache line counter"
 perf stat -e cache-misses,cache-references,LLC-load-misses,LLC-store-misses,L1-dcache-load-misses,L1-dcache-store-misses bin/false_sharing good

 echo "Showing stats where core affinity is for adjacent cores so they share CCX"
 perf stat -e cache-misses,cache-references,LLC-load-misses,LLC-store-misses,L1-dcache-load-misses,L1-dcache-store-misses bin/false_sharing same-ccx

 echo "Showing stats where cores dont belong to the same CCX"
 perf stat -e cache-misses,cache-references,LLC-load-misses,LLC-store-misses,L1-dcache-load-misses,L1-dcache-store-misses bin/false_sharing good cross-ccx

