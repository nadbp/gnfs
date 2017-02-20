#!/bin/bash 
if [ "$#" -ne 1 ]; then
	echo "Usage: $0 fileName"
	exit -1
fi

g++ -c -std=c++11 ReadWrite.cpp
g++ -o ReadWrite ReadWrite.o
./ReadWrite $1