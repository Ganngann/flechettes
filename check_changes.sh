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

    # Run Native Tests if environment exists
    if grep -q "\[env:native\]" platformio.ini; then
        echo "Running unit tests (native)..."
        if pio test -e native; then
            echo -e "${GREEN}Tests Passed.${NC}"
        else
            echo -e "${RED}Tests Failed in $project_dir${NC}"
            exit 1
        fi
    else
        echo "Native environment not found, skipping unit tests."
    fi

    # Verify Compilation (esp32dev)
    echo "Verifying compilation (esp32dev)..."
    if pio run -e esp32dev > /dev/null; then
        echo -e "${GREEN}Compilation Successful.${NC}"
    else
        echo -e "${RED}Compilation Failed in $project_dir${NC}"
        # Re-run to show error
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
