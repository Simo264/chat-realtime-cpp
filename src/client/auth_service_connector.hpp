#pragma once

#include <memory>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/support/status.h>
#include <grpcpp/client_context.h>

#include <auth_service.grpc.pb.h>
#include <auth_service.pb.h>

class AuthServiceConnector
{
	public:
		AuthServiceConnector(std::shared_ptr<grpc::Channel> channel) 
		: m_stub{ auth_service::AuthService::NewStub(channel) } {}
		
		grpc::Status CallRemoteLoginProcedure(const auth_service::AuthRequest& request, 
																					auth_service::AuthResponse& response, 
																					grpc::ClientContext& context);
		
		grpc::Status CallRemoteSignupProcedure(const auth_service::AuthRequest& request, 
																					auth_service::AuthResponse& response, 
																					grpc::ClientContext& context);	
	private:
		std::shared_ptr<auth_service::AuthService::Stub> m_stub;
};