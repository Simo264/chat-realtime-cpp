#include "auth_service_connector.hpp"

#include "grpcpp/support/status.h"

grpc::Status AuthServiceConnector::LoginProcedure(const AuthRequest& request, 
																									AuthResponse& response, 
																									grpc::ClientContext& context)
{
	auto status = m_stub->LoginProcedure(&context, request, &response);
	return status;
}

grpc::Status AuthServiceConnector::SignupProcedure(const AuthRequest& request, 
																									AuthResponse& response, 
																									grpc::ClientContext& context)
{
	auto status = m_stub->SignupProcedure(&context, request, &response);
	return status;
}