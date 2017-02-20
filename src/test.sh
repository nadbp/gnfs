#!/bin/bash 
if [ "$#" -ne 1 ]; then
	echo "Usage: $0 path/to/mount/point"
	exit -1
fi

cd $1

#create a folder
mkdir test_folder
cd test_folder

#create a file
touch test_file.txt

#write something to file
echo "wow, 
it works!" >> test_file.txt

# tar
cd ..
tar cf test_archive.tar test_folder

# untar
rm -rf test_folder
tar -xvf test_archive.tar

echo "Please check directory $1 for 'test_archive.tar' and 'test_folder'. "

exit 0



