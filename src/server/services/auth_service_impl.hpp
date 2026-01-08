#pragma once

#include <string_view>
#include <shared_mutex>
#include <atomic>

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
		bool find_user_record_by_username(std::string_view in_username,
																			ClientID& out_userid,
																			std::array<char, max_len_password>& out_password) const;
		
		bool find_user_record_by_userid(ClientID in_userid,
																		std::array<char, max_len_username>& out_username,
																		std::array<char, max_len_password>& out_password) const;
		
		bool validate_username(std::string_view username, 
													std::array<char, max_len_error_message>& error_message) const;	
		
		bool validate_password(std::string_view password, 
													std::array<char, max_len_error_message>& error_message) const;
		
		void create_user(ClientID client_id,
										std::string_view username, 
										std::string_view password);
	
		ClientID get_next_userid();
		
		// shared mutex: multipli lettori e un singolo scrittore
		std::shared_mutex m_db_users_mutex;
		// protezione semplice per l'aggiornamento del contatore
		std::atomic<ClientID> m_next_client_id{ invalid_client_id };
};