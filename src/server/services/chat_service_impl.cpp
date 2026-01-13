#include "chat_service_impl.hpp"
#include <grpcpp/support/status.h>
#include <set>
#include <string_view>

#include "../../common.hpp"
#include "chat_service.pb.h"

grpc::Status ChatServiceImpl::ChatStream(grpc::ServerContext* context, 
						    												grpc::ServerReaderWriter<chat_service::ChatStreamResponse, 
						                						chat_service::ChatStreamRequest>* stream)
{
 	auto request = chat_service::ChatStreamRequest{};
  auto client_id = invalid_client_id;
  while (stream->Read(&request))
  {
 		auto message_type = static_cast<int>(request.type());
   	auto sender_id = request.sender_id();
   	if (message_type == chat_service::CHAT_MESSAGE_TYPE_REGISTER) 
    {
      client_id = sender_id;

     	// sezione critica: registro il client
      std::lock_guard lock(m_mutex_client_streams);
      m_client_streams[client_id] = stream;
      continue;
    }
    
    if (message_type != chat_service::CHAT_MESSAGE_TYPE_TEXT)
    	continue;
   
  	auto room_id = request.room_id();
    auto sender_name = std::string_view{ request.sender_name() };
    auto content = std::string_view{ request.content() };
    
    // se l'utente non è nella stanza non gli inoltro il messaggio
    if (!m_rooms_service.IsClientInRoom(sender_id, room_id)) 
      continue;
    
    // prepara risposta
    auto response = chat_service::ChatStreamResponse{};
    response.set_room_id(room_id);
    response.set_sender_id(sender_id);
    response.set_sender_name(sender_name);
    response.set_content(content);
    
    // broadcast
    auto client_set = std::set<ClientID>{};
    m_rooms_service.GetRoomClients(room_id, client_set);
    std::vector<ChatStreamRW*> stream_list;
  	{
	   	std::lock_guard lock(m_mutex_client_streams);
			for (auto client : client_set) 
  		{
    		auto client_stream_it = m_client_streams.find(client);
		    if (client_stream_it != m_client_streams.end())
				  stream_list.push_back(client_stream_it->second);
		  }
   	}
    
    for (auto target : stream_list) 
      target->Write(response);
    
  } // while
  
  // cleanup disconnessione
  if(client_id != invalid_client_id)
  {
    std::lock_guard lock(m_mutex_client_streams);
    m_client_streams.erase(client_id);
  }
    
	return grpc::Status::OK;
}