
#include <grpcpp/grpcpp.h>
#include <chat_service.grpc.pb.h>
#include <chat_service.pb.h>
#include <mutex>
#include <thread>

#include "../common.hpp"

using ClientReaderWriter = grpc::ClientReaderWriter<chat_service::ChatStreamRequest, chat_service::ChatStreamResponse>;

class ChatServiceConnector
{
	public:
		ChatServiceConnector(std::shared_ptr<grpc::Channel> channel)
      : m_stub(chat_service::ChatService::NewStub(channel)) {}
    
    ~ChatServiceConnector()
	  {
	    if (m_stream) 
	      m_stream->WritesDone();
	      
	    if (m_context) 
	      m_context->TryCancel();
	  }
      
    void StartChatStream(ClientID client_id);
    void SendMessage(RoomID room_id, ClientID sender_id, std::string_view content); 
	
	private:
		std::unique_ptr<chat_service::ChatService::Stub> m_stub;
		
		std::unique_ptr<ClientReaderWriter> m_stream;
  	std::unique_ptr<grpc::ClientContext> m_context;
  
		std::thread m_thread;
		std::mutex m_mutex_stream;
};