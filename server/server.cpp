#include <print>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "absl/log/initialize.h"
#include "server_service.grpc.pb.h"

using server_service::ServerServiceInterface;
using server_service::MessageRequest;
using server_service::MessageResponse;

constexpr auto SERVER_ADDRESS = "localhost:9090";

class ServerServiceImpl : public ServerServiceInterface::Service
{
	
};


void RunServer(std::string_view address) 
{
  ServerServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address.data(), grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
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
