#pragma once

#include "auth_service.grpc.pb.h"
#include "auth_service.pb.h"
#include "../constants.hpp"
#include <string_view>

using auth_service::AuthServiceInterface;
using auth_service::AuthRequest;
using auth_service::AuthResponse;
class AuthServiceImpl : public AuthServiceInterface::Service
{
	public:
		grpc::Status LoginProcedure(grpc::ServerContext* context, 
																const AuthRequest* request, 
																AuthResponse* response) override;
		
		grpc::Status SignupProcedure(grpc::ServerContext* context, 
																const AuthRequest* request, 
																AuthResponse* response) override;
	private:
		bool find_user_record(std::string_view username, 
													std::array<char, max_len_username>& user_out,
													std::array<char, max_len_password>& password_out) const;
		
		bool validate_password(std::string_view password, 
													std::array<char, max_len_auth_message>& auth_message) const;
		
		bool create_user(std::string_view username, 
										std::string_view password,
										std::array<char, max_len_auth_message>& auth_message);
};