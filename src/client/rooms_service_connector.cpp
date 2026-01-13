#include "rooms_service_connector.hpp"
#include "rooms_service.pb.h"

#include <grpcpp/support/status.h>
#include <mutex>
#include <print>
#include <shared_mutex>
#include <thread>

#include "globals.hpp"

// ========================================
// Private methods
// ========================================

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

bool RoomsServiceConnector::CallRemoteListRoomsProcedure(std::vector<ClientRoomInfo>& out_vector, 
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
			auto room = this->create_empty_room_info(proto_room.room_id(), proto_room.creator_id(), proto_room.room_name());
	    out_vector.push_back(room);
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
      const auto event_type = static_cast<int>(response.type());      
      switch (event_type) 
      {
       	case rooms_service::ROOM_EVENT_TYPE_ROOM_CREATED:
       	{
          this->on_room_event_type_room_create(response);
					break;
       	}
        case rooms_service::ROOM_EVENT_TYPE_ROOM_DELETED:
       	{
        	this->on_room_event_type_room_deleted(response);
      		break;
       	} 
        case rooms_service::ROOM_EVENT_TYPE_USER_JOINED:
      	{
       		this->on_room_event_type_user_joined(response);
        	break;
        }
        case rooms_service::ROOM_EVENT_TYPE_USER_LEFT: 
      	{
       		this->on_room_event_type_user_left(response);
       		break;
       	}
        default:
         	break;
      }
    }
    
    std::println("Disconnected.");
	});
	m_thread_streaming.detach();
}

// ========================================
// Private methods
// ========================================

ClientRoomInfo RoomsServiceConnector::create_empty_room_info(RoomID room_id, ClientID creator, std::string_view room_name)
{
	auto room = ClientRoomInfo{};
 	room.room_id = room_id;
  room.creator_id = creator;
  room.user_count = 0;
  room.room_name.fill(0);
  auto length_to_copy = std::min(room_name.size(), static_cast<size_t>(max_len_room_name - 1));
  std::copy_n(room_name.begin(), length_to_copy, room.room_name.begin());
  return room;
}

void RoomsServiceConnector::on_room_event_type_room_create(rooms_service::WatchRoomsStreamingResponse& response) 
{
	const auto& proto_room = response.room();

	std::lock_guard<std::shared_mutex> lock(g_mutex_all_room_vector);
	auto new_room = this->create_empty_room_info(proto_room.room_id(), proto_room.creator_id(), proto_room.room_name());
  g_all_room_vector.push_back(new_room);
}

void RoomsServiceConnector::on_room_event_type_room_deleted(rooms_service::WatchRoomsStreamingResponse& response) 
{
	auto room_id = response.room_id();
	
  { // Aggiorno la lista globale g_all_room_vector
    std::lock_guard lock(g_mutex_all_room_vector);
    std::erase_if(g_all_room_vector, [room_id](const ClientRoomInfo& r) { 
      return r.room_id == room_id; 
    });
  }
	std::println("Room {} was deleted by creator.", response.room_id());
}

void RoomsServiceConnector::on_room_event_type_user_joined(rooms_service::WatchRoomsStreamingResponse& response) 
{
	auto target_room_id = response.room_id();
  auto new_count = response.room().user_count();
  auto actor_id = response.actor_id(); // L'ID di chi è appena entrato
  
  { // Aggiorno la lista globale g_all_room_vector
  	std::lock_guard lock(g_mutex_all_room_vector);
    for (auto& room : g_all_room_vector) 
    {
      if (room.room_id == target_room_id) 
      {
        room.user_count = new_count; 
        break;
      }
    }
  }
  
  // Aggiornamento personale: aggiorno la lista delle mie stanze g_joined_room_vector solo se l'attore sono io
  if (actor_id == g_client_id) 
  {
    std::lock_guard lock(g_mutex_joined_room_vector);
    g_joined_room_vector.insert(target_room_id);
  }
}

void RoomsServiceConnector::on_room_event_type_user_left(rooms_service::WatchRoomsStreamingResponse& response) 
{
	auto target_room_id = response.room_id();
  auto new_count = response.room().user_count();
  auto actor_id = response.actor_id(); // L'utente che è uscito dalla stanza
    
	{ // Aggiorno la lista globale g_all_room_vector
		std::lock_guard lock(g_mutex_all_room_vector);
    for (auto& room : g_all_room_vector) 
    {
      if (room.room_id == target_room_id) 
      {
        room.user_count = new_count;
        break;
      }
    }
	}
	
	// Aggiornamento personale: aggiorno la lista delle mie stanze g_joined_room_vector solo se l'attore sono io
	if (actor_id == g_client_id)
	{
		std::lock_guard lock(g_mutex_joined_room_vector);
		auto it = std::find(g_joined_room_vector.begin(), g_joined_room_vector.end(), target_room_id);
    g_joined_room_vector.erase(it);
	}
}