#include "rooms_service_impl.hpp"

#include <grpcpp/server_context.h>

grpc::Status RoomsServiceImpl::SubscribeMyRooms(grpc::ServerContext* context,
																								const RoomsSubscriptionRequest* request,
																								grpc::ServerWriter<RoomsSnapshot>* writer)
{
	// 1. leggere client_id dalla request
	auto client_id = static_cast<ClientID>(request->client_id()); 
	 
	// 2. selezionare solo le stanze in cui client_id è presente: il client non deve ricevere stanze non sue
	// 3. inviare lo snapshot iniziale
	// 4. registrare il client come subscriber
	// 5. rimanere bloccato
	// 6. cleanup
	
	return grpc::Status::OK;
}