#!/bin/bash
# Build script for testbed
set echo on

echo "Building Testbed..."

cd ..

mkdir -p bin
make -f "Makefile.testbed.linux.mak" all 
if [ $? -ne 0 ]; then
    echo "Error: $?"
    exit 1
fi

echo "Testbed built successfully."