#!/bin/bash

scripts/build.sh
echo "Started aggregation for 10 iterations..."
scripts/aggregate.sh

scripts/avg_results.sh /tmp/main_results_raw.txt /tmp/main_results.txt
scripts/avg_results.sh /tmp/pr_results_raw.txt /tmp/pr_results.txt
echo "Finished aggregation and averaging."

scripts/check_performance_regression.sh /tmp/main_results.txt /tmp/pr_results.txt

