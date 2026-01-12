#pragma once

#include <array>
#include <atomic>
#include <string_view>
#include <shared_mutex>

#include <rooms_service.grpc.pb.h>
#include <rooms_service.pb.h>
#include <grpcpp/support/status.h>

#include "../../common.hpp"

class RoomsServiceImpl : public rooms_service::RoomsService::Service
{
	public:
		RoomsServiceImpl();
		
		grpc::Status CreateRoomProcedure(grpc::ServerContext* context,
																		const rooms_service::CreateRoomProcedureRequest* request,
																		rooms_service::CreateRoomProcedureResponse* response) override;
		
		grpc::Status DeleteRoomProcedure(grpc::ServerContext* context,
																		const rooms_service::DeleteRoomProcedureRequest* request,
																		rooms_service::DeleteRoomProcedureResponse* response) override;	
	
		grpc::Status ListRoomsProcedure(grpc::ServerContext* context, 
																		const rooms_service::ListRoomsProcedureRequest* request, 
																		rooms_service::ListRoomsProcedureResponse* response) override;	
	
		grpc::Status JoinRoomProcedure(grpc::ServerContext* context,
    															const rooms_service::JoinRoomProcedureRequest* request,
                   								rooms_service::JoinRoomProcedureResponse* response) override;
	
		grpc::Status LeaveRoomProcedure(grpc::ServerContext* context,
    															const rooms_service::LeaveRoomProcedureRequest* request,
                   								rooms_service::LeaveRoomProcedureResponse* response) override;
	
		grpc::Status ListRoomUsersProcedure(grpc::ServerContext* context, 
                                        const rooms_service::ListRoomUsersProcedureRequest* request, 
                                        rooms_service::ListRoomUsersProcedureResponse* response) override;
		
		grpc::Status WatchRoomsStreaming(grpc::ServerContext* context, 
																		const rooms_service::WatchRoomsStreamingRequest* request, 
																		grpc::ServerWriter<rooms_service::WatchRoomsStreamingResponse>* writer) override;
		
	private:
		bool validate_room_name(std::string_view room_name, std::array<char, max_len_error_message>& error_message) const;
		bool check_duplicate(std::string_view room_name);
		void insert_new_room_db(RoomID room_id, ClientID creator_id, std::string_view room_name);
		void mark_room_as_deleted_db(RoomID room_id);
		
		std::shared_mutex m_rooms_mutex; 
		std::atomic<RoomID> m_next_room_id;
		// mi salvo la relazione stanza-utenti: in una stanza quanti (e quali) utenti ci sono
		std::map<RoomID, RoomInfo> m_room_users;
		// mi salvo la relazione utente-stanze: l'insieme delle stanze in cui un utente è iscritto
		std::map<ClientID, std::set<RoomID>> m_user_rooms;
		// Mi salvo i client ai loro "writer" per inviare notifiche
		std::map<ClientID, grpc::ServerWriter<rooms_service::WatchRoomsStreamingResponse>*> m_subscribers;
		std::mutex m_mutex_subscribers;
};