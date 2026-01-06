#include "rooms_service_impl.hpp"

#include <grpcpp/server_context.h>

#include <print>
#include <thread>
#include <chrono>

grpc::Status RoomsServiceImpl::SubscribeMyRooms(grpc::ServerContext* context,
																								const RoomsSubscriptionRequest* request,
																								grpc::ServerWriter<RoomsSnapshot>* writer)
{
	auto client_id = static_cast<ClientID>(request->client_id());
	std::println("[SubscribeMyRooms] client_id={}", client_id);
	
	uint32_t counter = 0;
	while (!context->IsCancelled())
  {
    auto snapshot = RoomsSnapshot{};

    // Room 1
    auto room1 = snapshot.add_rooms();
    room1->set_room_id(1);
    room1->set_room_name("General");
    room1->set_user_count(3 + (counter % 2));

    // Room 2
    auto room2 = snapshot.add_rooms();
    room2->set_room_id(2);
    room2->set_room_name("Gaming");
    room2->set_user_count(5);

    // Room 3 (dinamica)
    if (counter % 3 == 0)
    {
      auto room3 = snapshot.add_rooms();
      room3->set_room_id(3);
      room3->set_room_name("Random");
      room3->set_user_count(1);
    }

    writer->Write(snapshot);

    counter++;
    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
  
	std::println("[SubscribeMyRooms] client {} disconnected", client_id);
	return grpc::Status::OK;
}