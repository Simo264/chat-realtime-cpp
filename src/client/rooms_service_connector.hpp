#pragma once

#include <memory>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/support/status.h>
#include <grpcpp/client_context.h>

#include <rooms_service.grpc.pb.h>
#include <rooms_service.pb.h>

#include "../common.hpp"

using rooms_service::RoomsServiceInterface;
using rooms_service::RoomsSubscriptionRequest;
//using rooms_service::RoomInfo;
using rooms_service::RoomsSnapshot;

class RoomsServiceConnector
{
	public:
		RoomsServiceConnector(std::shared_ptr<grpc::Channel> channel) 
			: m_stub{ RoomsServiceInterface::NewStub(channel) } {}
			
   // Avvia lo stream DOPO il login
   void SubscribeMyRooms(ClientID client_id);
		
	private:
		std::shared_ptr<RoomsServiceInterface::Stub> m_stub;
};