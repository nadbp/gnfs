#define FUSE_USE_VERSION 30

//#include <config.h>

#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>//open
#include <iostream>
#include <memory>
#include <string>
#include <cstring>

#include <grpc++/grpc++.h>

#include "helloworld.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;
using helloworld::Path;
using helloworld::Stbuf;
using helloworld::Request;
using helloworld::Errno;
using helloworld::Directory;
using helloworld::WriteRequest;
using helloworld::WriteBytes;
using helloworld::PathFlags;
using helloworld::FileHandle;
using helloworld::ReadReq;
using helloworld::Buffer;
using helloworld::Empty;
using helloworld::FlushReq;
using helloworld::RenameReq;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

int grpc_mkdir(const char *path, mode_t mode)
{
  ClientContext context;
  Request req;
  req.set_path(path);
  req.set_mode(mode);
  Errno err;

// The actual RPC.  
Status status = stub_->grpc_mkdir(&context, req, &err);
std::cout<<err.err()<<std::endl;
// Act upon its status.
    if (status.ok()) {
      return 0;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return err.err();
    }
}

int grpc_getattr(const char *client_path, struct stat *statbuf)
{
  ClientContext context;
  Path pathName;
  pathName.set_path(client_path);
  Stbuf stbuf;

// The actual RPC.  
Status status = stub_->grpc_getattr(&context, pathName, &stbuf);
    memset(statbuf, 0, sizeof(struct stat));

    statbuf->st_mode = stbuf.stmode();
	statbuf->st_nlink = stbuf.stnlink();
	statbuf->st_size = stbuf.stsize();
	std::cout << "link: " << stbuf.stnlink() << std::endl;
	std::cout << "size: *** : " << stbuf.stsize() << std::endl;
	std::cout<<"stbuf.err()="<<stbuf.err()<<std::endl;

    	std::cout<<"stbuf.err() here="<<stbuf.err()<<std::endl;
      return stbuf.err();
    	//return -2;
}

int grpc_readdir(const char *client_path, void *buf, fuse_fill_dir_t filler)
{
  ClientContext context;
  Path pathName;
  pathName.set_path(client_path);
  Directory directory;

  std::unique_ptr<ClientReader<Directory> >reader(
           stub_->grpc_readdir(&context, pathName));
   while (reader->Read(&directory)){
   	struct stat st;
	memset(&st, 0, sizeof(st));
	st.st_ino = directory.dino();
	st.st_mode = directory.dtype() << 12;
	if (filler(buf, directory.dname().c_str(), &st, 0, static_cast<fuse_fill_dir_flags>(0)))
		break; 
   }

   Status status = reader->Finish();
   // if (status.ok()) {
   //    std::cout << "readdir rpc succeeded." << std::endl;
   //    return 0;
   //  } else {
   //    std::cout << "readdir rpc failed." << std::endl;
      return directory.err();
	// }
}

int grpc_write(const char * path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    ClientContext context;
    WriteRequest req;
    req.set_path(path);
    req.set_buffer(buffer);
    req.set_size(size);
    req.set_offset(offset);
    req.set_fh(fi->fh);
    WriteBytes nbytes;

    Status s = stub_->grpc_write(&context, req, &nbytes);
    if(s.ok()) {
        std::cout<< "write rpc succeeded." << std::endl;
    } else {
        std::cout << "write rpc failed." << std::endl;
    }

    return nbytes.nbytes();

}

int grpc_flush(const char* path, struct fuse_file_info *fi) 
{
    ClientContext context;
    FlushReq req;
    req.set_path(path);
    req.set_fh(fi->fh);
    Empty empty;

    Status s = stub_->grpc_flush(&context, req, &empty);
    if(s.ok()) {
        std::cout << "flush rpc succeeded." << std::endl;
    } else {
        std::cout << "flush rpc failed." << std::endl;
        return -1;
    }
    return 0;
}

int grpc_open(const char *client_path, struct fuse_file_info *fi)
{
	ClientContext context;
  	PathFlags path_flags;
  	path_flags.set_path(client_path);
  	path_flags.set_flags(fi->flags);
  	FileHandle file_handle;
  	Status status = stub_->grpc_open(&context, path_flags, &file_handle);
  	//fh=file_handle.fh();
    fi->fh = file_handle.fh();
    return 0;
}

int grpc_read(const char *client_path, char *buf, size_t size, off_t offset, int fh)
{
	ClientContext context;
  	ReadReq read_req;
  	read_req.set_path(client_path);
  	read_req.set_size(size);
  	read_req.set_offset(offset);
  	read_req.set_fh(fh);
  	Buffer buffer;
  	Status status = stub_->grpc_read(&context, read_req, &buffer);
  	std::string buf_string(buffer.buffer());
  	std::cout<<"buf_string="<<buf_string<<std::endl;
  	std::strcpy(buf, buf_string.c_str());
  	std::cout<<"*buf="<<*buf<<std::endl;
  	return buffer.nbytes();
}

int grpc_rename(const char *from, const char *to, unsigned int flags)
{
	ClientContext context;
  	RenameReq rename_req;
  	rename_req.set_from(from);
  	rename_req.set_to(to);
  	rename_req.set_flags(flags);
  	Errno err;
  	Status status = stub_->grpc_rename(&context, rename_req, &err);
  	return err.err();
}

int grpc_rmdir(const char *path)
{
    ClientContext context;
    Path client_path;
    client_path.set_path(path);
    Errno err;
    Status status = stub_->grpc_rmdir(&context, client_path, &err);
    std::cout<<"rmdir: err.err()="<<err.err()<<std::endl;
    return err.err();
}

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};


/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
    GreeterClient *greeter;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int grpc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;
	return options.greeter->grpc_readdir(path, buf, filler);
}

static int grpc_open(const char *path, struct fuse_file_info *fi)
{
//	return options.greeter->grpc_open(path, fi->flags, fi->fh);
    return options.greeter->grpc_open(path, fi);
}

static int grpc_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	return options.greeter->grpc_read(path, buf, size, offset, fi->fh);
}

static int grpc_getattr(const char *path, struct stat *statbuf,struct fuse_file_info *fi)
{
  (void) fi;
  return options.greeter->grpc_getattr(path, statbuf);
} 

static int grpc_mkdir(const char *path, mode_t mode)
{
  //mode=S_IRWXU | S_IRWXG | S_IRWXO
  return options.greeter->grpc_mkdir(path, mode);
}

static int grpc_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info *fi) 
{
    return options.greeter->grpc_write(path, buffer, size, offset, fi);
}

static int grpc_flush(const char* path, struct fuse_file_info *fi) 
{
    return options.greeter->grpc_flush(path, fi);
}

static int grpc_rename(const char *from, const char *to, unsigned int flags) 
{
    return options.greeter->grpc_rename(from,to, flags);
}

static int grpc_rmdir(const char *path) 
{
    return options.greeter->grpc_rmdir(path);
}

static struct hello_operations : fuse_operations {
	hello_operations() {
		init    = hello_init;
		getattr	= grpc_getattr;//hello_getattr;//grpc_getattr;
		readdir	= grpc_readdir; //hello_readdir;
		open	= grpc_open; //hello_open;
		read	= grpc_read; //hello_read;
        mkdir	= grpc_mkdir;
        write   = grpc_write;
        flush   = grpc_flush;
        rename  = grpc_rename;
        rmdir  =grpc_rmdir;
    }
} hello_oper_init;

static void show_help(const char *progname)
{
	std::cout<<"usage: "<<progname<<" [options] <mountpoint>\n\n";
	/*printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");*/
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);


	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");
    options.greeter = new GreeterClient(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
	}


	return fuse_main(args.argc, args.argv, &hello_oper_init, NULL);
}
