#include "rooms_service_connector.hpp"

#include <mutex>
#include <print>
#include <thread>

void RoomsServiceConnector::Subscribe(ClientID client_id)
{
	m_thread_running = true;
	m_thread = std::thread([this, client_id](){
		auto context = grpc::ClientContext{};
		auto request = rooms_service::RoomsSubscriptionRequest{};
		request.set_client_id(client_id);
		
		auto reader = m_stub->SubscribeMyRooms(&context, request);
		auto snapshot = rooms_service::RoomsSnapshot{};
		while(m_thread_running && reader->Read(&snapshot))
		{
			auto& snapshot_rooms = snapshot.rooms();
			std::lock_guard<std::mutex> guard(m_rooms_mutex);
			m_rooms.clear();
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
				m_rooms.emplace_back(room_info);
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