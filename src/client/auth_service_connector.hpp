#pragma once

#include <array>
#include <memory>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/support/status.h>
#include <grpcpp/client_context.h>

#include <auth_service.grpc.pb.h>
#include <auth_service.pb.h>
#include <string_view>

#include "../common.hpp"

class AuthServiceConnector
{
	public:
		AuthServiceConnector(std::shared_ptr<grpc::Channel> channel) 
		: m_stub{ auth_service::AuthService::NewStub(channel) } {}
		
		[[nodiscard]] ClientID CallRemoteLoginProcedure(std::string_view username, 
																										std::string_view password,
																										std::array<char, max_len_error_message>& out_error_message);
		
		[[nodiscard]] ClientID CallRemoteSignupProcedure(std::string_view username, 
																										std::string_view password,
																										std::array<char, max_len_error_message>& out_error_message);	
	private:
		std::shared_ptr<auth_service::AuthService::Stub> m_stub;
};