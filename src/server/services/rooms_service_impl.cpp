#include "rooms_service_impl.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <grpcpp/server_context.h>

#include <grpcpp/support/status.h>
#include <mutex>
#include <print>
#include <string>

#include <csv.h>
#include <utility>

// ==================================
// Public methods 
// ==================================

RoomsServiceImpl::RoomsServiceImpl()
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
		auto current_id = static_cast<RoomID>(std::stoul(field_room_id));
    auto creator_id = static_cast<ClientID>(std::stoul(field_creator_id));
   	if (current_id >= m_next_room_id)
    	m_next_room_id.store(current_id + 1);
    
    auto new_room = this->create_empty_server_room_info(current_id, creator_id, field_room_name.c_str());
    m_room_users[current_id] = std::move(new_room);	
	}
}

grpc::Status RoomsServiceImpl::CreateRoomProcedure(grpc::ServerContext* context,
																									const rooms_service::CreateRoomProcedureRequest* request,
																									rooms_service::CreateRoomProcedureResponse* response)
{
	auto in_room_name = std::string_view{ request->room_name() };
	auto in_creator_id = request->creator_id();
	std::println("[CreateRoomProcedure] room_name={}, creator_id={}", in_room_name, in_creator_id);
	
	auto error_message = std::array<char, max_len_error_message>{};
	if(!this->validate_room_name(in_room_name, error_message))
	{
		std::println("INVALID_ARGUMENT: {}", error_message.data());
		return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message.data());
	}

  auto broadcast_msg = rooms_service::WatchRoomsStreamingResponse{};
  broadcast_msg.set_type(rooms_service::ROOM_EVENT_TYPE_ROOM_CREATED);
  broadcast_msg.set_actor_id(in_creator_id);
	
	{ // inizio sezione critica. Lock esclusivo: blocca tutti i lettori e tutti gli altri scrittori
		std::lock_guard lock(m_rooms_mutex);
		if(!this->check_duplicate(in_room_name))
		{
			std::println("ALREADY_EXISTS: a room with this name already exists.");
			return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "A room with this name already exists.");
		}
		
		auto current_room_id = m_next_room_id.fetch_add(1);
		this->insert_new_room_db(current_room_id, in_creator_id, in_room_name);
		
		auto new_room = this->create_empty_server_room_info(current_room_id, in_creator_id, in_room_name);
	  m_room_users[current_room_id] = std::move(new_room);
	
		auto room_proto = response->mutable_room();
	  room_proto->set_room_id(current_room_id);
	  room_proto->set_creator_id(in_creator_id);
	  room_proto->set_room_name(in_room_name);
	  room_proto->set_user_count(0);
			
		auto broadcast_room = broadcast_msg.mutable_room();
    broadcast_room->CopyFrom(*room_proto);
	} // fine sezione critica
	
	// Eseguiamo il broadcast a tutti i client connessi
  {
    std::lock_guard lock(m_mutex_subscribers);
    this->broadcast_message(broadcast_msg);
  }
	
	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::DeleteRoomProcedure(grpc::ServerContext* context,
																									const rooms_service::DeleteRoomProcedureRequest* request,
																									rooms_service::DeleteRoomProcedureResponse* response)
{
	auto in_room_id = static_cast<RoomID>(request->room_id());
	auto in_client_id = static_cast<ClientID>(request->client_id());
	std::println("[DeleteRoomProcedure] in_room_id={}, in_client_id={}", in_room_id, in_client_id);
	
	{ // inizio sezione critica. Lock esclusivo: blocca tutti i lettori e tutti gli altri scrittori
		std::lock_guard lock(m_rooms_mutex);
	
		auto find_room = m_room_users.find(in_room_id);	
		if(find_room == m_room_users.end())
		{
			std::println("Room not found or already deleted.");
			return grpc::Status(grpc::StatusCode::NOT_FOUND, "Room not found or already deleted.");
		}
		
		const auto& server_room = find_room->second;
		if(in_client_id != server_room.creator_id)
		{
			// solo il proprietario della stanza può eliminare la stanza
			std::println("You do not have permission to delete this room.");
			return grpc::Status(grpc::StatusCode::PERMISSION_DENIED, "You do not have permission to delete this room.");
		}
		if(server_room.client_set.size() > 0)
		{
			std::println("Cannot delete a non-empty room.");
			return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, "Cannot delete a non-empty room.");
		}
 
		this->mark_room_as_deleted_db(in_room_id);
		m_room_users.erase(find_room);	
	} // fine sezione critica

	// broadcast a tutti
	{
		auto broadcast_msg = rooms_service::WatchRoomsStreamingResponse{};
	  broadcast_msg.set_type(rooms_service::ROOM_EVENT_TYPE_ROOM_DELETED);
	  broadcast_msg.set_room_id(in_room_id);
		broadcast_msg.set_actor_id(in_client_id);
  
    std::lock_guard lock(m_mutex_subscribers);
    this->broadcast_message(broadcast_msg);
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
	
	auto broadcast_msg = rooms_service::WatchRoomsStreamingResponse{};
  broadcast_msg.set_type(rooms_service::ROOM_EVENT_TYPE_USER_JOINED);
  broadcast_msg.set_actor_id(client_id);
  broadcast_msg.set_room_id(room_id);
  
	{ // inizio sezione critica. Lock esclusivo: blocca tutti i lettori e tutti gli altri scrittori
		std::lock_guard lock(m_rooms_mutex);
		
		auto it_room = m_room_users.find(room_id);
		if(it_room == m_room_users.end())
			return grpc::Status(grpc::StatusCode::NOT_FOUND, "This room does not exist.");
				
		auto& server_room = it_room->second;
		if (server_room.client_set.find(client_id) != server_room.client_set.end()) 
	    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "This user is already in this room.");
				
		server_room.client_set.insert(client_id);
	  m_user_rooms[client_id].insert(room_id);
			
		auto proto_room = broadcast_msg.mutable_room();
    proto_room->set_room_id(server_room.room_id);
    proto_room->set_creator_id(server_room.creator_id);
    proto_room->set_room_name(server_room.room_name.data());
    proto_room->set_user_count(static_cast<uint32_t>(server_room.client_set.size()));
	} // fine sezione critica
	
	// Invio broadcast
  {
    std::lock_guard lock(m_mutex_subscribers);
    this->broadcast_message(broadcast_msg);
  }

	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::LeaveRoomProcedure(grpc::ServerContext* context,
									  															const rooms_service::LeaveRoomProcedureRequest* request,
									                 								rooms_service::LeaveRoomProcedureResponse* response)
{
	auto client_id = static_cast<ClientID>(request->client_id());
	auto room_id = static_cast<RoomID>(request->room_id());
	std::println("[LeaveRoomProcedure] client_id={}, room_id={}", client_id, room_id);
	
	auto broadcast_msg = rooms_service::WatchRoomsStreamingResponse{};
  broadcast_msg.set_type(rooms_service::ROOM_EVENT_TYPE_USER_LEFT);
  broadcast_msg.set_room_id(room_id);
  broadcast_msg.set_actor_id(client_id);
	
	{ // inizio sezione critica. Lock esclusivo: blocca tutti i lettori e tutti gli altri scrittori
		std::lock_guard lock(m_rooms_mutex);

		auto it_room = m_room_users.find(room_id);
	  if (it_room == m_room_users.end()) 
	  {
	  	std::println("This room does not exist in memory.");	
	   	return grpc::Status(grpc::StatusCode::NOT_FOUND, "This room does not exist in memory.");
	  }
			
	  auto& serve_room = it_room->second;
	  auto it_user_in_room = serve_room.client_set.find(client_id);
	  if (it_user_in_room == serve_room.client_set.end())
	  {
			 	std::println("This user is not in this room.");	
			  return grpc::Status(grpc::StatusCode::NOT_FOUND, "This user is not in this room.");
	  }
	  
	  serve_room.client_set.erase(it_user_in_room);
	
	  // Rimozione della stanza dal set dell'utente
	  auto it_user_rooms = m_user_rooms.find(client_id);
	  if (it_user_rooms != m_user_rooms.end()) 
	    it_user_rooms->second.erase(room_id);
			
		auto proto_room = broadcast_msg.mutable_room();
    proto_room->set_room_id(serve_room.room_id);
    proto_room->set_room_name(serve_room.room_name.data());
    proto_room->set_creator_id(serve_room.creator_id);
    proto_room->set_user_count(static_cast<uint32_t>(serve_room.client_set.size()));
	} // fine sezione critica
	
	// Invio broadcast
  {
    std::lock_guard lock(m_mutex_subscribers);
    this->broadcast_message(broadcast_msg);
  }

	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::WatchRoomsStreaming(grpc::ServerContext* context, 
																									const rooms_service::WatchRoomsStreamingRequest* request, 
																									grpc::ServerWriter<rooms_service::WatchRoomsStreamingResponse>* writer)
{
	auto client_id = request->client_id();
	
	// Registrazione del client
	{
    std::lock_guard lock(m_mutex_subscribers);
    m_subscribers.insert(std::make_pair(client_id, writer));
    std::println("Client {} subscribed to WatchRoomsStreaming.", client_id);
  }
  
  // Invio lo stato iniziale
  {
  	std::shared_lock<std::shared_mutex> lock(m_rooms_mutex);
    for (const auto& [id, info] : m_room_users) 
    {
      auto initial_msg = rooms_service::WatchRoomsStreamingResponse{};
      initial_msg.set_type(rooms_service::ROOM_EVENT_TYPE_ROOM_CREATED);
      
      auto proto_room = initial_msg.mutable_room();
      proto_room->set_room_id(info.room_id);
      proto_room->set_creator_id(info.creator_id);
      proto_room->set_room_name(info.room_name.data());
      proto_room->set_user_count(static_cast<uint32_t>(info.client_set.size()));

      if (!writer->Write(initial_msg)) 
       	return grpc::Status::CANCELLED;
    }
  }
  
  // Attesa passiva
  while (!context->IsCancelled()) 
  {
  	// Aspettiamo semplicemente che il canale venga chiuso dal client
  	std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  
  // Pulizia
  {
    std::lock_guard lock(m_mutex_subscribers);
    m_subscribers.erase(client_id);
  }

  std::println("Client {} unsubscribed.", client_id);
  return grpc::Status::OK;
}

		
bool RoomsServiceImpl::IsClientInRoom(ClientID client_id, RoomID room_id)
{
	std::shared_lock lock(m_rooms_mutex);
  auto it = m_user_rooms.find(client_id);
  if (it == m_user_rooms.end())
    return false;
  return it->second.contains(room_id);
}

void RoomsServiceImpl::GetRoomClients(RoomID room_id, std::set<ClientID>& out_clients)
{
	out_clients = {};
	
	std::shared_lock lock(m_rooms_mutex);
  auto it = m_room_users.find(room_id);
  if (it != m_room_users.end())
  	out_clients = it->second.client_set;
}


// ==================================
// Private methods 
// ==================================

ServerRoomInfo RoomsServiceImpl::create_empty_server_room_info(RoomID room_id, ClientID creator, std::string_view room_name)
{
	auto room = ServerRoomInfo{};
  room.room_id = room_id;
  room.creator_id = creator;
  room.client_set = {};
  room.room_name.fill(0);
  auto length_to_copy = std::min(room_name.size(), static_cast<size_t>(max_len_room_name - 1));
  std::copy_n(room_name.begin(), length_to_copy, room.room_name.begin());
  return room; 
}

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

void RoomsServiceImpl::broadcast_message(const rooms_service::WatchRoomsStreamingResponse& msg)
{
	// Nota: m_mutex_subscribers deve essere già acquisito da chi chiama questa funzione
  for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ) 
  {
    if (!it->second->Write(msg)) 
    {
      std::println("Client {} disconnected, removing from subscribers.", it->first);
      it = m_subscribers.erase(it); // Rimuove il sottoscrittore non più valido
    } 
    else 
    {
    	++it;
    }
  }
}