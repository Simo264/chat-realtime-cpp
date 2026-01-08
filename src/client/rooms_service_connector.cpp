#include "rooms_service_connector.hpp"
#include "rooms_service.pb.h"

#include <cstdlib>
#include <mutex>
#include <print>
#include <thread>

#if 0
void RoomsServiceConnector::CallRemoteWatchRoomsProcedure(ClientID client_id)
{
	m_thread_running = true;
	m_thread = std::thread([this, client_id](){
		// auto context = grpc::ClientContext{};
		// auto request = rooms_service::WatchRoomsRequest{};
		// request.set_client_id(client_id);
		// 
		// auto reader = m_stub->WatchRoomsProcedure(&context, request);
		// auto room_list = rooms_service::RoomList{};
		// while(m_thread_running && reader->Read(&room_list))
		// {
		// 	auto& snapshot_rooms = room_list.rooms();
		// 	std::lock_guard<std::mutex> guard(m_rooms_mutex);
		// 	m_room_vector.clear();
		// 	for (const auto& room : snapshot_rooms)
		// 	{
		// 		auto room_info = RoomInfo{};
		// 		room_info.room_id = room.room_id(); // copy room id
		// 		room_info.user_count = room.user_count(); // copy user count
		// 		auto& name_dest = room_info.room_name;
		// 		name_dest.fill(0);
		// 		auto name_src = std::string_view{ room.room_name() };
		// 		std::copy_n(name_src.begin(), max_len_room_name, name_dest.begin()); // copy room name
		// 		
		// 		room_info.clients = {}; // copy room clients
		// 		m_room_vector.emplace_back(room_info);
		// 	}
		// }

		// grpc::Status status = reader->Finish();
		// std::println("Rooms stream ended: {}", status.ok());
	}); // end thread 
}
	
void RoomsServiceConnector::Stop()
{
	m_thread_running = false;
	if (m_thread.joinable())
	 	m_thread.join();
}

void RoomsServiceConnector::CallRemoteGetAllRoomsProcedure(std::vector<RoomInfo>& out_vector) const
{
	out_vector.clear();
	
	auto request = rooms_service::ListRoomsRequest{};
	auto response = rooms_service::RoomList{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->ListAllRoomsProcedure(&context, request, &response);
	
	if(!status.ok())
	{		
		std::println("gRPC error {}: {} - {}", static_cast<int>(status.error_code()), status.error_message(), status.error_details());
		exit(EXIT_FAILURE);	
	}
	
	out_vector.reserve(response.rooms_size());
	for(const auto& room : response.rooms())
	{
		auto room_info = RoomInfo{};
		room_info.room_id = room.room_id();
		
		auto& name_dest = room_info.room_name;
		name_dest.fill(0);
		auto name_src = std::string_view{ room.room_name() };
		std::copy_n(name_src.begin(), max_len_room_name, name_dest.begin()); // copy room name
		
		room_info.clients = {};
		room_info.user_count = 0;
		out_vector.push_back(room_info);
	}
}
#endif

void RoomsServiceConnector::CallRemoteCreateRoomProcedure(ClientID creator_id, 
																													std::string_view room_name,
																													std::array<char, max_len_error_message>& out_message)
{
	auto request = rooms_service::CreateRoomRequest{};
  request.set_creator_id(creator_id);
  request.set_room_name(std::string(room_name));
    
	auto response = rooms_service::CreateRoomResponse{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->CreateRoomProcedure(&context, request, &response);
	if (status.ok())
	{
		const auto& room = response.room();
    std::println("Room created successfully! creator_id={}, room_id={}, room_name: {}", creator_id, room.room_id(), room.room_name());
		return;
	}
	
	out_message.fill(0);
	switch (status.error_code()) 
  {
    case grpc::StatusCode::ALREADY_EXISTS:
      std::format_to(out_message.begin(), "Error: This room name is already taken.");
      break;
    case grpc::StatusCode::INVALID_ARGUMENT:
    	std::format_to(out_message.begin(), "Invalid name: {}", status.error_message());
      break;
    case grpc::StatusCode::UNAVAILABLE:
	   	std::format_to(out_message.begin(), "Server is offline.");
      break;
    default:
	    std::format_to(out_message.begin(), "Unexpected error: {}", status.error_message());
      break;
  }
  std::println("Fail on creating new room. {}", out_message.data());	
}