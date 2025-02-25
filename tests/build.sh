#!/bin/bash
# Build script for testbed
set echo on

echo "Building Tests..."

cd ..

mkdir -p bin
make -f "Makefile.tests.linux.mak" all 
if [ $? -ne 0 ]; then
    echo "Error: $?"
    exit 1
fi

echo "Tests built successfully."