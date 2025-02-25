#!/bin/bash
# Build script for engine
set echo on

echo "Building Engine..."

cd ..

mkdir -p bin
make -f "Makefile.engine.linux.mak" all 
if [ $? -ne 0 ]; then
    echo "Error: $?"
    exit 1
fi

echo "Engine built successfully."