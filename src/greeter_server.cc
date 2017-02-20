#include <iostream>
#include <memory>
#include <string>
#include <sys/stat.h> //mkdir()
#include <sys/types.h> //mode_t
#include <grpc++/grpc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <utime.h>
#include <thread> //this_thread::sleep_for
#include <chrono> //chrono::seconds

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using std::string;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;
using helloworld::Path;
using helloworld::Stbuf;
using helloworld::Request;
using helloworld::Errno;
using helloworld::Directory;
using helloworld::WriteBytes;
using helloworld::WriteRequest;
using helloworld::PathFlags;
using helloworld::FileHandle;
using helloworld::ReadReq;
using helloworld::Buffer;
using helloworld::Empty;
using helloworld::FlushReq;
using helloworld::RenameReq;
using helloworld::ReleaseReq;
using helloworld::CreateReq;
using helloworld::UtimeReq;

static const char* server_root_path;

void translatePath(const char* client_path,char * server_path){
 strcat(server_path,server_root_path);
 strcat(server_path+4,client_path);
 server_path[strlen(server_path)] = '\0';
}

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {

  Status grpc_read(ServerContext* context, const ReadReq* read_req, 
  Buffer* buffer)override{
  char server_path[512] ={0};
  translatePath(read_req->path().c_str(),server_path);
  printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,read_req->path().c_str(), server_path);

  std::cout<<"---------------read file_handle="<<read_req->fh()<<std::endl;

  ////******sleep for 7 seconds, test client read() timeout
  ////******sleep for 5 seconds, test server crash
  //std::this_thread::sleep_for(std::chrono::seconds(5)); 

  int file_handle;
  char * buf = (char*)malloc(read_req->size());
    int nbytes;
//use the fd passed by client to read file
 nbytes = pread(read_req->fh(),buf,read_req->size(), read_req->offset());
 if ( nbytes==-1){   //server reboot
      file_handle= open (server_path ,O_RDONLY); //use path to open the file
      if (file_handle ==0){ //path is wrong, return error to client      
        printf("failed to open %s\n",server_path);
        buffer->set_err(1);
      }else{ //path is right
        nbytes = pread(file_handle, buf,read_req->size(), read_req->offset());
        buffer->set_fh(file_handle);
        if (nbytes==-1){
          buffer->set_err(-errno);
          printf("server cannot seek at: %d\n", read_req->offset());
        }
      }
    }else{//file handle passed by client is right
      buffer->set_fh(read_req->fh());
    }

    printf("----------file handle to return to client :%d ------------\n",buffer->fh());
   if (!buffer->err()) {// no error happens
     printf("server :no of bytes read :%d \n",nbytes);

     string buf_string(buf);
   // std::cout<<"*buf="<<*buf<<std::endl;
   // std::cout<<"buf_string="<<buf_string<<std::endl;
     buffer->set_buffer(buf_string);
     buffer->set_nbytes(nbytes);
   }

   // if (file_handle >0)
   //  close(file_handle);

   free(buf);
   return Status::OK;
}

  Status grpc_write(ServerContext* context, const WriteRequest* req, WriteBytes* noBytes ) override {
    int file_handle, nbytes;
    char server_path[512] = {0};
    translatePath(req->path().c_str(), server_path);
    printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,req->path().c_str(), server_path);
    
    std::cout<<"---------------write file_handle="<<req->fh()<<std::endl;
    //use the fd passed by client to read file
    nbytes = pwrite(req->fh(), req->buffer().c_str(), req->size(), req->offset());
    std::cout<<"-----------nbytes="<<nbytes<<std::endl;
    std::cout<<"-----------error no="<<strerror(errno)<<std::endl;
    if ( nbytes==-1){  //server reboot
      file_handle= open (server_path ,O_WRONLY); //use path to open the file
      std::cout<<"---------------open file_handle="<<file_handle<<std::endl;
      if (file_handle ==0){ //path is wrong, return error to client      
        printf("failed to open %s\n",server_path);
        noBytes->set_err(1);
      }else{ //path is right
        nbytes = pwrite(file_handle, req->buffer().c_str(), req->size(), req->offset());
        noBytes->set_fh(file_handle);
        if (nbytes==-1){
          noBytes->set_err(-errno);
          printf("File system write failed zero data write\n");
        }
      }
    }else{//file handle passed by client is right
      noBytes->set_fh(req->fh());
    }

     printf("----------file handle to return to client :%d ------------\n",noBytes->fh());
   if (!noBytes->err()) {// no error happens
     printf("------------server :no of bytes written :%d \n",nbytes);
     noBytes->set_nbytes(nbytes);
   }

  //call fsync for every write operation, this is the write-through approach.
  if(noBytes->fh()) {
    printf("----------call fsync() ------------\n");
    if(fsync(noBytes->fh()) < 0) {
      perror(strerror(errno));
   }
  }

   return Status::OK;
  }

  Status grpc_mkdir(ServerContext* context, const Request* request,
    Errno* err) override {
    char server_path[512] ={0};
    translatePath(request->path().c_str(),server_path);
    printf("Server before mkdir: %s, Path : %s, Translated path: %s\n",__FUNCTION__,request->path().c_str(), server_path);
    int res=mkdir(server_path, request->mode());
    printf("Server after mkdir: %s, Path : %s, Translated path: %s\n",__FUNCTION__,request->path().c_str(), server_path);

    if(res == -1){
     perror(strerror(errno));
     err->set_err(-errno);
   }else
   err->set_err(0);
   return Status::OK;
 }

 Status grpc_create(ServerContext* context, const CreateReq* request, Errno* err) override {
  char server_path[512] ={0};
  translatePath(request->path().c_str(),server_path);
  printf("Server before mkdir: %s, Path : %s, Translated path: %s\n",__FUNCTION__,request->path().c_str(), server_path);
  int res=open(server_path, request->flag(), request->mode());
  printf("Server after mkdir: %s, Path : %s, Translated path: %s\n",__FUNCTION__,request->path().c_str(), server_path);

  if(res < 0){
   perror(strerror(errno));
   err->set_err(-errno);
 }else
 err->set_err(0);
 return Status::OK;

}

Status grpc_flush(ServerContext* context, const FlushReq* req, Errno* err) override {
  char server_path[512] = {0};
  translatePath(req->path().c_str(), server_path);
  printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,req->path().c_str(), server_path);
  err->set_err(0);

  return Status::OK;
}

Status grpc_getattr(ServerContext* context, const Path* client_path,
  Stbuf* stbuf) override{

  char server_path[512] ={0};
  translatePath(client_path->path().c_str(),server_path);
  printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,client_path->path().c_str(), server_path);

  struct stat stemp = {0};
  int res=lstat(server_path,&stemp);
  stbuf->set_stmode(stemp.st_mode);
  stbuf->set_stnlink(stemp.st_nlink);
  stbuf->set_stsize(stemp.st_size);
  if(res==-1){
    perror(strerror(errno));
    stbuf->set_err(-errno);
    printf("stbuf->err(): %d\n", stbuf->err());
    std::cout<<"-errno="<<-errno<<std::endl;
  }else
  stbuf->set_err(0);
  return Status::OK;
}

Status grpc_readdir(ServerContext* context, const Path* client_path, 
  ServerWriter<Directory>* writer)override{
  char server_path[512] ={0};
  translatePath(client_path->path().c_str(),server_path);

  Directory directory;
  DIR *dp;
  struct dirent *de;
  dp = opendir(server_path);
  if (dp == NULL){
    perror(strerror(errno));
    directory.set_err(-errno);
    return Status::CANCELLED;
  }
  while ((de = readdir(dp)) != NULL){
    directory.set_dname(de->d_name);
    directory.set_dino(de->d_ino);
    directory.set_dtype(de->d_type);
    writer->Write(directory);
  }
  directory.set_err(0);
  closedir(dp);
  return Status::OK;
}

Status grpc_open(ServerContext* context, const PathFlags* path_flags, 
  FileHandle* fh)override {
  char server_path[512] ={0};
  translatePath(path_flags->path().c_str(),server_path);
  printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,path_flags->path().c_str(), server_path);

  int file_handle= open (server_path ,path_flags->flags());
  std::cout<<"---------------open file_handle="<<file_handle<<std::endl;
  fh->set_fh(file_handle);
  if(file_handle == -1){
    fh->set_err(-errno);
    perror(strerror(errno));
  }else{    
    fh->set_err(0);    
  }
  return Status::OK;     
}

Status grpc_unlink(ServerContext* context, const Path* path, Errno * err) override {
  int res;
  char server_path[512] ={0};
  translatePath(path->path().c_str(),server_path);
  printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,path->path().c_str(), server_path);

  res = unlink(server_path);
  if(res == -1){
    err->set_err(-errno);
  } else {
    err->set_err(0);
  }

  return Status::OK;

}

Status grpc_release(ServerContext* context, const ReleaseReq* req, Errno* err) override {
  char server_path[512] ={0};
  translatePath(req->path().c_str(),server_path);
  printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,req->path().c_str(), server_path);

  std::cout<<"---------------release file handle= "<<req->fh()<<std::endl;

  if(req->fh()) {
    if(fsync(req->fh()) < 0) {
      perror(strerror(errno));
      err->set_err(-errno);
    }
    if(close(req->fh()) == -1) {
      perror(strerror(errno));
      err->set_err(-errno);
    }
  }

  err->set_err(0);
  return Status::OK;
}





Status grpc_rename(ServerContext* context, const RenameReq* rename_req, 
  Errno* err)override {
  char server_from_path[512] ={0};
  translatePath(rename_req->from().c_str(),server_from_path);
  printf("Server : -From %s, Path : %s, Translated path: %s\n",__FUNCTION__,rename_req->from().c_str(), server_from_path);

  char server_to_path[512] ={0};
  translatePath(rename_req->to().c_str(),server_to_path);
  printf("Server : -To %s, Path : %s, Translated path: %s\n",__FUNCTION__,rename_req->to().c_str(), server_to_path);

  if (rename_req->flags())
    err->set_err(-EINVAL);

  int res = rename(server_from_path, server_to_path);
  if(res == -1)
    err->set_err(-errno);
  else
    err->set_err(0);
  return Status::OK;    
}

Status grpc_rmdir(ServerContext* context, const Path* client_path, 
  Errno* err)override {
  char server_path[512] ={0};
  translatePath(client_path->path().c_str(),server_path);
  printf("server : %s, path : %s, translated path: %s\n",__FUNCTION__,client_path->path().c_str(), server_path);

  int res = rmdir(server_path);
  if(res == -1)
    err->set_err(-errno);
  else
    err->set_err(0);
  return Status::OK;    
}

Status grpc_utimens(ServerContext* context, const UtimeReq* req, Errno* err) override {
//    struct timespec time[2];
  struct utimbuf buf;
  char server_path[512] ={0};
  translatePath(req->path().c_str(),server_path);
  printf("server : %s, path : %s, translated path: %s\n",__FUNCTION__,req->path().c_str(), server_path);

  //  struct timespec at = {req->at()};
  //  struct timespec mt = {req->mt()};

  //  time[0] = at;
  //  time[1] = mt;
  //
  buf.actime = req->at();
  buf.modtime = req->mt();

  if(utime(server_path, &buf) == -1) {
    err->set_err(-errno);
  } else {
    err->set_err(0);
  }

  return Status::OK;

  
}


};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  GreeterServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  if (argc!=2){
    std::cout<<"usage: "<<argv[0]<<" server_root_path"<<std::endl;
    return 1;
  }

  server_root_path=argv[1];
  std::cout<<"server_root_path="<<server_root_path<<std::endl;
  RunServer();

  return 0;
}
