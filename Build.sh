#!/bin/bash

mkdir build && cd build
cmake ..
make
cd .. && mv build/NFAMATCH . && rm -rf build
