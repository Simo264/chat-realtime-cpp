#include "rooms_service_connector.hpp"
#include "rooms_service.pb.h"

#include <grpcpp/support/status.h>
#include <print>
#include <thread>

#include "globals.hpp"

bool RoomsServiceConnector::CallRemoteCreateRoomProcedure(ClientID client_id,
																													std::string_view room_name,
																													std::array<char, max_len_error_message>& error_message)
{
	error_message.fill(0);
	
	auto request = rooms_service::CreateRoomProcedureRequest{};
  request.set_creator_id(client_id);
  request.set_room_name(std::string(room_name));
    
	auto response = rooms_service::CreateRoomProcedureResponse{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->CreateRoomProcedure(&context, request, &response);
	if (!status.ok())
	{
		auto server_error = status.error_message();
		std::copy_n(server_error.begin(), max_len_error_message - 1, error_message.begin());
	}
	return status.ok();
}

bool RoomsServiceConnector::CallRemoteDeleteRoomProcedure(RoomID room_id,
																													ClientID client_id, 
																													std::array<char, max_len_error_message>& error_message)
{
	error_message.fill(0);
	
	auto request = rooms_service::DeleteRoomProcedureRequest{};
  request.set_room_id(room_id);
  request.set_client_id(client_id);
  auto response = rooms_service::DeleteRoomProcedureResponse{};
  auto context = grpc::ClientContext{};
  auto status = m_stub->DeleteRoomProcedure(&context, request, &response);
  if (!status.ok()) 
  {
		auto server_error = status.error_message();
		std::copy_n(server_error.begin(), max_len_error_message - 1, error_message.begin());
  }
  return status.ok();
}

bool RoomsServiceConnector::CallRemoteListRoomsProcedure(std::vector<RoomInfo>& out_vector, 
																												std::array<char, max_len_error_message>& error_message)
{
 	error_message.fill(0);
	
  auto request = rooms_service::ListRoomsProcedureRequest{}; 
  auto response = rooms_service::ListRoomsProcedureResponse{};
  auto context = grpc::ClientContext{};
  auto status = m_stub->ListRoomsProcedure(&context, request, &response);
  if(status.ok())
  {
	  out_vector.clear();
	 	out_vector.reserve(response.rooms_size());
	 	for (const auto& proto_room : response.rooms())
	 	{
	  	auto info = RoomInfo{};
	   	info.room_id = proto_room.room_id();
	    info.creator_id = proto_room.creator_id();
	    info.room_name.fill(0);
	    std::copy_n(proto_room.room_name().begin(), max_len_room_name - 1, info.room_name.begin());
	    out_vector.push_back(info);
	 	} 
  }
  else 
  {
		auto server_error = status.error_message();
		std::copy_n(server_error.begin(), max_len_error_message - 1, error_message.begin());
  }
  return status.ok();
}

bool RoomsServiceConnector::CallRemoteJoinRoomProcedure(RoomID room_id, 
																												ClientID client_id, 
																												std::array<char, max_len_error_message>& error_message)
{
	error_message.fill(0);
	
	auto request = rooms_service::JoinRoomProcedureRequest{};
	request.set_room_id(room_id);
  request.set_client_id(client_id);
    
	auto response = rooms_service::JoinRoomProcedureResponse{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->JoinRoomProcedure(&context, request, &response);
 	if (!status.ok()) 
  {
  	auto server_error = status.error_message();
  	std::copy_n(server_error.begin(), max_len_error_message - 1, error_message.begin());
  }
  
	return status.ok();
}

bool RoomsServiceConnector::CallRemoteLeaveRoomProcedure(RoomID room_id, 
																												ClientID client_id, 
																												std::array<char, max_len_error_message>& error_message)
{
	error_message.fill(0);
	
	auto request = rooms_service::LeaveRoomProcedureRequest{};
	request.set_room_id(room_id);
  request.set_client_id(client_id);
    
	auto response = rooms_service::LeaveRoomProcedureResponse{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->LeaveRoomProcedure(&context, request, &response);
 	if (!status.ok()) 
  {
  	auto server_error = status.error_message();
  	std::copy_n(server_error.begin(), max_len_error_message - 1, error_message.begin());
  }
  
	return status.ok();
}

void RoomsServiceConnector::CallRemoteWatchRoomsStreaming(ClientID client_id)
{
	m_thread_streaming = std::thread([this, client_id](){
		auto context = grpc::ClientContext{};
    auto request = rooms_service::WatchRoomsStreamingRequest{};
    request.set_client_id(client_id);
    auto reader = m_stub->WatchRoomsStreaming(&context, request);
    auto response = rooms_service::WatchRoomsStreamingResponse{};
    
    // Read() è bloccante: aspetta finché il server non invia qualcosa o finché lo stream non viene chiuso
    while (reader->Read(&response))
    {
    	if(response.has_room())
     	{
	     	const auto& proto_room = response.room();
        const auto event_type = response.type();      
      
        std::unique_lock lock(g_mutex_all_room_vector);
        if(event_type == rooms_service::ROOM_EVENT_TYPE_ROOM_CREATED)
        {
        	auto new_room = RoomInfo{};
          new_room.room_id = proto_room.room_id();
          new_room.creator_id = proto_room.creator_id();
          new_room.room_name.fill(0);
          
          auto& name = proto_room.room_name();
          std::copy_n(name.begin(), max_len_room_name - 1, new_room.room_name.begin());
          g_all_room_vector.push_back(new_room);
        }
      }
  		// std::println("Event received: type={}, room_id={}, client_id={}", static_cast<int>(response.type()), response.room_id(), response.client_id());
    }
    
    std::println("Disconnected.");
	});
	m_thread_streaming.detach();
}