#include <chat_service.grpc.pb.h>
#include <chat_service.pb.h>

#include "rooms_service_impl.hpp"

using ChatStreamRW = grpc::ServerReaderWriter<chat_service::ChatStreamResponse,
                                              chat_service::ChatStreamRequest>;

/**
 * @class ChatServiceImpl
 * @brief Implementazione server del servizio di chat.
 *
 * @details
 * Gestisce lo stream bidirezionale `ChatStream` per l'invio e la ricezione di
 * messaggi di chat tra i client. Mantiene una mappa dei stream aperti per ogni
 * client connesso e un mutex per proteggere l'accesso concorrente a tali
 * strutture. Utilizza `RoomsServiceImpl` per interrogare lo stato delle stanze
 * e verificare la partecipazione degli utenti prima di inoltrare messaggi.
 */
class ChatServiceImpl : public chat_service::ChatService::Service
{
public:
  ChatServiceImpl(RoomsServiceImpl &rooms_service)
      : m_rooms_service(rooms_service)
  {
  }

  [[nodiscard]] grpc::Status ChatStream(grpc::ServerContext *context,
                                        ChatStreamRW *stream) override;

private:
  RoomsServiceImpl &m_rooms_service;

  std::map<ClientID, ChatStreamRW *> m_client_streams;
  std::mutex m_mutex_client_streams;
};
