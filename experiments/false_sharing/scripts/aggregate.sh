#!/bin/bash

for i in {1..10}; do
    bin/false_sharing bad >> /tmp/main_results_raw.txt
    bin/false_sharing good >> /tmp/main_results_raw.txt
    bin/false_sharing good cross-ccx >> /tmp/main_results_raw.txt
    bin/false_sharing good same-ccx >> /tmp/main_results_raw.txt
done

for i in {1..10}; do
    bin/false_sharing bad >> /tmp/pr_results_raw.txt
    bin/false_sharing good >> /tmp/pr_results_raw.txt
    bin/false_sharing good cross-ccx >> /tmp/pr_results_raw.txt
    bin/false_sharing good same-ccx >> /tmp/pr_results_raw.txt
done
