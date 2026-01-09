#include <print>

#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <absl/log/initialize.h>

#include "services/auth_service_impl.hpp"
#include "services/rooms_service_impl.hpp"

constexpr auto SERVER_ADDRESS = "localhost:9090";

int main()
{
  absl::InitializeLog();
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  
  auto auth_service = AuthServiceImpl{};
  auth_service.Initialize();
  auto rooms_service = RoomsServiceImpl{};
  rooms_service.Initialize();
  
  auto builder = grpc::ServerBuilder{};
  builder.AddListeningPort(SERVER_ADDRESS, grpc::InsecureServerCredentials());
  builder.RegisterService(&auth_service);
  builder.RegisterService(&rooms_service);
  
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::println("Server listening on {}", SERVER_ADDRESS);

  // Wait for the server to shutdown
  server->Wait();
  return 0;
}
