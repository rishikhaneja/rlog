#!/bin/bash
mkdir -p build
cd build
cmake ../
make
mkdir -p outputs
./tests
echo _______DONE________