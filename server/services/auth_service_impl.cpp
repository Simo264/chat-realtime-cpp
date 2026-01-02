#include "auth_service_impl.hpp"
#include "grpcpp/support/status.h"

#include "csv.h"

#include <print>
#include <filesystem>

grpc::Status AuthServiceImpl::LoginProcedure(grpc::ServerContext* context, 
																						const AuthRequest* request, 
																						AuthResponse* response)
{
	std::println("[LoginProcedure] received: {},{}", request->username(), request->password());
	
	static auto db_users = std::filesystem::current_path() / "database/users.csv";
	constexpr auto nr_columns = 2; // username, password
	auto user_found = false;
	auto is_password_correct = false;
	try
	{
		io::CSVReader<nr_columns> reader(db_users);
		// Legge l'header (la prima riga del file)
		reader.read_header(io::ignore_extra_column, "username", "password");

		auto username = std::string{};
		auto password = std::string{};
		username.reserve(32);
		password.reserve(32);
	  while(reader.read_row(username, password))
	  {
			if(username == request->username())
			{
				user_found = true;
				if(password == request->password())
				{
					is_password_correct = true;
					break;
				}
				else
					break;
			}
	  }
	}
	catch (const io::error::can_not_open_file& e) 
	{
		std::println("Error on opening file {}", db_users.string());
		return grpc::Status::CANCELLED;
  }
  catch (const io::error::base& e) 
  {
		std::println("Error on reading CSV file: {}", e.what());
		return grpc::Status::CANCELLED;
  }

	response->set_auth_success(user_found && is_password_correct);

  if(!user_found)
  	response->set_auth_message("User not found!");
  else if(user_found && !is_password_correct)
 		response->set_auth_message("Password incorrect!");
  else if(user_found && is_password_correct)
		response->set_auth_message("Authentication successful!");
  
	return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::SignupProcedure(grpc::ServerContext* context, 
																							const AuthRequest* request, 
																							AuthResponse* response)
{
	return grpc::Status::OK;
}