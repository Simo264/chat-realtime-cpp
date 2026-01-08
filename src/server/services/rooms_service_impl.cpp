#include "rooms_service_impl.hpp"

#include <fstream>
#include <grpcpp/server_context.h>

#include <grpcpp/support/status.h>
#include <print>
#include <string>
#include <thread>
#include <chrono>

#include <csv.h>

// ==================================
// Public methods 
// ==================================
 
#if 0
grpc::Status RoomsServiceImpl::WatchRoomsProcedure(grpc::ServerContext* context,
																									const rooms_service::WatchRoomsRequest* request,
																									grpc::ServerWriter<rooms_service::RoomList>* writer)
{
	auto client_id = static_cast<ClientID>(request->client_id());
	std::println("[WatchRooms] client_id={}", client_id);
	
	uint32_t counter = 0;
	while (!context->IsCancelled())
  {
    auto rooms = rooms_service::RoomList{};

    // Room 1
    auto room1 = rooms.add_rooms();
    room1->set_room_id(1);
    room1->set_room_name("General");
    room1->set_user_count(3 + (counter % 2));

    // Room 2
    auto room2 = rooms.add_rooms();
    room2->set_room_id(2);
    room2->set_room_name("Gaming");
    room2->set_user_count(5);

    // Room 3 (dinamica)
    if (counter % 3 == 0)
    {
      auto room3 = rooms.add_rooms();
      room3->set_room_id(3);
      room3->set_room_name("Random");
      room3->set_user_count(1);
    }

    writer->Write(rooms);

    counter++;
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
  
	std::println("[WatchRooms] client {} disconnected", client_id);
	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::ListAllRoomsProcedure(grpc::ServerContext* context,
																										const rooms_service::ListRoomsRequest* request,
																										rooms_service::RoomList* response)
{
	// Questa è una sezione critica: lettura condivisa.
	// shared_lock permette letture parallele ma si blocca se c'è qualcuno che sta scrivendo
	std::shared_lock lock(m_db_rooms_mutex);
	auto reader = io::CSVReader<3>(db_rooms); // 3 -> creator_id, room_name, room_id
	reader.read_header(io::ignore_extra_column, "creator_id", "room_name", "room_id");

	auto field_creator_id = std::string{};
	auto field_room_name = std::string{};
	auto field_room_id = std::string{};
	field_room_name.reserve(max_len_room_name);
	field_creator_id.reserve(8);
	field_room_id.reserve(8);
  while(reader.read_row(field_creator_id, field_room_name, field_room_id))
  {
  	// auto room_info = response->add_rooms();
   	// room_info->set_room_id(std::stoul(tmp_room_id));
    // room_info->set_room_name(tmp_room_name);
    // room_info->set_user_count(0);
  }
	return grpc::Status::OK;
}
#endif


grpc::Status RoomsServiceImpl::CreateRoomProcedure(grpc::ServerContext* context,
																									const rooms_service::CreateRoomRequest* request,
																									rooms_service::CreateRoomResponse* response)
{
	auto in_room_name = std::string_view{ request->room_name() };
	auto in_creator_id = request->creator_id();
	
	auto error_message = std::array<char, max_len_error_message>{};
	if(!this->validate_room_name(in_room_name, error_message))
	{
		std::println("[CreateRoomProcedure] INVALID_ARGUMENT: {}", error_message.data());
		return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error_message.data());
	}
	
	// sezione critica: scrittura esclusiva! Nessuno può leggere o scrivere finché non abbiamo finito
	{
		std::unique_lock lock(m_db_rooms_mutex);
		
		if(!this->check_duplicate(in_room_name))
		{
			std::println("[CreateRoomProcedure] ALREADY_EXISTS: a room with this name already exists.");
			return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "A room with this name already exists.");
		}
		
		if(m_next_room_id == invalid_room_id)
	    m_next_room_id = this->get_next_room_id();
		
		auto current_room_id = m_next_room_id.fetch_add(1);	
		this->create_room(current_room_id, in_creator_id, in_room_name);
		
		auto room_info = response->mutable_room();
	  room_info->set_room_id(current_room_id);
	  room_info->set_creator_id(in_creator_id);
	  room_info->set_room_name(std::string(in_room_name));
	  room_info->set_user_count(0);
		std::println("[CreateRoomProcedure] Room '{}' created with id {} by user {}", in_room_name, current_room_id, in_creator_id);
	}
	return grpc::Status::OK;
}

// ==================================
// Private methods 
// ==================================

bool RoomsServiceImpl::validate_room_name(std::string_view room_name,
																					std::array<char, max_len_error_message>& error_message) const
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
	auto reader = io::CSVReader<3>(db_rooms); // 3 -> creator_id, room_name, room_id
	reader.read_header(io::ignore_extra_column, "creator_id", "room_name", "room_id");

	auto field_creator_id = std::string{};
	auto field_room_name = std::string{};
	auto field_room_id = std::string{};
	field_room_name.reserve(max_len_room_name);
	field_creator_id.reserve(8);
	field_room_id.reserve(8);
	while(reader.read_row(field_creator_id, field_room_name, field_room_id))
	{
		if(field_room_name == room_name)
			return false;
	}	
	return true;
}

RoomID RoomsServiceImpl::get_next_room_id()
{
	auto reader = io::CSVReader<3>(db_rooms); // 3 -> creator_id, room_name, room_id
	reader.read_header(io::ignore_extra_column, "creator_id", "room_name", "room_id");

	auto field_creator_id = std::string{};
	auto field_room_name = std::string{};
	auto field_room_id = std::string{};
	field_room_name.reserve(max_len_room_name);
	field_creator_id.reserve(8);
	field_room_id.reserve(8);
	
	auto max_id = RoomID{ 0 };
  auto has_records = false;
	while(reader.read_row(field_creator_id, field_room_name, field_room_id))
	{
 		auto current_id = static_cast<RoomID>(std::stoull(field_room_id));
   	if (current_id > max_id)
      max_id = current_id;
    
    has_records = true;
	}	

 if(!has_records)
  	return 0;
  
 return max_id + 1;
}

void RoomsServiceImpl::create_room(RoomID room_id, ClientID creator_id, std::string_view room_name)
{
	std::ofstream os(db_rooms, std::ios_base::app);
	if(!os)
	{
		std::println("Error on opening file {}", db_users.string());
		exit(EXIT_FAILURE);
	}
	
	os << static_cast<int>(room_id) << "," << static_cast<int>(creator_id) << "," << room_name.data() << "\n";
}
