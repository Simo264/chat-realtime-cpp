#include "rooms_service_impl.hpp"

#include <grpcpp/server_context.h>

#include <print>
#include <string>
#include <thread>
#include <chrono>

#include <csv.h>

grpc::Status RoomsServiceImpl::WatchRooms(grpc::ServerContext* context,
																					const rooms_service::WatchRoomsRequest* request,
																					grpc::ServerWriter<rooms_service::RoomList>* writer)
{
	auto client_id = static_cast<ClientID>(request->client_id());
	std::println("[SubscribeMyRooms] client_id={}", client_id);
	
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
  
	std::println("[SubscribeMyRooms] client {} disconnected", client_id);
	return grpc::Status::OK;
}

grpc::Status RoomsServiceImpl::ListAllRooms(grpc::ServerContext* context,
																						const rooms_service::ListRoomsRequest* request,
																						rooms_service::RoomList* response)
{
	auto reader = io::CSVReader<2>(db_rooms); // 2 -> room_name, room_id
	reader.read_header(io::ignore_extra_column, "room_name", "room_id");
	
	auto tmp_room_name = std::string{};
	auto tmp_room_id = std::string{};
	tmp_room_name.reserve(max_len_room_name);
  while(reader.read_row(tmp_room_name, tmp_room_id))
  {
  	auto room_info = response->add_rooms();
   	room_info->set_room_id(std::stoul(tmp_room_id));
    room_info->set_room_name(tmp_room_name);
    room_info->set_user_count(0);
  }
	return grpc::Status::OK;
}