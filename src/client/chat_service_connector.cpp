#include "chat_service_connector.hpp"

#include "chat_service.pb.h"
#include "globals.hpp"
#include <print>

void ChatServiceConnector::StartChatStream(ClientID client_id)
{
 	m_context = std::make_unique<grpc::ClientContext>();
  m_stream = m_stub->ChatStream(m_context.get());
  
  auto msg_register = chat_service::ChatStreamRequest{};
  msg_register.set_type(chat_service::CHAT_MESSAGE_TYPE_REGISTER);
  msg_register.set_sender_id(client_id);
  msg_register.set_sender_name(g_client_username.data());
  m_stream->Write(msg_register);
 	
	m_thread = std::thread([this]() {
    auto response = chat_service::ChatStreamResponse{};

    while (m_stream->Read(&response)) 
    {
    	auto room_id = response.room_id();
     	auto sender_id = response.sender_id();
      auto sender_name = std::string_view{ response.sender_name() };
      auto content = std::string_view{ response.content() };
      	
      auto message = ChatMessage{};
      message.room_id = room_id;
      message.sender_id = sender_id;
      message.sender_name.fill(0);
      message.content.fill(0);
      std::copy_n(sender_name.data(), std::min(sender_name.size(), message.sender_name.size() - 1), message.sender_name.data());
      std::copy_n(content.data(), std::min(content.size(), message.content.size() - 1), message.content.data());
      
      // sezione critica
     	std::lock_guard lock(g_mutex_chat_messages);
     	g_chat_messages[room_id].push_back(message);
    }

    std::println("Chat stream closed");
	});
	m_thread.detach();
}

void ChatServiceConnector::SendMessage(RoomID room_id, ClientID sender_id, std::string_view content)
{
	std::lock_guard lock(m_mutex_stream);
	
	if(!m_stream) 
		return;
	
	auto request = chat_service::ChatStreamRequest{};
	request.set_type(chat_service::CHAT_MESSAGE_TYPE_TEXT);
	request.set_room_id(room_id);
	request.set_sender_id(sender_id);
	request.set_sender_name(g_client_username.data());
	request.set_content(std::string(content));
	m_stream->Write(request);
}