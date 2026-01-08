#pragma once

#include <array>
#include <string_view>
#include <vector>
#include <mutex>
#include <shared_mutex>

#include <rooms_service.grpc.pb.h>
#include <rooms_service.pb.h>
#include <grpcpp/support/status.h>

#include "../../common.hpp"

class RoomsServiceImpl : public rooms_service::RoomsServiceInterface::Service
{
	public:
		//		grpc::Status WatchRoomsProcedure(grpc::ServerContext* context,
		//																		const rooms_service::WatchRoomsRequest* request,
		//																		grpc::ServerWriter<rooms_service::RoomList>* writer) override;
		//				
		//		grpc::Status ListAllRoomsProcedure(grpc::ServerContext* context,
		//																			const rooms_service::ListRoomsRequest* request,
		//																			rooms_service::RoomList* response) override;
		
		grpc::Status CreateRoomProcedure(grpc::ServerContext* context,
																		const rooms_service::CreateRoomRequest* request,
																		rooms_service::CreateRoomResponse* response) override;
		
	private:
		bool validate_room_name(std::string_view room_name,
														std::array<char, max_len_error_message>& error_message) const;
		
		bool check_duplicate(std::string_view room_name);
		
		void create_room(RoomID room_id, ClientID creator_id, std::string_view room_name);

		RoomID get_next_room_id();
		
		std::shared_mutex m_db_rooms_mutex;
		
		// protezione semplice per l'aggiornamento del contatore
		std::atomic<RoomID> m_next_room_id{ invalid_room_id };
};