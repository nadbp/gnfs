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
using helloworld::Empty;
using helloworld::Directory;

 void translatePath(const char* client_path,char * server_path){
   strcat(server_path,"./798");
   strcat(server_path+4,client_path);
   server_path[strlen(server_path)] = '\0';
}

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {

 

  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
    std::string prefix("Hello ");
    std::cout<<request->name()<<std::endl;
    reply->set_message(prefix + request->name());
    return Status::OK;
  }

  Status xmp_mkdir(ServerContext* context, const Request* request,
                  Empty* empty) override {
    char server_path[512] ={0};
    translatePath(request->path().c_str(),server_path);
    mkdir(server_path, request->mode());
    return Status::OK;
  }

  Status grpc_getattr(ServerContext* context, const Path* client_path,
                  Stbuf* stbuf) override{
    
    char server_path[512] ={0};
    translatePath(client_path->path().c_str(),server_path);
    printf("Server : %s, Path : %s, Translated path: %s\n",__FUNCTION__,client_path->path().c_str(), server_path);

    struct stat stemp = {0};

  if(lstat(server_path,&stemp)==-1){
    //int error = errno;
    //printf("error in the server %d:\n",error);
    perror(strerror(errno));
    return Status::CANCELLED;
  }else{
    stbuf->set_stmode(stemp.st_mode);
    stbuf->set_stnlink(stemp.st_nlink);
    stbuf->set_stsize(stemp.st_size);
  }  
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
        return Status::CANCELLED;
      }
      while ((de = readdir(dp)) != NULL){
        directory.set_dname(de->d_name);
        directory.set_dino(de->d_ino);
        directory.set_dtype(de->d_type);
        writer->Write(directory);
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
  RunServer();

  return 0;
}
