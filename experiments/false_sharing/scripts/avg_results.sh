#!/bin/bash

input_file="$1"
output_file="$2"

if [ $# -ne 2 ]; then
    echo "Invalid args passed: Usage -> $0 <input_file> <output_file>"
    exit 1
fi

> "$output_file"

# Use awk to process all data in one pass
awk '
/^(Cache Aligned|False Aligned|False Sharing) time: [0-9]+\.[0-9]+ seconds/ {
    # Extract the type (everything before " time:")
    match($0, /^(.+) time:/, arr)
    type = arr[1]
    
    # Extract the time value
    match($0, /time: ([0-9]+\.[0-9]+)/, arr)
    time = arr[1]
    
    if (type && time) {
        sum[type] += time
        count[type]++
    }
}
END {
    # Output in specific order
    types[1] = "Cache Aligned"
    types[2] = "False Aligned" 
    types[3] = "False Sharing"
    
    for (i = 1; i <= 3; i++) {
        type = types[i]
        if (count[type] > 0) {
            avg = sum[type] / count[type]
            printf "%s time: %.6f seconds\n", type, avg
        }
    }
}' "$input_file" >> "$output_file"

echo "Average calculated and saved to $output_file"
