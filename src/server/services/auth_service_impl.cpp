#include "auth_service_impl.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <fstream>
#include <cassert>
#include <print>
#include <string>
#include <string_view>
#include <mutex>

#include <grpcpp/support/status.h>
#include <csv.h>


grpc::Status AuthServiceImpl::LoginProcedure(grpc::ServerContext* context, 
																						const auth_service::AuthRequest* request, 
																						auth_service::AuthResponse* response)
{
	auto in_username = std::string_view{ request->username() };
	auto in_password = std::string_view{ request->password() };	
	std::println("[LoginProcedure] received: '{}','{}'", in_username, in_password);
	
	auto username = std::array<char, max_len_username>{};
	auto password = std::array<char, max_len_password>{};
	auto user_found = this->find_user_record(request->username(), username, password);
	auto is_password_correct = (in_password.compare(password.data()) == 0);
	
	std::println("user_found: {}", user_found);
	std::println("is_password_correct: {}", is_password_correct);
	
	response->set_auth_success(user_found && is_password_correct);
	
	if(!user_found)
	{
		response->set_auth_message("User not found!");
	}
	else if(user_found && !is_password_correct)
	{
		response->set_auth_message("Password incorrect!");
	}
	else if(user_found && is_password_correct)
	{
		auto msg = std::format("Authentication successful: username={} client_id={}", in_username, m_next_client_id);
		response->set_auth_message(msg.c_str());
		response->set_client_id(m_next_client_id);
		
		std::lock_guard<std::mutex> guard{ m_client_id_mutex };
		m_next_client_id++;
	}
	return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::SignupProcedure(grpc::ServerContext* context, 
																							const auth_service::AuthRequest* request, 
																							auth_service::AuthResponse* response)
{
	auto in_username = std::string_view{ request->username() };
	auto in_password = std::string_view{ request->password() };	
	std::println("[SignupProcedure] received: '{}','{}'", in_username, in_password);

	auto username = std::array<char, max_len_username>{};
	auto password = std::array<char, max_len_password>{};
	auto user_found = this->find_user_record(request->username(), username, password);
	if(user_found)
	{
		std::println("This username is already taken");
		response->set_auth_success(false);
		response->set_auth_message("This username is already taken");
		return grpc::Status::OK;
	}
	
	// Password checking
	auto auth_message = std::array<char, max_len_auth_message>{};
	if(!validate_password(in_password, auth_message))
	{
		std::println("Password checking false: {}", auth_message.data());
		response->set_auth_success(false);
		response->set_auth_message(auth_message.data());
		return grpc::Status::OK;
	}
	std::println("Password checking true");

	auth_message.fill(0);
	auto user_created = this->create_user(in_username, in_password, auth_message);
	if(!user_created)
	{
		std::println("Error on creating user: {}", auth_message.data());
		response->set_auth_success(false);
		response->set_auth_message(auth_message.data());
		return grpc::Status::OK;
	}
	
	response->set_auth_success(true);
	response->set_auth_message("User account created successfully!");
	response->set_client_id(m_next_client_id);
	
	std::lock_guard<std::mutex> guard{ m_client_id_mutex };
	m_next_client_id++;
	return grpc::Status::OK;
}


// ==================================
// Private methods 
// ==================================

bool AuthServiceImpl::find_user_record(std::string_view username, 
																			 std::array<char, max_len_username>& user_out,
																			 std::array<char, max_len_password>& password_out) const
{
	user_out.fill(0);
	password_out.fill(0);
	
	auto reader = io::CSVReader<2>(db_users); // 2 -> username, password
	reader.read_header(io::ignore_extra_column, "username", "password");

	auto tmp_username = std::string{};
	auto tmp_password = std::string{};
	tmp_username.reserve(max_len_username);
	tmp_password.reserve(max_len_password);
  while(reader.read_row(tmp_username, tmp_password))
  {
  	if(tmp_username == username)
	  {
	   	std::copy_n(tmp_username.begin(), max_len_username, user_out.begin());
	   	std::copy_n(tmp_password.begin(), max_len_password, password_out.begin());
			return true;
	  }
  }
  return false;
}

bool AuthServiceImpl::validate_password(std::string_view password,
																				std::array<char, max_len_auth_message>& auth_message) const
{
	auto has_upper = false; 
	auto has_lower = false; 
	auto has_digit = false; 
	auto has_special = false;
  constexpr auto special_chars = std::string_view{ "!@#$%^&*_-+=?" };
  for (char c : password) 
  {
    if (std::isupper(static_cast<unsigned char>(c))) has_upper = true;
    else if (std::islower(static_cast<unsigned char>(c))) has_lower = true;
    else if (std::isdigit(static_cast<unsigned char>(c))) has_digit = true;
    else if (special_chars.find(c) != std::string_view::npos) has_special = true;
  }
  
  auth_message.fill(0);
  auto offset = 0u;
  auto is_valid = true;  
 	if (password.length() < 8 || password.length() > 16) 
	{
		constexpr auto msg = std::string_view{ "Password must be between 8 and 16 characters.\n" } ;
		std::copy_n(msg.begin(), msg.size(), auth_message.begin() + offset);
		offset += msg.size();
    is_valid = false;
  }
  if (!has_upper) 
  {
  	constexpr auto msg = std::string_view{ "Password needs at least one uppercase letter.\n" } ;
   	std::copy_n(msg.begin(), msg.size(), auth_message.begin() + offset);
    offset += msg.size();
    is_valid = false;
  } 
  if (!has_lower)
  {
 		constexpr auto msg = std::string_view{ "Password needs at least one uppercase letter.\n" };
 		std::copy_n(msg.begin(), msg.size(), auth_message.begin() + offset);	
   	offset += msg.size(); 
    is_valid = false;
  }
  if (!has_digit) 
  {
		constexpr auto msg = std::string_view{ "Password needs at least one number.\n" };
		std::copy_n(msg.begin(), msg.size(), auth_message.begin() + offset);
		offset += msg.size(); 
   	is_valid = false;
  } 
  if (!has_special) 
  {
		constexpr auto msg = std::string_view{ "Password needs at least one special character.\n" };
		std::copy_n(msg.begin(), msg.size(), auth_message.begin() + offset);
		offset += msg.size();
  	is_valid = false;
  }
  assert(offset <= max_len_auth_message);
	return is_valid;
}

bool AuthServiceImpl::create_user(std::string_view username, 
																	std::string_view password,
																	std::array<char, max_len_auth_message>& auth_message)
{
	auth_message.fill(0);
	
	// Il mutex garantisce che un solo thread alla volta scriva nel file
	std::lock_guard<std::mutex> guard(m_db_users_mutex);
	std::ofstream os(db_users, std::ios_base::app);
	if(!os)
	{
		constexpr auto msg = "error on opening file users.db";
		std::copy_n(msg, std::strlen(msg), auth_message.begin());
		return false;
	}
	
	os << username << "," << password << '\n';
	return true;
}
