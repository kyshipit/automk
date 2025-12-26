#!/bin/bash

# AutoMLOS Automotive Code Quality Check Script
# ISO 26262 ASIL-D compliant code quality validation

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SOURCE_DIRS=("kernel" "lib" "services" "platform")
HEADER_DIRS=("include" "kernel/core" "lib" "services")
MAX_LINE_LENGTH=100
MAX_FUNCTION_LENGTH=50

# Tools check
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        return 1
    fi
    return 0
}

# Header check
check_headers() {
    echo -e "${BLUE}=== Checking file headers ===${NC}"
    local error_count=0
    
    for dir in "${SOURCE_DIRS[@]}"; do
        while IFS= read -r -d '' file; do
            if [[ "$file" == *.c || "$file" == *.h ]]; then
                if ! head -n 10 "$file" | grep -q "AutoMLOS Team"; then
                    echo -e "${YELLOW}Missing AutoMLOS header: $file${NC}"
                    ((error_count++))
                fi
            fi
        done < <(find "$dir" -type f -name "*.c" -o -name "*.h" -print0)
    done
    
    if [ $error_count -eq 0 ]; then
        echo -e "${GREEN}All files have proper headers${NC}"
    else
        echo -e "${RED}Found $error_count files with missing headers${NC}"
    fi
    return $error_count
}

# Line length check
check_line_length() {
    echo -e "${BLUE}=== Checking line length ===${NC}"
    local error_count=0
    
    for dir in "${SOURCE_DIRS[@]}"; do
        while IFS= read -r -d '' file; do
            if grep -n ".\{$((MAX_LINE_LENGTH + 1))\}" "$file" | grep -v "http" | grep -v "//.*>.*<"; then
                echo -e "${YELLOW}Line length exceeded in $file:${NC}"
                grep -n ".\{$((MAX_LINE_LENGTH + 1))\}" "$file" | head -5
                ((error_count++))
            fi
        done < <(find "$dir" -type f -name "*.c" -o -name "*.h" -print0)
    done
    
    if [ $error_count -eq 0 ]; then
        echo -e "${GREEN}All lines within $MAX_LINE_LENGTH characters${NC}"
    else
        echo -e "${RED}Found $error_count files with long lines${NC}"
    fi
    return $error_count
}

# Function length check
check_function_length() {
    echo -e "${BLUE}=== Checking function length ===${NC}"
    local error_count=0
    
    for dir in "${SOURCE_DIRS[@]}"; do
        while IFS= read -r -d '' file; do
            if [[ "$file" == *.c ]]; then
                # Simple function length check (line count between { and })
                local long_functions=$(awk '
                    /^[a-zA-Z_].*{/ { 
                        in_function=1; start_line=NR; brace_count=0 
                    }
                    in_function {
                        if (/{/) brace_count++
                        if (/}/) brace_count--
                        if (brace_count == 0 && in_function) {
                            if (NR - start_line > '"$MAX_FUNCTION_LENGTH"') {
                                print "Function at line", start_line, "is too long:", NR - start_line, "lines"
                            }
                            in_function=0
                        }
                    }
                ' "$file")
                
                if [ -n "$long_functions" ]; then
                    echo -e "${YELLOW}Long functions in $file:${NC}"
                    echo "$long_functions"
                    ((error_count++))
                fi
            fi
        done < <(find "$dir" -type f -name "*.c" -print0)
    done
    
    if [ $error_count -eq 0 ]; then
        echo -e "${GREEN}All functions within $MAX_FUNCTION_LENGTH lines${NC}"
    else
        echo -e "${RED}Found $error_count files with long functions${NC}"
    fi
    return $error_count
}

# Comment quality check
check_comments() {
    echo -e "${BLUE}=== Checking comment quality ===${NC}"
    local error_count=0
    
    for dir in "${SOURCE_DIRS[@]}"; do
        while IFS= read -r -d '' file; do
            if [[ "$file" == *.c ]]; then
                # Check for functions without Doxygen comments
                local functions_without_comments=$(grep -n "^[a-zA-Z_].*[^;]$" "$file" | \
                    grep -v "//" | grep -v "/\*" | head -10)
                
                if [ -n "$functions_without_comments" ]; then
                    echo -e "${YELLOW}Functions without Doxygen comments in $file:${NC}"
                    echo "$functions_without_comments"
                    ((error_count++))
                fi
            fi
        done < <(find "$dir" -type f -name "*.c" -print0)
    done
    
    if [ $error_count -eq 0 ]; then
        echo -e "${GREEN}All functions have proper documentation${NC}"
    else
        echo -e "${RED}Found $error_count files with undocumented functions${NC}"
    fi
    return $error_count
}

# Safety check for critical functions
check_safety_functions() {
    echo -e "${BLUE}=== Checking safety-critical functions ===${NC}"
    local error_count=0
    
    # List of safety-critical functions that must have null checks
    local safety_functions=("memory_alloc" "interrupt_register_handler" "log_client_send_entry")
    
    for function in "${safety_functions[@]}"; do
        local files_with_function=$(grep -r -l "$function" "${SOURCE_DIRS[@]}" --include="*.c")
        
        for file in $files_with_function; do
            if ! grep -A5 -B5 "$function" "$file" | grep -q "NULL_CHECK\|SAFETY_CHECK"; then
                echo -e "${YELLOW}Safety function $function in $file may lack null checks${NC}"
                ((error_count++))
            fi
        done
    done
    
    if [ $error_count -eq 0 ]; then
        echo -e "${GREEN}All safety functions have proper checks${NC}"
    else
        echo -e "${RED}Found $error_count potential safety issues${NC}"
    fi
    return $error_count
}

# Main execution
main() {
    echo -e "${GREEN}=== AutoMLOS Code Quality Check ===${NC}"
    echo -e "Safety Level: ASIL-D (ISO 26262)"
    echo -e "Compliance: AUTOSAR, MISRA C 2012"
    echo
    
    # Check required tools
    check_tool "grep" || exit 1
    check_tool "find" || exit 1
    check_tool "awk" || exit 1
    
    local total_errors=0
    
    # Run all checks
    check_headers || ((total_errors++))
    echo
    
    check_line_length || ((total_errors++))
    echo
    
    check_function_length || ((total_errors++))
    echo
    
    check_comments || ((total_errors++))
    echo
    
    check_safety_functions || ((total_errors++))
    echo
    
    # Final summary
    if [ $total_errors -eq 0 ]; then
        echo -e "${GREEN}=== Code Quality Check PASSED ===${NC}"
        echo -e "All automotive coding standards met"
        exit 0
    else
        echo -e "${RED}=== Code Quality Check FAILED ===${NC}"
        echo -e "Found $total_errors categories of issues"
        echo -e "Please fix the reported issues for ASIL-D compliance"
        exit 1
    fi
}

# Run main function
main "$@"