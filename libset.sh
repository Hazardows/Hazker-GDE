#!/bin/bash
# Script for an easy task management at WSE

# This script XD
# Run this command in a terminal if the script can't be executed
# chmod +x libset.sh

# TODO: Check cleaning routines

# Build script
ls -l build-all.sh
chmod +x build-all.sh

# Clean script
ls -l clean-obj.sh
chmod +x clean-obj.sh

# Shader compiler script
ls -l post-build.sh
chmod +x post-build.sh

pushd bin

# libhazkerEngine.so
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH

echo $LD_LIBRARY_PATH
ls $(pwd)/libhazkerEngine.so

# testbed
ls -l testbed
chmod +x testbed

# tests
ls -l tests
chmod +x tests

# endl
echo ""

echo "Select an option:"
echo "1 -> Launch testbed"
echo "2 -> Launch tests"
echo "3 -> Build engine"
echo "4 -> Build testbed"
echo "5 -> Build tests"
echo "6 -> Build all"
echo "7 -> Clean object files"
echo "8 -> Post-build (build shaders)"
echo "9 -> Exit"

read -p "Your choice is: " option

echo ""

case $option in
  1)
    echo "Launching testbed..."
    ./testbed
    popd
    ;;
  2)
    echo "Launching tests..."
    ./tests
    popd
    ;;
  3)
    popd
    pushd engine
    ./build.sh
    popd
    ;;
  4)
    popd
    pushd testbed
    ./build.sh
    popd
    ;;
   5)
    popd
    pushd tests
    ./build.sh
    popd
    ;;
  6)
    popd
    ./build-all.sh
    ;;
  7)
    echo "Cleaning object files..."
    popd
    ./clean-obj.sh
    rmdir obj
    ;;
  8)
    echo "Building shaders..."
    popd
    ./post-build.sh
    ;;
  9)
    echo "Exiting..."
    popd
    exit 1
    ;;
  *)
    echo "Invalid option."
    ;;
esac