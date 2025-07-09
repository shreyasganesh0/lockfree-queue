#!/bin/bash
set -euo pipefail # strict error handling

# Simple colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Performance Regression Check${NC}"
echo "============================"

# Validate arguments
if [ $# -ne 2 ]; then
    echo -e "${RED}ERROR: Invalid args passed${NC}"
    echo "Usage: $0 <main_results_file> <pr_results_file>"
    exit 1
fi

MAIN_RESULTS_FILE="$1"
PR_RESULTS_FILE="$2"

# Validate input files exist
if [ ! -f "$MAIN_RESULTS_FILE" ]; then
    echo -e "${RED}ERROR: Main results file '$MAIN_RESULTS_FILE' not found${NC}"
    exit 1
fi

if [ ! -f "$PR_RESULTS_FILE" ]; then
    echo -e "${RED}ERROR: PR results file '$PR_RESULTS_FILE' not found${NC}"
    exit 1
fi

# Function to extract value for a specific test type
extract_value() {
    local file="$1"
    local test_type="$2"
    
    local value=$(grep "^$test_type time:" "$file" | awk '{print $4}' | head -1)
    
    if [ -z "$value" ]; then
        echo "ERROR: No data found for '$test_type' in $file" >&2
        echo "0"
        return 1
    fi
    
    if ! echo "$value" | grep -qE '^[0-9]+\.[0-9]+$'; then
        echo "ERROR: Invalid value '$value' for '$test_type' in $file" >&2
        echo "0"
        return 1
    fi
    
    echo "$value"
}

# Extract all values
echo "Extracting performance values..."

MAIN_FALSE_SHARING=$(extract_value "$MAIN_RESULTS_FILE" "False Sharing")
MAIN_FALSE_ALIGNED=$(extract_value "$MAIN_RESULTS_FILE" "False Aligned")
MAIN_CACHE_ALIGNED=$(extract_value "$MAIN_RESULTS_FILE" "Cache Aligned")

PR_FALSE_SHARING=$(extract_value "$PR_RESULTS_FILE" "False Sharing")
PR_FALSE_ALIGNED=$(extract_value "$PR_RESULTS_FILE" "False Aligned")
PR_CACHE_ALIGNED=$(extract_value "$PR_RESULTS_FILE" "Cache Aligned")

# Validate extracted values
if [ "$MAIN_FALSE_SHARING" = "0" ] || [ "$MAIN_FALSE_ALIGNED" = "0" ] || [ "$MAIN_CACHE_ALIGNED" = "0" ] || \
   [ "$PR_FALSE_SHARING" = "0" ] || [ "$PR_FALSE_ALIGNED" = "0" ] || [ "$PR_CACHE_ALIGNED" = "0" ]; then
    echo -e "${RED}ERROR: Failed to extract valid data from one or both files${NC}"
    exit 1
fi

echo "Main branch results: FS=$MAIN_FALSE_SHARING FA=$MAIN_FALSE_ALIGNED CA=$MAIN_CACHE_ALIGNED"
echo "PR branch results:   FS=$PR_FALSE_SHARING FA=$PR_FALSE_ALIGNED CA=$PR_CACHE_ALIGNED"

# Function to calculate regression (simplified)
calculate_regression() {
    local main_val="$1"
    local pr_val="$2" 
    local test_name="$3"
    local threshold=5
    
    # Use awk for calculation
    local result=$(awk -v main="$main_val" -v pr="$pr_val" -v thresh="$threshold" '
    BEGIN {
        percentage = (pr - main) / main * 100
        abs_percentage = (percentage < 0) ? -percentage : percentage
        
        if (abs_percentage > thresh && percentage > 0) {
            status = "REGRESSION"
            exit_code = 1
        } else if (percentage < -thresh) {
            status = "IMPROVEMENT" 
            exit_code = 0
        } else {
            status = "ACCEPTABLE"
            exit_code = 0
        }
        
        printf "%s %.2f %d", status, percentage, exit_code
    }')
    
    local status=$(echo "$result" | cut -d' ' -f1)
    local percentage=$(echo "$result" | cut -d' ' -f2)
    local exit_code=$(echo "$result" | cut -d' ' -f3)
    
    # Print result
    printf "%-15s: Main=%8.6fs PR=%8.6fs Change=%+6.2f%% Status=" "$test_name" "$main_val" "$pr_val" "$percentage"
    
    case $status in
        "REGRESSION")
            echo -e "${RED}REGRESSION${NC}"
            return 1
            ;;
        "IMPROVEMENT")
            echo -e "${GREEN}IMPROVEMENT${NC}"
            return 0
            ;;
        *)
            echo -e "${YELLOW}ACCEPTABLE${NC}"
            return 0
            ;;
    esac
}

echo ""
echo "Performance Comparison Results:"
echo "============================================="

FAILED=0
TOTAL_TESTS=0

# Test 1: False Sharing
if ! calculate_regression "$MAIN_FALSE_SHARING" "$PR_FALSE_SHARING" "False Sharing"; then
    FAILED=$((FAILED + 1))
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))

# Test 2: False Aligned  
if ! calculate_regression "$MAIN_FALSE_ALIGNED" "$PR_FALSE_ALIGNED" "False Aligned"; then
    FAILED=$((FAILED + 1))
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))

# Test 3: Cache Aligned
if ! calculate_regression "$MAIN_CACHE_ALIGNED" "$PR_CACHE_ALIGNED" "Cache Aligned"; then
    FAILED=$((FAILED + 1))
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))

echo "============================================="

# Summary
PASSED_TESTS=$((TOTAL_TESTS - FAILED))
echo ""
echo "SUMMARY:"
echo "  Tests passed: $PASSED_TESTS/$TOTAL_TESTS"
echo "  Tests failed: $FAILED/$TOTAL_TESTS"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}OVERALL RESULT: PASSED - No performance regressions detected${NC}"
    OVERALL_STATUS="PASSED"
else
    echo -e "${RED}OVERALL RESULT: FAILED - Performance regression detected${NC}"
    OVERALL_STATUS="FAILED"
fi

# Simple CSV logging
CSV_FILE="performance_history.csv"
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
GIT_HASH=$(git rev-parse HEAD 2>/dev/null || echo "unknown")
GIT_BRANCH=$(git branch --show-current 2>/dev/null || echo "unknown")

# Create CSV header if file doesn't exist
if [ ! -f "$CSV_FILE" ]; then
    echo "timestamp,git_hash,git_branch,false_sharing_time,false_aligned_time,cache_aligned_time,tests_passed,tests_failed,overall_status" > "$CSV_FILE"
fi

# Append results to CSV
echo "$TIMESTAMP,$GIT_HASH,$GIT_BRANCH,$PR_FALSE_SHARING,$PR_FALSE_ALIGNED,$PR_CACHE_ALIGNED,$PASSED_TESTS,$FAILED,$OVERALL_STATUS" >> "$CSV_FILE"

echo ""
echo "Results logged to: $CSV_FILE"

# Clear exit indication
echo ""
echo "============================================="
if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}SCRIPT EXITING WITH SUCCESS CODE (0)${NC}"
    exit 0
else
    echo -e "${RED}SCRIPT EXITING WITH FAILURE CODE (1)${NC}"
    exit 1
fi
