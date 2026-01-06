#pragma once

#include <vector>
#include <mutex>

#include <rooms_service.grpc.pb.h>
#include <rooms_service.pb.h>
#include <grpcpp/support/status.h>

#include "../../common.hpp"

using rooms_service::RoomsServiceInterface;
using rooms_service::RoomsSubscriptionRequest;
using rooms_service::RoomsSnapshot;

class RoomsServiceImpl : public RoomsServiceInterface::Service
{
	public:
		grpc::Status SubscribeMyRooms(grpc::ServerContext* context,
																	const RoomsSubscriptionRequest* request,
																	grpc::ServerWriter<RoomsSnapshot>* writer) override;
	private:
};