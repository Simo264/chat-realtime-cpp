#pragma once

#include <memory>

#include "grpcpp/grpcpp.h"
#include "grpcpp/channel.h"
#include "grpcpp/support/status.h"
#include "grpcpp/client_context.h"

#include "auth_service.grpc.pb.h"
#include "auth_service.pb.h"

using auth_service::AuthServiceInterface;
using auth_service::AuthRequest;
using auth_service::AuthResponse;

class AuthServiceConnector
{
	public:
		AuthServiceConnector(std::shared_ptr<grpc::Channel> channel) 
		: m_stub{ AuthServiceInterface::NewStub(channel) } {}
		
		grpc::Status LoginProcedure(const AuthRequest& request, 
																AuthResponse& response, 
																grpc::ClientContext& context);
		
		grpc::Status SignupProcedure(const AuthRequest& request, 
																AuthResponse& response, 
																grpc::ClientContext& context);	
	private:
		std::shared_ptr<AuthServiceInterface::Stub> m_stub;
};