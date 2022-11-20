#!/bin/sh
rm -rf CMakeFiles
rm -f CMakeCache.txt cmake_install.cmake Makefile siriusTest
sleep 1
cmake ../program
sleep 1
make
