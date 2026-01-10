#include "auth_service_connector.hpp"
#include <algorithm>
#include <grpcpp/support/status.h>

ClientID AuthServiceConnector::CallRemoteLoginProcedure(std::string_view username, 
																												std::string_view password,
																												std::array<char, max_len_error_message>& out_error_message)
{
	auto request = auth_service::AuthRequest{};
	request.set_username(username.data());
	request.set_password(password.data());
	auto response = auth_service::AuthResponse{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->LoginProcedure(&context, request, &response);
	if(status.ok())
		return response.client_id();
	
	auto server_error = status.error_message();
	out_error_message.fill(0);
	std::copy_n(server_error.begin(), max_len_error_message - 1, out_error_message.begin());	
	return invalid_client_id;
}

ClientID AuthServiceConnector::CallRemoteSignupProcedure(std::string_view username, 
																												std::string_view password,
																												std::array<char, max_len_error_message>& out_error_message)
{
	auto request = auth_service::AuthRequest{};
	request.set_username(username.data());
	request.set_password(password.data());
	auto response = auth_service::AuthResponse{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->SignupProcedure(&context, request, &response);
	if(status.ok())
		return response.client_id();
	
	auto server_error = status.error_message();
	out_error_message.fill(0);
	std::copy_n(server_error.begin(), max_len_error_message - 1, out_error_message.begin());
	return invalid_client_id;
}