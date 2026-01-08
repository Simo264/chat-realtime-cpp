#include "auth_service_impl.hpp"

#include <algorithm>
#include <cstdlib>
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
	
	auto userid = ClientID{ invalid_client_id };
	
	// Sezione critica: lettura condivisa
	{
    std::shared_lock lock(m_db_users_mutex);
    auto password = std::array<char, max_len_password>{};
    auto user_found = this->find_user_record_by_username(in_username, userid, password);
    auto is_password_correct = (in_password.compare(password.data()) == 0);
    if (!user_found || !is_password_correct) 
    {
   		std::println("[LoginProcedure] Invalid username or password");
    	return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Invalid username or password");
    }

	} // fine sezione critica
	
	std::println("[LoginProcedure] Authentication successful");
	response->set_client_id(userid);
	return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::SignupProcedure(grpc::ServerContext* context, 
																							const auth_service::AuthRequest* request, 
																							auth_service::AuthResponse* response)
{
	auto in_username = std::string_view{ request->username() };
	auto in_password = std::string_view{ request->password() };	
	std::println("[SignupProcedure] received: '{}','{}'", in_username, in_password);

	auto error_message = std::array<char, max_len_auth_message>{};
	
	// username checking
	if(!this->validate_username(in_username, error_message))
	{
		std::println("[SignupProcedure] INVALID_ARGUMENT: {}", error_message.data());
		return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message.data());
	}
	// Password checking
	if(!this->validate_password(in_password, error_message))
	{
		std::println("[SignupProcedure] INVALID_ARGUMENT: {}", error_message.data());
		return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message.data());
	}
	
	// sezione critica: scrittura esclusiva. Blocca tutti i lettori e tutti gli altri scrittori 	
	{
		std::unique_lock lock(m_db_users_mutex);
		
		auto userid = ClientID{ invalid_client_id };
		auto password = std::array<char, max_len_password>{};
		
		// Controllo se ci sono valori duplicati di "username"
		auto user_found = this->find_user_record_by_username(in_username, userid, password);
		if(user_found)
		{
			std::println("[SignupProcedure] ALREADY_EXISTS: this username is already taken");
			return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "This username is already taken");
		}
		
		if(m_next_client_id == invalid_client_id)
      m_next_client_id = this->get_next_userid();

		ClientID current_id = m_next_client_id.fetch_add(1);
    this->create_user(current_id, in_username, in_password);
    response->set_client_id(current_id);
	} // fine sezione critica

	return grpc::Status::OK;
}


// ==================================
// Private methods 
// ==================================

bool AuthServiceImpl::find_user_record_by_username(std::string_view in_username,
																									ClientID& out_userid,
																									std::array<char, max_len_password>& out_password) const
{
	out_userid = invalid_client_id;
	out_password.fill(0);
	
	auto reader = io::CSVReader<3>(db_users); // 3 -> userid, username, password
	reader.read_header(io::ignore_extra_column, "userid", "username", "password");
	
	auto field_userid = std::string{};
	auto field_username = std::string{};
	auto field_password = std::string{};
	field_userid.reserve(8);
	field_username.reserve(max_len_username);
	field_password.reserve(max_len_password);
 	while(reader.read_row(field_userid, field_username, field_password))
  {
  	if(field_username == in_username)
	  {
			out_userid = static_cast<ClientID>(std::stoi(field_userid));
	   	std::copy_n(field_password.begin(), max_len_password, out_password.begin());
			return true;
	  }
  }
	return false;
}
		
bool AuthServiceImpl::find_user_record_by_userid(ClientID in_userid,
																								std::array<char, max_len_username>& out_username,
																								std::array<char, max_len_password>& out_password) const 
{
	out_username.fill(0);
	out_password.fill(0);
	
	auto reader = io::CSVReader<3>(db_users); // 3 -> userid, username, password
	reader.read_header(io::ignore_extra_column, "userid", "username", "password");
	
	auto field_userid = std::string{};
	auto field_username = std::string{};
	auto field_password = std::string{};
	field_userid.reserve(8);
	field_username.reserve(max_len_username);
	field_password.reserve(max_len_password);
 	while(reader.read_row(field_userid, field_username, field_password))
  {
  	if(static_cast<ClientID>(std::stoi(field_userid)) == in_userid)
	  {
	   	std::copy_n(field_username.begin(), max_len_password, out_username.begin());
	   	std::copy_n(field_password.begin(), max_len_password, out_password.begin());
			return true;
	  }
  }
	return false;
}

bool AuthServiceImpl::validate_username(std::string_view username,
																				std::array<char, max_len_auth_message>& error_message) const
{
  error_message.fill(0);

  if (username.empty()) 
  {
  	std::format_to_n(error_message.begin(), max_len_username, "Username cannot be empty");
    return false;
  }
  if (username.length() >= max_len_username) 
  {
 		std::format_to_n(error_message.begin(), max_len_username, "Username is too long (max {} characters)", max_len_username);
    return false;
  }
  if (std::any_of(username.begin(), username.end(), ::isspace)) 
  {
		std::format_to_n(error_message.begin(), max_len_username, "Username cannot contain spaces.");
    return false;
  }

  return true;
}

bool AuthServiceImpl::validate_password(std::string_view password,
																				std::array<char, max_len_auth_message>& error_message) const
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
  
  error_message.fill(0);
  auto offset = 0u;
  auto is_valid = true;  
 	if (password.length() < 8 || password.length() > 16) 
	{
		constexpr auto msg = std::string_view{ "Password must be between 8 and 16 characters.\n" } ;
		std::copy_n(msg.begin(), msg.size(), error_message.begin() + offset);
		offset += msg.size();
    is_valid = false;
  }
  if (!has_upper) 
  {
  	constexpr auto msg = std::string_view{ "Password needs at least one uppercase letter.\n" } ;
   	std::copy_n(msg.begin(), msg.size(), error_message.begin() + offset);
    offset += msg.size();
    is_valid = false;
  } 
  if (!has_lower)
  {
 		constexpr auto msg = std::string_view{ "Password needs at least one uppercase letter.\n" };
 		std::copy_n(msg.begin(), msg.size(), error_message.begin() + offset);	
   	offset += msg.size(); 
    is_valid = false;
  }
  if (!has_digit) 
  {
		constexpr auto msg = std::string_view{ "Password needs at least one number.\n" };
		std::copy_n(msg.begin(), msg.size(), error_message.begin() + offset);
		offset += msg.size(); 
   	is_valid = false;
  } 
  if (!has_special) 
  {
		constexpr auto msg = std::string_view{ "Password needs at least one special character.\n" };
		std::copy_n(msg.begin(), msg.size(), error_message.begin() + offset);
		offset += msg.size();
  	is_valid = false;
  }
  assert(offset <= max_len_auth_message);
	return is_valid;
}

void AuthServiceImpl::create_user(ClientID userid,
																	std::string_view username, 
																	std::string_view password)
{
	std::ofstream os(db_users, std::ios_base::app);
	if(!os)
	{
		std::println("Error on opening file {}", db_users.string());
		exit(EXIT_FAILURE);
	}
	os << static_cast<int>(userid) << "," << username << "," << password << '\n';
}

ClientID AuthServiceImpl::get_next_userid()
{
	auto reader = io::CSVReader<3>(db_users); // 3 -> userid, username, password
	reader.read_header(io::ignore_extra_column, "userid", "username", "password");
	
	auto field_userid = std::string{};
	auto field_username = std::string{};
	auto field_password = std::string{};
	field_userid.reserve(8);
	field_username.reserve(max_len_username);
	field_password.reserve(max_len_password);
	
	auto max_id = ClientID{ 0 };
  auto has_records = false;
 	while(reader.read_row(field_userid, field_username, field_password))
  {
  	auto current_id = static_cast<ClientID>(std::stoull(field_userid));
   	if (current_id > max_id)
      max_id = current_id;
    
    has_records = true;
  }
  
  if(!has_records)
  	return 0;
  
	return max_id + 1;
}