#include "rooms_service_impl.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <grpcpp/server_context.h>

#include <grpcpp/support/status.h>
#include <print>
#include <string>

#include <csv.h>

// ==================================
// Public methods 
// ==================================

void RoomsServiceImpl::Initialize()
{	
	auto reader = io::CSVReader<4>(db_rooms);
	reader.read_header(io::ignore_extra_column, "creator_id", "room_name", "room_id", "is_delete");
 	
	auto field_creator_id = std::string{};
	auto field_room_name = std::string{};
	auto field_room_id = std::string{};
	auto field_is_delete = std::string{};
	
	m_next_room_id = RoomID{ 0 };
	m_room_users.clear();
	m_user_rooms.clear();
	
	while(reader.read_row(field_creator_id, field_room_name, field_room_id, field_is_delete))
	{
		// if room is deleted
		if(std::stoi(field_is_delete) == 1) 
			continue;

		auto current_id = static_cast<RoomID>(std::stoul(field_room_id));
    auto creator_id = static_cast<ClientID>(std::stoul(field_creator_id));
		
   	if (current_id > m_next_room_id)
      m_next_room_id = current_id;
   
    auto info = RoomInfo{};
    info.room_id = current_id;
    info.creator_id = creator_id;
    info.client_set = {};
    
    info.room_name.fill(0);
    std::copy_n(field_room_name.begin(), max_len_room_name - 1, info.room_name.begin());
    
    m_room_users[current_id] = std::move(info);	
	}
}

grpc::Status RoomsServiceImpl::CreateRoomProcedure(grpc::ServerContext* context,
																									const rooms_service::CreateRoomProcedureRequest* request,
																									rooms_service::CreateRoomProcedureResponse* response)
{
	auto in_room_name = std::string_view{ request->room_name() };
	auto in_creator_id = request->creator_id();
	std::println("[CreateRoomProcedure] room_name={}, creator_id={}", in_room_name.data(), in_creator_id);
	
	auto error_message = std::array<char, max_len_error_message>{};
	if(!this->validate_room_name(in_room_name, error_message))
	{
		std::println("INVALID_ARGUMENT: {}", error_message.data());
		return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message.data());
	}

	if(!this->check_duplicate(in_room_name))
	{
		std::println("ALREADY_EXISTS: a room with this name already exists.");
		return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "A room with this name already exists.");
	}
		
	auto current_room_id = m_next_room_id;
	this->insert_new_room_db(current_room_id, in_creator_id, in_room_name);
	
	auto new_info = RoomInfo{};
  new_info.room_id = current_room_id;
  new_info.creator_id = in_creator_id;
  new_info.room_name.fill(0);
  std::copy_n(in_room_name.begin(), max_len_room_name - 1, new_info.room_name.begin());
  m_room_users[current_room_id] = std::move(new_info);
	
	auto room_proto = response->mutable_room();
  room_proto->set_room_id(current_room_id);
  room_proto->set_creator_id(in_creator_id);
  room_proto->set_room_name(in_room_name);
  room_proto->set_user_count(0);
  
  m_next_room_id++;
	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::DeleteRoomProcedure(grpc::ServerContext* context,
																									const rooms_service::DeleteRoomProcedureRequest* request,
																									rooms_service::DeleteRoomProcedureResponse* response)
{
	auto in_room_id = static_cast<RoomID>(request->room_id());
	auto in_client_id = static_cast<ClientID>(request->client_id());
	std::println("[DeleteRoomProcedure] in_room_id={}, in_client_id={}", in_room_id, in_client_id);
	
	auto find_room = m_room_users.find(in_room_id);	
	if(find_room == m_room_users.end())
	{
		std::println("Room not found or already deleted.");
		return grpc::Status(grpc::StatusCode::NOT_FOUND, "Room not found or already deleted.");
	}
	
	const auto& info = find_room->second;
	// solo il proprietario della stanza può eliminare la stanza
	if(in_client_id != info.creator_id)
	{
		std::println("You do not have permission to delete this room.");
		return grpc::Status(grpc::StatusCode::PERMISSION_DENIED, "You do not have permission to delete this room.");
	}

	if(info.client_set.size() > 0)
	{
		std::println("Cannot delete a non-empty room.");
		return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Cannot delete a non-empty room.");
	}
 
	this->mark_room_as_deleted_db(in_room_id);
	m_room_users.erase(find_room);
	return grpc::Status::OK;
}


grpc::Status RoomsServiceImpl::ListRoomsProcedure(grpc::ServerContext* context, 
																									const rooms_service::ListRoomsProcedureRequest* request, 
																									rooms_service::ListRoomsProcedureResponse* response)
{
	for (const auto& [id, info] : m_room_users) 
  {
    auto proto_room = response->add_rooms();
    proto_room->set_room_id(info.room_id);
    proto_room->set_creator_id(info.creator_id);
    proto_room->set_room_name(info.room_name.data());
  }
	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::JoinRoomProcedure(grpc::ServerContext* context,
								  															const rooms_service::JoinRoomProcedureRequest* request,
								                 								rooms_service::JoinRoomProcedureResponse* response)
{
	auto client_id = static_cast<ClientID>(request->client_id());
	auto room_id = static_cast<RoomID>(request->room_id());
	std::println("[JoinRoomProcedure] client_id={}, room_id={}", client_id, room_id);
	
	auto it_room = m_room_users.find(room_id);
	if(it_room == m_room_users.end())
		return grpc::Status(grpc::StatusCode::NOT_FOUND, "This room does not exist.");
	
	auto& info = it_room->second;
	if (info.client_set.find(client_id) != info.client_set.end()) 
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "This user is already in this room.");
	
	info.client_set.insert(client_id);
  m_user_rooms.at(client_id).insert(room_id);
	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::LeaveRoomProcedure(grpc::ServerContext* context,
									  															const rooms_service::LeaveRoomProcedureRequest* request,
									                 								rooms_service::LeaveRoomProcedureResponse* response)
{
	auto client_id = static_cast<ClientID>(request->client_id());
	auto room_id = static_cast<RoomID>(request->room_id());
	
	auto it_room = m_room_users.find(room_id);
  if (it_room == m_room_users.end()) 
  {
  	std::println("This room does not exist in memory.");	
   	return grpc::Status(grpc::StatusCode::NOT_FOUND, "This room does not exist in memory.");
  }
	
  auto& info = it_room->second;
  
  auto it_user_in_room = info.client_set.find(client_id);
  if (it_user_in_room == info.client_set.end())
  {
	 	std::println("This user is not in this room.");	
	  return grpc::Status(grpc::StatusCode::NOT_FOUND, "This user is not in this room.");
  }
  
  info.client_set.erase(it_user_in_room);

  // Rimozione della stanza dal set dell'utente
  auto it_user_rooms = m_user_rooms.find(client_id);
  if (it_user_rooms != m_user_rooms.end()) 
    it_user_rooms->second.erase(room_id);

	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::ListRoomUsersProcedure(grpc::ServerContext* context, 
								                                      const rooms_service::ListRoomUsersProcedureRequest* request, 
								                                      rooms_service::ListRoomUsersProcedureResponse* response)
{
	auto in_room_id = static_cast<RoomID>(request->room_id());
	std::println("[ListRoomUsersProcedure] in_room_id={}", in_room_id);
	auto it = m_room_users.find(in_room_id);
	if (it == m_room_users.end()) 
	{
    std::println("The specified room does not exist.");  
		return grpc::Status(grpc::StatusCode::NOT_FOUND, "The specified room does not exist.");
  }

	const auto& info = it->second;
  for (auto client_id : info.client_set) 
    response->add_client_ids(static_cast<uint32_t>(client_id));
	
	return grpc::Status::OK;
}

// ==================================
// Private methods 
// ==================================

bool RoomsServiceImpl::validate_room_name(std::string_view room_name, std::array<char, max_len_error_message>& error_message) const
{
	error_message.fill(0);
  if (room_name.empty())
  {
  	std::format_to(error_message.begin(), "Room name cannot be empty.");
	  return false;
  }
  if (room_name.length() > max_len_room_name) 
  {
   	std::format_to(error_message.begin(), "Room name is too long (max {} characters).", max_len_room_name);
    return false;
  }
  if (std::any_of(room_name.begin(), room_name.end(), ::isspace)) 
  {
	 	std::format_to(error_message.begin(), "Room name cannot contain spaces.");
    return false;
  }
  return true;
}

bool RoomsServiceImpl::check_duplicate(std::string_view room_name)
{
	for (const auto& [id, info] : m_room_users) 
	{
    if (room_name == info.room_name.data()) 
      return false;
  }
  return true;
}

void RoomsServiceImpl::insert_new_room_db(RoomID room_id, ClientID creator_id, std::string_view room_name)
{
	std::ofstream os(db_rooms, std::ios_base::app);
	if(!os)
	{
		std::println("Error on opening file {}", db_users.string());
		exit(EXIT_FAILURE);
	}
	os << static_cast<int>(room_id) << "," << static_cast<int>(creator_id) << "," << room_name.data() << ",0\n";
}

void RoomsServiceImpl::mark_room_as_deleted_db(RoomID room_id)
{
	std::fstream file(db_rooms, std::ios::in | std::ios::out | std::ios::binary);
	if(!file)
	{
		std::println("Error on opening file {}", db_users.string());
		exit(EXIT_FAILURE);
	}
	
	std::array<char, 8> target_id_str{};
	std::format_to(target_id_str.begin(), "{},", room_id);
	
	auto line = std::string{};
	line.reserve(64);
	while (std::getline(file, line))
	{
		if (!line.starts_with(target_id_str.data()))
			continue;
		
		auto last_comma = line.find_last_of(',');
		auto current_flag = line.back(); // L'ultimo carattere è lo '0' o '1'
		if (current_flag == '0'){}
		
		auto current_pos = file.tellg();
		auto line_ending_size = 1; // getline rimuove il terminatore, quindi dobbiamo ricalcolarlo.
		file.seekp(current_pos - std::streamoff(line_ending_size + 1));
		file.put('1'); // Sovrascriviamo '0' con '1'
		file.flush();  // Forza la scrittura su disco
		return;
	}
}
