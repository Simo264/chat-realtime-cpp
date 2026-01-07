#pragma once

#include <string_view>
#include <mutex>

#include <auth_service.grpc.pb.h>
#include <auth_service.pb.h>

#include "../../common.hpp"

class AuthServiceImpl : public auth_service::AuthServiceInterface::Service
{
	public:
		grpc::Status LoginProcedure(grpc::ServerContext* context, 
																const auth_service::AuthRequest* request, 
																auth_service::AuthResponse* response) override;
		
		grpc::Status SignupProcedure(grpc::ServerContext* context, 
																const auth_service::AuthRequest* request, 
																auth_service::AuthResponse* response) override;
	private:
		bool find_user_record(std::string_view username, 
													std::array<char, max_len_username>& user_out,
													std::array<char, max_len_password>& password_out) const;
		
		bool validate_password(std::string_view password, 
													std::array<char, max_len_auth_message>& auth_message) const;
		
		bool create_user(std::string_view username, 
										std::string_view password,
										std::array<char, max_len_auth_message>& auth_message);
		
		ClientID m_next_client_id{ 0 };
		std::mutex m_client_id_mutex;
		std::mutex m_db_users_mutex;
};