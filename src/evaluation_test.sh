#!/bin/bash 
if [ "$#" -ne 1 ]; then
	echo "Usage: $0 /mout/point/path"
	exit -1
fi

g++ -c -std=c++11 write_test.cpp
g++ -o evaluation write_test.o
#for ((i=0; i < 10; i++))
#do
    ./evaluation $1
#done
