#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <thread> //this_thread::sleep_for
#include <chrono> //chrono::seconds
using namespace std;

int main(int argc, char *argv[])
{
if (argc<2){
  cout<<"Please input file name."<<endl;
  cout<<"argc="<<argc<<endl;
  return 1;
}

int fd;
u_int size=32;
int offset=0;
int read_bytes, write_bytes;
char * buffer = (char*)malloc(size); 
fd = open (argv[1] ,O_RDWR);
if (fd<0)
  perror(strerror(errno));
std::cout<<"open file handle="<<fd<<std::endl;

read_bytes = pread(fd,buffer,size, offset);
if (read_bytes<0)
  perror(strerror(errno));
cout<<"content read="<<buffer<<endl;
cout<<"# of bytes read="<<read_bytes<<endl;

//first write
write_bytes = pwrite(fd,buffer,read_bytes,offset+read_bytes);
if (write_bytes<0)
  perror(strerror(errno));
cout<<"# of bytes written="<<write_bytes<<endl;

//second write
write_bytes = pwrite(fd,buffer,read_bytes,offset+2*read_bytes);
if (write_bytes<0)
  perror(strerror(errno));
cout<<"# of bytes written="<<write_bytes<<endl;

// if (fd >0){
//   cout<<"closing the fd="<<fd<<endl;
//   close(fd);
//   cout<<"closing finish. fd="<<fd<<endl;
// }
      
free(buffer);

return 0;
}