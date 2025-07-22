#!/bin/bash

echo "Building..."
clang++ -std=c++20 src/ring_buffer.cpp -o ring_buffer -Iinclude
echo "Added binary: ring_buffer"
