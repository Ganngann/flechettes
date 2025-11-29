#!/bin/bash
set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Running mandatory tests...${NC}"

# Function to run checks for a project
check_project() {
    project_dir=$1
    echo -e "${GREEN}Checking project: $project_dir${NC}"
    cd $project_dir

    # Run Native Tests
    echo "Running unit tests (native)..."
    if pio test -e native; then
        echo -e "${GREEN}Tests Passed.${NC}"
    else
        echo -e "${RED}Tests Failed in $project_dir${NC}"
        exit 1
    fi

    # Verify Compilation (optional but recommended)
    echo "Verifying compilation (esp32dev)..."
    # We use -s to suppress output unless there is an error, and just check exit code
    if pio run -e esp32dev > /dev/null; then
        echo -e "${GREEN}Compilation Successful.${NC}"
    else
        echo -e "${RED}Compilation Failed in $project_dir${NC}"
        # Print the last few lines of log if feasible, or just fail
        pio run -e esp32dev
        exit 1
    fi

    cd - > /dev/null
}

# Check Bar
check_project "projets/bar"

# Check Flechettes
check_project "projets/flechettes"

echo -e "${GREEN}All mandatory tests passed successfully!${NC}"
