#include <print>
#include <memory>
#include <string>
#include <string_view>

#include <grpcpp/grpcpp.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "hello_world.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using hello_world::Greeter;
using hello_world::HelloReply;
using hello_world::HelloRequest;

constexpr auto SERVER_ADDRESS = "localhost:9090";

class GreeterClient 
{
 public:
 	GreeterClient(std::shared_ptr<Channel> channel) : m_stub(Greeter::NewStub(channel)) {}
  
  // Assembles the client's payload, sends it and presents the response back
  // from the server.     
  std::string SayHello(std::string_view user)
  {
	  // Data we are sending to the server.
	  HelloRequest request;
	  request.set_name(user);
	
	  // Container for the data we expect from the server.
	  HelloReply reply;
	
	  // Context for the client. It could be used to convey extra information to
	  // the server and/or tweak certain RPC behaviors.
	  ClientContext context;
	  // The actual RPC.
	  Status status = m_stub->SayHello(&context, request, &reply);
	
	  // Act upon its status.
	  if (status.ok()) 
	    return reply.message();
	  
		std::println("status.error_code = {}", static_cast<int>(status.error_code()));
		std::println("status.error_message = {}", status.error_message());
    return "RPC failed";
  }
  
  private:
   std::unique_ptr<Greeter::Stub> m_stub;
};


int main()
{
	GreeterClient greeter(grpc::CreateChannel(SERVER_ADDRESS, grpc::InsecureChannelCredentials()));
  
  std::string reply = greeter.SayHello("world");
  std::println("Greeter received: {}", reply);
  return 0;
}
