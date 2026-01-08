#include "auth_service_connector.hpp"
#include <grpcpp/support/status.h>

grpc::Status AuthServiceConnector::CallRemoteLoginProcedure(const AuthRequest& request, 
																														AuthResponse& response, 
																														grpc::ClientContext& context)
{
	auto status = m_stub->LoginProcedure(&context, request, &response);
	return status;
}

grpc::Status AuthServiceConnector::CallRemoteSignupProcedure(const AuthRequest& request, 
																														AuthResponse& response, 
																														grpc::ClientContext& context)
{
	auto status = m_stub->SignupProcedure(&context, request, &response);
	return status;
}