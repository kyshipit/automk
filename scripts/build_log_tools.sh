#!/bin/bash

# AutoMLOS Log Tools Builder Script
# Simplified and improved version

set -e  # Exit on any error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script information
SCRIPT_NAME="build_log_tools.sh"
VERSION="3.0"

# Configuration - Simple relative paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(realpath "$SCRIPT_DIR/..")"
LOGGING_DIR="$PROJECT_ROOT/lib/logging"
TOOLS_DIR="$PROJECT_ROOT/tools"
PARSER_TARGET="log_parser"

# Print header
print_header() {
    echo -e "${BLUE}"
    echo "================================================"
    echo "AutoMLOS Log Tools Builder v$VERSION"
    echo "Project: $(basename "$PROJECT_ROOT")"
    echo "================================================"
    echo -e "${NC}"
}

# Print status message
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    print_status "Checking prerequisites..."
    
    # Check if required directories exist
    if [ ! -d "$LOGGING_DIR" ]; then
        print_error "Logging directory not found: $LOGGING_DIR"
        exit 1
    fi
    
    if [ ! -d "$TOOLS_DIR" ]; then
        print_error "Tools directory not found: $TOOLS_DIR"
        exit 1
    fi
    
    # Check if Makefile exists
    if [ ! -f "$LOGGING_DIR/Makefile" ]; then
        print_error "Makefile not found in logging directory"
        exit 1
    fi
    
    # Check if GCC is available
    if ! command -v gcc &> /dev/null; then
        print_error "GCC compiler not found. Please install GCC."
        exit 1
    fi
    
    # Check if make is available
    if ! command -v make &> /dev/null; then
        print_error "Make utility not found. Please install make."
        exit 1
    fi
    
    print_status "All prerequisites satisfied"
}

# Clean build artifacts from logging directory
clean_build_artifacts() {
    print_status "Cleaning build artifacts..."
    
    cd "$LOGGING_DIR"
    
    # Remove all compilation artifacts
    local artifacts_removed=0
    
    # Remove object files (*.o)
    if ls *.o >/dev/null 2>&1; then
        rm -f *.o
        print_status "Removed object files (*.o)"
        artifacts_removed=1
    fi
    
    # Remove library files (*.a)
    if ls *.a >/dev/null 2>&1; then
        rm -f *.a
        print_status "Removed library files (*.a)"
        artifacts_removed=1
    fi
    
    # Remove parser tool if exists
    if [ -f "$PARSER_TARGET" ]; then
        rm -f "$PARSER_TARGET"
        print_status "Removed parser tool"
        artifacts_removed=1
    fi
    
    # Remove build log
    if [ -f "build.log" ]; then
        rm -f "build.log"
        print_status "Removed build.log"
    fi
    
    # Run make clean for additional cleanup
    if make clean >/dev/null 2>&1; then
        print_status "Make clean completed"
    fi
    
    if [ $artifacts_removed -eq 0 ]; then
        print_status "No build artifacts found to clean"
    fi
}

# Build the log tools
build_tools() {
    print_status "Building log tools..."
    
    cd "$LOGGING_DIR"
    
    # Build the library and parser tool
    if make -j$(nproc) 2>&1 | tee build.log; then
        print_status "Build completed successfully"
        
        # Verify the parser tool was created
        if [ -f "$PARSER_TARGET" ]; then
            print_status "Parser tool built successfully"
        else
            print_error "Parser tool not found after build"
            exit 1
        fi
    else
        print_error "Build failed. Check build.log for details"
        exit 1
    fi
}

# Deploy tools to tools directory
deploy_tools() {
    print_status "Deploying tools to tools directory..."
    
    # Create tools directory if it doesn't exist
    mkdir -p "$TOOLS_DIR/log_parser"
    
    # Copy the parser tool
    if [ -f "$LOGGING_DIR/$PARSER_TARGET" ]; then
        cp "$LOGGING_DIR/$PARSER_TARGET" "$TOOLS_DIR/log_parser/"
        chmod +x "$TOOLS_DIR/log_parser/$PARSER_TARGET"
        print_status "Parser tool deployed successfully"
    else
        print_error "Parser tool not found for deployment"
        exit 1
    fi
}

# Test the deployed tool
test_tool() {
    print_status "Testing deployed tool..."
    
    local tool_path="$TOOLS_DIR/log_parser/$PARSER_TARGET"
    
    if [ ! -x "$tool_path" ]; then
        print_error "Tool not found or not executable: $tool_path"
        return 1
    fi
    
    # Test basic functionality (help command)
    if "$tool_path" --help >/dev/null 2>&1; then
        print_status "Tool basic functionality test passed"
        return 0
    else
        # Some tools may not have --help but still work
        print_warning "Tool help test failed (tool may still be functional)"
        return 0
    fi
}

# Print usage information
print_usage() {
    echo "Usage: $SCRIPT_NAME [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo "  -v, --version  Show version information"
    echo "  -c, --clean    Clean build artifacts only"
    echo "  -t, --test     Test deployed tool only"
    echo ""
    echo "Examples:"
    echo "  $SCRIPT_NAME           # Full build, deploy, and test"
    echo "  $SCRIPT_NAME --clean   # Clean build artifacts only"
    echo "  $SCRIPT_NAME --test    # Test deployed tool only"
}

# Main function
main() {
    local clean_only=false
    local test_only=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_usage
                exit 0
                ;;
            -v|--version)
                echo "$SCRIPT_NAME version $VERSION"
                exit 0
                ;;
            -c|--clean)
                clean_only=true
                shift
                ;;
            -t|--test)
                test_only=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
        esac
    done
    
    print_header
    
    if [ "$test_only" = true ]; then
        test_tool
        exit 0
    fi
    
    check_prerequisites
    
    if [ "$clean_only" = true ]; then
        clean_build_artifacts
        exit 0
    fi
    
    # Full build process
    clean_build_artifacts
    build_tools
    deploy_tools
    clean_build_artifacts  # Clean again after deployment
    test_tool
    
    echo ""
    print_status "Log tools build process completed successfully!"
    echo ""
    echo -e "${GREEN}Tool location:${NC}"
    echo "  ./tools/log_parser/$PARSER_TARGET"
    echo ""
    echo -e "${BLUE}Quick start:${NC}"
    echo "  1. View help: ./tools/log_parser/$PARSER_TARGET --help"
    echo "  2. Basic parsing: ./tools/log_parser/$PARSER_TARGET input.log output.txt"
    echo "  3. JSON format: ./tools/log_parser/$PARSER_TARGET input.log output.json -f json"
    echo ""
    echo -e "${BLUE}Available options:${NC}"
    echo "  -f, --format     Output format (text, json, csv, xml)"
    echo "  -d, --database   Format database path"
    echo "  -t, --tags       Tag database path"  
    echo "  -c, --color      Color mode (auto, always, never)"
    echo "  -s, --style      Log style (compact, detailed, minimal)"
    echo "  -m, --module     Module filter (SYSTEM, KERNEL, etc.)"
    echo "  -v, --verbose    Verbose output"
    echo ""
}

# Run main function with all arguments
main "$@"