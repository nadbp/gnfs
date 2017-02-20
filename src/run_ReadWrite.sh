#!/bin/bash 
if [ "$#" -ne 1 ]; then
	echo "Usage: $0 fileName"
	exit -1
fi

g++ -o ReadWrite ReadWrite.cpp
./ReadWrite $1