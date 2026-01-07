#include "rooms_service_connector.hpp"
#include "rooms_service.pb.h"

#include <cstdlib>
#include <mutex>
#include <print>
#include <thread>

void RoomsServiceConnector::WatchRooms(ClientID client_id)
{
	m_thread_running = true;
	m_thread = std::thread([this, client_id](){
		auto context = grpc::ClientContext{};
		auto request = rooms_service::WatchRoomsRequest{};
		request.set_client_id(client_id);
		
		auto reader = m_stub->WatchRooms(&context, request);
		auto room_list = rooms_service::RoomList{};
		while(m_thread_running && reader->Read(&room_list))
		{
			auto& snapshot_rooms = room_list.rooms();
			std::lock_guard<std::mutex> guard(m_rooms_mutex);
			m_room_vector.clear();
			for (const auto& room : snapshot_rooms)
			{
				auto room_info = RoomInfo{};
				room_info.room_id = room.room_id(); // copy room id
				room_info.user_count = room.user_count(); // copy user count
				auto& name_dest = room_info.room_name;
				name_dest.fill(0);
				auto name_src = std::string_view{ room.room_name() };
				std::copy_n(name_src.begin(), max_len_room_name, name_dest.begin()); // copy room name
				
				room_info.clients = {}; // copy room clients
				m_room_vector.emplace_back(room_info);
			}
		}

		grpc::Status status = reader->Finish();
		std::println("Rooms stream ended: {}", status.ok());
	}); // end thread 
}
	
void RoomsServiceConnector::Stop()
{
	m_thread_running = false;
	if (m_thread.joinable())
	 	m_thread.join();
}

void RoomsServiceConnector::GetAllRooms(std::vector<RoomInfo>& out_vector) const
{
	out_vector.clear();
	
	auto request = rooms_service::ListRoomsRequest{};
	auto response = rooms_service::RoomList{};
	auto context = grpc::ClientContext{};
	auto status = m_stub->ListAllRooms(&context, request, &response);
	
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