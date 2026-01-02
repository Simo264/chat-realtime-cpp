#pragma once

#include "auth_service.grpc.pb.h"
#include "auth_service.pb.h"

using auth_service::AuthServiceInterface;
using auth_service::AuthRequest;
using auth_service::AuthResponse;

class AuthServiceImpl : public AuthServiceInterface::Service
{
	public:
		grpc::Status LoginProcedure(grpc::ServerContext* context, 
																const AuthRequest* request, 
																AuthResponse* response);
		
		grpc::Status SignupProcedure(grpc::ServerContext* context, 
																const AuthRequest* request, 
																AuthResponse* response);
};