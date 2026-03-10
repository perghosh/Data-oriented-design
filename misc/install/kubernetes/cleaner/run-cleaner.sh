#!/bin/bash
#
# @file run-cleaner.sh
# @brief Convenience script for running cleaner on Kubernetes pods
#
# This script provides an easy interface to execute the cleaner application
# on any directory inside the Kubernetes pods.
#
# @usage ./run-cleaner.sh [command] [options]
#
# @PROJECT [name: cleaner-k8s] [summary: Kubernetes deployment for FileCleaner tool]
#

set -euo pipefail

# Script version
readonly VERSION="1.0.0"

# Default values
DEPLOYMENT_NAME="cleaner-112"
CONTAINER_NAME="cleaner-container"
NAMESPACE=""
CLEANER_CMD="count"
SOURCE_DIR="/app/www"
RECURSIVE_DEPTH=1
OUTPUT_FILE=""
VERBOSE=false
INTERACTIVE=false
LIST_PODS=false

# Color codes for output
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly NC='\033[0m' # No Color

# @brief Print colored message
# @param $1 Color code
# @param $2 Message to print
print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# @brief Print error message
# @param $1 Error message
print_error() {
    print_color "$RED" "ERROR: $1" >&2
}

# @brief Print success message
# @param $1 Success message
print_success() {
    print_color "$GREEN" "$1"
}

# @brief Print info message
# @param $1 Info message
print_info() {
    print_color "$BLUE" "$1"
}

# @brief Print warning message
# @param $1 Warning message
print_warning() {
    print_color "$YELLOW" "$1"
}

# @brief Print usage information
show_usage() {
    cat << EOF
Usage: $(basename "$0") [OPTIONS] COMMAND [COMMAND_OPTIONS]

Convenience script for running cleaner on Kubernetes pods.

COMMANDS:
    count           Count lines in files (default)
    list            List lines matching patterns
    find            Search for patterns in files
    dir             List files in directories
    help            Show cleaner help
    shell           Open interactive shell in pod

OPTIONS:
    -d, --deployment NAME    Deployment name (default: cleaner-112)
    -c, --container NAME     Container name (default: cleaner-container)
    -n, --namespace NAME     Kubernetes namespace
    -s, --source DIR         Source directory to analyze (default: /app/www)
    -r, --recursive DEPTH    Recursion depth (default: 1, use 0 for unlimited)
    -o, --output FILE        Save output to file (local machine)
    -p, --pod NAME           Specific pod to run on (instead of deployment)
    -l, --list-pods          List available pods and exit
    -v, --verbose            Enable verbose output
    -i, --interactive        Run in interactive mode
    -h, --help               Show this help message

COMMON COMMAND OPTIONS:
    For 'count':
        --filter PATTERN     Filter files (e.g., "*.cpp;*.h")
        --comment CHARS      Comment delimiters (e.g., "/**/")
        --string CHARS       String delimiters (e.g., "\"\"")

    For 'list':
        --pattern PATTERN    Search pattern
        --context NUM        Context lines (1 or 2 numbers)
        --expression EXPR    Expression for custom processing

    For 'find':
        --pattern PATTERN    Search pattern
        --filter PATTERN     Filter files
        --icase              Ignore case
        --word               Match whole words

EXAMPLES:
    # Count lines in /app/www with depth 1
    $(basename "$0") --source /app/www count

    # Count with custom filter and depth 3
    $(basename "$0") -s /data/code -r 3 count --filter "*.cpp;*.h"

    # Search for "function" pattern
    $(basename "$0") find --pattern "function" --context 2

    # Save output to local file
    $(basename "$0") -o results.txt count --source /app/www

    # List available pods
    $(basename "$0") --list-pods

    # Open interactive shell
    $(basename "$0") shell

    # Run with verbose output
    $(basename "$0") -v count --source /usr/local/bin

VERSION: $VERSION

@PROJECT [name: cleaner-k8s] [summary: Kubernetes deployment for FileCleaner tool]
EOF
}

# @brief Parse command line arguments
parse_args() {
    local cleaner_opts=()
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--deployment)
                DEPLOYMENT_NAME="$2"
                shift 2
                ;;
            -c|--container)
                CONTAINER_NAME="$2"
                shift 2
                ;;
            -n|--namespace)
                NAMESPACE="$2"
                shift 2
                ;;
            -s|--source)
                SOURCE_DIR="$2"
                shift 2
                ;;
            -r|--recursive)
                RECURSIVE_DEPTH="$2"
                shift 2
                ;;
            -o|--output)
                OUTPUT_FILE="$2"
                shift 2
                ;;
            -p|--pod)
                POD_NAME="$2"
                shift 2
                ;;
            -l|--list-pods)
                LIST_PODS=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -i|--interactive)
                INTERACTIVE=true
                shift
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            count|list|find|dir|help|shell)
                CLEANER_CMD="$1"
                shift
                # Remaining arguments are for cleaner
                cleaner_opts=("$@")
                break
                ;;
            *)
                cleaner_opts+=("$1")
                shift
                ;;
        esac
    done
    
    # Store cleaner options for later use
    CLEANER_OPTS=("${cleaner_opts[@]}")
}

# @brief Get kubectl command with namespace if specified
get_kubectl_cmd() {
    local cmd="kubectl"
    if [[ -n "$NAMESPACE" ]]; then
        cmd="$cmd -n $NAMESPACE"
    fi
    echo "$cmd"
}

# @brief Get target (pod or deployment)
get_target() {
    if [[ -n "${POD_NAME:-}" ]]; then
        echo "pod/$POD_NAME"
    else
        echo "deployment/$DEPLOYMENT_NAME"
    fi
}

# @brief List available pods
list_pods() {
    local kubectl_cmd=$(get_kubectl_cmd)
    local target=$(get_target)
    
    print_info "Listing pods for deployment: $DEPLOYMENT_NAME"
    print_info "------------------------------------------------"
    $kubectl_cmd get pods -l app=$DEPLOYMENT_NAME
    print_info "------------------------------------------------"
    print_info "Use -p/--pod option to target a specific pod"
}

# @brief Run cleaner command
run_cleaner() {
    local kubectl_cmd=$(get_kubectl_cmd)
    local target=$(get_target)
    
    # Build cleaner command
    local cleaner_args=(
        "--source" "$SOURCE_DIR"
        "--recursive" "$RECURSIVE_DEPTH"
        "${CLEANER_OPTS[@]}"
    )
    
    # Handle special commands
    if [[ "$CLEANER_CMD" == "shell" ]]; then
        print_info "Opening interactive shell in: $target"
        $kubectl_cmd exec -it $target -c $CONTAINER_NAME -- sh
        return $?
    fi
    
    if [[ "$CLEANER_CMD" == "help" ]]; then
        print_info "Getting cleaner help"
        $kubectl_cmd exec -it $target -c $CONTAINER_NAME -- cleaner help
        return $?
    fi
    
    # Build full command
    local full_cmd="cleaner $CLEANER_CMD ${cleaner_args[*]}"
    
    if $VERBOSE; then
        print_info "Running: $full_cmd"
        print_info "Target: $target"
    fi
    
    # Execute command
    local output=""
    if $INTERACTIVE; then
        $kubectl_cmd exec -it $target -c $CONTAINER_NAME -- $full_cmd
    else
        if [[ -n "$OUTPUT_FILE" ]]; then
            print_info "Saving output to: $OUTPUT_FILE"
            $kubectl_cmd exec $target -c $CONTAINER_NAME -- $full_cmd > "$OUTPUT_FILE"
            print_success "Output saved to: $OUTPUT_FILE"
        else
            $kubectl_cmd exec $target -c $CONTAINER_NAME -- $full_cmd
        fi
    fi
}

# @brief Save output to pod and download
save_output_to_pod() {
    local kubectl_cmd=$(get_kubectl_cmd)
    local target=$(get_target)
    local temp_file="/tmp/cleaner_output_$(date +%s).txt"
    
    # Run cleaner and save to pod
    print_info "Running cleaner and saving output to pod: $temp_file"
    $kubectl_cmd exec $target -c $CONTAINER_NAME -- \
        bash -c "cleaner $CLEANER_CMD --source $SOURCE_DIR --recursive $RECURSIVE_DEPTH ${CLEANER_OPTS[*]} > $temp_file"
    
    # Copy to local machine
    print_info "Copying output to: $OUTPUT_FILE"
    $kubectl_cmd exec $target -c $CONTAINER_NAME -- cat $temp_file > "$OUTPUT_FILE"
    
    # Cleanup
    $kubectl_cmd exec $target -c $CONTAINER_NAME -- rm -f $temp_file
    
    print_success "Output saved to: $OUTPUT_FILE"
}

# @brief Verify deployment exists
verify_deployment() {
    local kubectl_cmd=$(get_kubectl_cmd)
    
    if [[ -n "${POD_NAME:-}" ]]; then
        if ! $kubectl_cmd get pod "$POD_NAME" &>/dev/null; then
            print_error "Pod '$POD_NAME' not found"
            exit 1
        fi
    else
        if ! $kubectl_cmd get deployment "$DEPLOYMENT_NAME" &>/dev/null; then
            print_error "Deployment '$DEPLOYMENT_NAME' not found"
            exit 1
        fi
    fi
}

# @brief Check if cleaner binary exists in pod
check_cleaner_binary() {
    local kubectl_cmd=$(get_kubectl_cmd)
    local target=$(get_target)
    
    if ! $kubectl_cmd exec $target -c $CONTAINER_NAME -- which cleaner &>/dev/null; then
        print_error "Cleaner binary not found in container"
        exit 1
    fi
}

# @brief Main function
main() {
    # Parse arguments
    parse_args "$@"
    
    # Handle list pods
    if $LIST_PODS; then
        list_pods
        exit 0
    fi
    
    # Verify deployment
    verify_deployment
    
    # Check cleaner binary
    check_cleaner_binary
    
    # Run cleaner command
    run_cleaner
}

# Run main function
main "$@"