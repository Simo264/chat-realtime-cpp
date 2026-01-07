#pragma once

#include <vector>
#include <mutex>

#include <rooms_service.grpc.pb.h>
#include <rooms_service.pb.h>
#include <grpcpp/support/status.h>

#include "../../common.hpp"

class RoomsServiceImpl : public rooms_service::RoomsServiceInterface::Service
{
	public:
		grpc::Status WatchRooms(grpc::ServerContext* context,
														const rooms_service::WatchRoomsRequest* request,
														grpc::ServerWriter<rooms_service::RoomList>* writer) override;
		grpc::Status ListAllRooms(grpc::ServerContext* context,
															const rooms_service::ListRoomsRequest* request,
															rooms_service::RoomList* response) override;
	private:
		std::mutex m_db_rooms_mutex;
};