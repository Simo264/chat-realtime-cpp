#include <cstdint>
#include <print>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "absl/log/initialize.h"
#include "hello_world.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using hello_world::Greeter;
using hello_world::HelloRequest;
using hello_world::HelloReply;

constexpr auto SERVER_ADDRESS = "localhost:9090";

class GreeterServiceImpl final : public Greeter::Service 
{
  Status SayHello(ServerContext* context, const HelloRequest* request, HelloReply* reply) override 
  {
  	auto req_name = request->name();
    auto response = std::format("Hello, {}", req_name);
   	std::println("Received request from {}", req_name);
    std::println("Reply with message: {}", response); 
    reply->set_message(response);
    return Status::OK;
  }
};

void RunServer(std::string_view address) 
{
  GreeterServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  builder.AddListeningPort(address.data(), grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::println("Server listening on {}", address.data());

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main()
{
  absl::InitializeLog();
  RunServer(SERVER_ADDRESS);
  return 0;
}
