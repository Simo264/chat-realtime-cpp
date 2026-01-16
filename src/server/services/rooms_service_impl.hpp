#pragma once

#include <array>
#include <atomic>
#include <shared_mutex>
#include <string_view>

#include <grpcpp/support/status.h>
#include <rooms_service.grpc.pb.h>
#include <rooms_service.pb.h>

#include "../../common.hpp"

/**
 * @class RoomsServiceImpl
 * @brief Implementazione server per la gestione delle stanze.
 *
 * @details
 * Implementa le procedure RPC definite in `rooms_service::RoomsService`:
 * - `CreateRoomProcedure` per creare nuove stanze;
 * - `DeleteRoomProcedure` per marcare stanze come cancellate (soft-delete);
 * - `JoinRoomProcedure` / `LeaveRoomProcedure` per gestire ingressi e uscite;
 * - `WatchRoomsStreaming` e `WatchRoomUsersStreaming` per fornire aggiornamenti
 *   in tempo reale tramite streaming verso i client sottoscritti.
 *
 * La classe mantiene strutture dati in memoria per mappare stanze, utenti e
 * sottoscrittori; l'accesso concorrente è protetto tramite `std::shared_mutex`
 * e gli identificativi delle stanze sono generati in modo atomico tramite
 * `std::atomic`.
 */
class RoomsServiceImpl : public rooms_service::RoomsService::Service
{
public:
  RoomsServiceImpl();

  [[nodiscard]] grpc::Status CreateRoomProcedure(
      grpc::ServerContext *context,
      const rooms_service::CreateRoomProcedureRequest *request,
      rooms_service::CreateRoomProcedureResponse *response) override;

  [[nodiscard]] grpc::Status DeleteRoomProcedure(
      grpc::ServerContext *context,
      const rooms_service::DeleteRoomProcedureRequest *request,
      rooms_service::DeleteRoomProcedureResponse *response) override;

  [[nodiscard]] grpc::Status JoinRoomProcedure(
      grpc::ServerContext *context,
      const rooms_service::JoinRoomProcedureRequest *request,
      rooms_service::JoinRoomProcedureResponse *response) override;

  [[nodiscard]] grpc::Status LeaveRoomProcedure(
      grpc::ServerContext *context,
      const rooms_service::LeaveRoomProcedureRequest *request,
      rooms_service::LeaveRoomProcedureResponse *response) override;

  [[nodiscard]] grpc::Status WatchRoomsStreaming(
      grpc::ServerContext *context,
      const rooms_service::WatchRoomsStreamingRequest *request,
      grpc::ServerWriter<rooms_service::WatchRoomsStreamingResponse> *writer)
      override;

  [[nodiscard]] grpc::Status WatchRoomUsersStreaming(
      grpc::ServerContext *context,
      const rooms_service::WatchRoomUsersStreamingRequest *request,
      grpc::ServerWriter<rooms_service::WatchRoomUsersStreamingResponse>
          *writer) override;

  bool IsClientInRoom(ClientID client_id, RoomID room_id);
  void GetRoomClients(RoomID room_id, std::set<ClientID> &out_clients);

  // Notifica tutti i client presenti nella stanza che un nuovo utente è entrato
  // nella stanza
  void NotifyUserJoined(RoomID room_id, ClientID user_id);
  // Notifica tutti i client presenti nella stanza che un utente ha lasciato la
  // stanza
  void NotifyUserLeft(RoomID room_id, ClientID user_id);

private:
  ServerRoomInfo create_empty_server_room_info(RoomID room_id, ClientID creator,
                                               std::string_view room_name);
  bool validate_room_name(
      std::string_view room_name,
      std::array<char, max_len_error_message> &error_message) const;
  bool check_duplicate(std::string_view room_name);
  void insert_new_room_db(RoomID room_id, ClientID creator_id,
                          std::string_view room_name);
  void mark_room_as_deleted_db(RoomID room_id);
  void broadcast_message(const rooms_service::WatchRoomsStreamingResponse &msg);

  std::shared_mutex m_rooms_mutex;
  std::atomic<RoomID> m_next_room_id;
  // mi salvo la relazione stanza-utenti: in una stanza quanti (e quali) utenti
  // ci sono
  std::map<RoomID, ServerRoomInfo> m_room_users;
  // mi salvo la relazione utente-stanze: l'insieme delle stanze in cui un
  // utente è iscritto
  std::map<ClientID, std::set<RoomID>> m_user_rooms;
  // Mi salvo i client ai loro "writer" per inviare notifiche
  std::map<ClientID,
           grpc::ServerWriter<rooms_service::WatchRoomsStreamingResponse> *>
      m_subscribers;
  std::shared_mutex m_mutex_subscribers;
  // Mi salvo il mapping stanza-insieme di writer (client) che osservano gli
  // utenti della stanza
  std::map<RoomID, std::set<grpc::ServerWriter<
                       rooms_service::WatchRoomUsersStreamingResponse> *>>
      m_room_user_watchers;
  std::shared_mutex m_mutex_user_watchers;
};
