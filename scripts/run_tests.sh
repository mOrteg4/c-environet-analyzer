#!/bin/bash
# Script to run unit tests for all components

# Activate virtual environment
source .venv/bin/activate

# Run tests
echo "Running unit tests..."
python -m unittest discover -s tests

echo "Tests completed."