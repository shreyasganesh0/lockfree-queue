#!/bin/bash
set -euo pipefail # strict error handling

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Performance Regression Check${NC}"
echo "============================"

if [ $# -ne 2 ]; then
    echo -e "${RED}ERROR: Invalid args passed${NC}"
    echo "Usage: $0 <main_results_file> <pr_results_file>"
    exit 1
fi

MAIN_RESULTS_FILE="$1"
PR_RESULTS_FILE="$2"

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
        echo -e "${RED}ERROR: No data found for '$test_type' in $file${NC}" >&2
        echo "0"
        return 1
    fi
    
    if ! echo "$value" | grep -qE '^[0-9]+\.[0-9]+$'; then
        echo -e "${RED}ERROR: Invalid value '$value' for '$test_type' in $file${NC}" >&2
        echo "0"
        return 1
    fi
    
    echo -e "${BLUE}  Extracted '$test_type' from $(basename "$file"): ${value}s${NC}" >&2
    echo "$value"
}

# Function to validate extracted value
validate_value() {
    local value="$1"
    local test_name="$2"
    local file_type="$3"
    
    if [ -z "$value" ] || [ "$value" = "0" ]; then
        echo -e "${RED}ERROR: Could not extract valid data for $test_name from $file_type results${NC}"
        return 1
    fi
    
    return 0
}

echo -e "${BLUE}Extracting performance values...${NC}"

echo "Main branch results:"
MAIN_FALSE_SHARING=$(extract_value "$MAIN_RESULTS_FILE" "False Sharing")
MAIN_FALSE_ALIGNED=$(extract_value "$MAIN_RESULTS_FILE" "False Aligned")
MAIN_CACHE_ALIGNED=$(extract_value "$MAIN_RESULTS_FILE" "Cache Aligned")

echo "PR branch results:"
PR_FALSE_SHARING=$(extract_value "$PR_RESULTS_FILE" "False Sharing")
PR_FALSE_ALIGNED=$(extract_value "$PR_RESULTS_FILE" "False Aligned")
PR_CACHE_ALIGNED=$(extract_value "$PR_RESULTS_FILE" "Cache Aligned")

echo -e "\n${BLUE}Validating extracted data...${NC}"
validate_value "$MAIN_FALSE_SHARING" "False Sharing" "main" || exit 1
validate_value "$MAIN_FALSE_ALIGNED" "False Aligned" "main" || exit 1
validate_value "$MAIN_CACHE_ALIGNED" "Cache Aligned" "main" || exit 1
validate_value "$PR_FALSE_SHARING" "False Sharing" "PR" || exit 1
validate_value "$PR_FALSE_ALIGNED" "False Aligned" "PR" || exit 1
validate_value "$PR_CACHE_ALIGNED" "Cache Aligned" "PR" || exit 1

echo -e "${GREEN}✓ All data validation passed${NC}"

# Function to calculate and display regression
calculate_regression() {
    local main_value=$1
    local pr_value=$2
    local test_name=$3
    local threshold=${4:-5} # Default threshold is 5%
    
    local percentage=$(echo "scale=3; (($pr_value - $main_value) / $main_value) * 100" | bc)
    local abs_percentage=$(echo "scale=3; if ($percentage < 0) -$percentage else $percentage" | bc)
    local is_regression=$(echo "$abs_percentage > $threshold" | bc)
    local is_improvement=$(echo "$percentage < -$threshold" | bc)
    
    printf "%-15s | Main: %8.6fs | PR: %8.6fs | Change: %+7.2f%% | " "$test_name" "$main_value" "$pr_value" "$percentage"
    
    if [ "$is_regression" -eq 1 ] && [ "$is_improvement" -eq 0 ]; then
        echo -e "${RED}REGRESSION (>${threshold}%)${NC}"
        return 1
    elif [ "$is_improvement" -eq 1 ]; then
        echo -e "${GREEN}IMPROVEMENT (>${threshold}%)${NC}"
        return 0
    else
        echo -e "${YELLOW}ACCEPTABLE (<=${threshold}%)${NC}"
        return 0
    fi
}

echo -e "\n${BLUE}Performance Comparison Results:${NC}"
echo "=================================================================================="
printf "%-15s | %-12s | %-12s | %-12s | %s\n" "Test Name" "Main Branch" "PR Branch" "Change" "Status"
echo "=================================================================================="

FAILED=0
TOTAL_TESTS=0

echo -n "False Sharing   | "
if ! calculate_regression "$MAIN_FALSE_SHARING" "$PR_FALSE_SHARING" "False Sharing"; then
    FAILED=1
fi
((TOTAL_TESTS++))

echo -n "False Aligned   | "
if ! calculate_regression "$MAIN_FALSE_ALIGNED" "$PR_FALSE_ALIGNED" "False Aligned"; then
    FAILED=1
fi
((TOTAL_TESTS++))

echo -n "Cache Aligned   | "
if ! calculate_regression "$MAIN_CACHE_ALIGNED" "$PR_CACHE_ALIGNED" "Cache Aligned"; then
    FAILED=1
fi
((TOTAL_TESTS++))

echo "=================================================================================="

echo -e "\n${BLUE}Summary:${NC}"
PASSED_TESTS=$((TOTAL_TESTS - FAILED))
if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All $TOTAL_TESTS tests passed - No performance regressions detected${NC}"
else
    echo -e "${RED}✗ $FAILED out of $TOTAL_TESTS tests failed - Performance regression detected${NC}"
fi

CSV_FILE="performance_history.csv"
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')
GIT_HASH=$(git rev-parse HEAD 2>/dev/null || echo "unknown")
GIT_BRANCH=$(git branch --show-current 2>/dev/null || echo "unknown")

# Create CSV header if file doesn't exist
if [ ! -f "$CSV_FILE" ]; then
    echo "timestamp,git_hash,git_branch,false_sharing_time,false_aligned_time,cache_aligned_time,tests_passed,tests_failed" > "$CSV_FILE"
fi

echo "$TIMESTAMP,$GIT_HASH,$GIT_BRANCH,$PR_FALSE_SHARING,$PR_FALSE_ALIGNED,$PR_CACHE_ALIGNED,$PASSED_TESTS,$FAILED" >> "$CSV_FILE"

echo -e "${BLUE}Results logged to: $CSV_FILE${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}Performance check PASSED${NC}"
    exit 0
else
    echo -e "${RED}Performance check FAILED${NC}"
    exit 1
fi
