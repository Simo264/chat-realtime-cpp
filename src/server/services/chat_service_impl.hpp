#include <chat_service.grpc.pb.h>
#include <chat_service.pb.h>

#include "rooms_service_impl.hpp"

using ChatStreamRW = grpc::ServerReaderWriter<chat_service::ChatStreamResponse, chat_service::ChatStreamRequest>;
        
class ChatServiceImpl : public chat_service::ChatService::Service
{
	public:
		ChatServiceImpl(RoomsServiceImpl& rooms_service) : m_rooms_service(rooms_service) {}

		grpc::Status ChatStream(grpc::ServerContext* context, ChatStreamRW* stream) override;
	
	private:
		RoomsServiceImpl& m_rooms_service;
		
  	std::map<ClientID, ChatStreamRW*> m_client_streams;
   	std::mutex m_mutex_client_streams;
};