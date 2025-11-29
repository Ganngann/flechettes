#!/bin/bash
set -e

# Verify Bar project
echo "--------------------------------------"
echo "Building project: Bar"
echo "--------------------------------------"
cd projets/bar
pio run
cd ../..

# Verify Flechettes project
echo "--------------------------------------"
echo "Building project: Flechettes"
echo "--------------------------------------"
cd projets/flechettes
pio run
cd ../..

echo "--------------------------------------"
echo "All checks passed successfully!"
echo "--------------------------------------"
