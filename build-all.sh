#!/bin/bash
# Build Everything

echo "Building everything..."

# Engine
make -f "Makefile.engine.linux.mak" all
if [ $? -ne 0 ]; then
    echo "Error: $?"
    exit 1
fi

# Testbed
make -f "Makefile.testbed.linux.mak" all
if [ $? -ne 0 ]; then
    echo "Error: $?"
    exit 1
fi

# Tests
make -f "Makefile.tests.linux.mak" all
if [ $? -ne 0 ]; then
    echo "Error: $?"
    exit 1
fi

echo "All assemblies built successfully."
