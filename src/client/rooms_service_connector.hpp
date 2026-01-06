#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/support/status.h>
#include <grpcpp/client_context.h>

#include <rooms_service.grpc.pb.h>
#include <rooms_service.pb.h>

#include "../common.hpp"

using rooms_service::RoomsServiceInterface;

class RoomsServiceConnector
{
	public:
		RoomsServiceConnector(std::shared_ptr<grpc::Channel> channel) 
			: m_stub{ RoomsServiceInterface::NewStub(channel) } {}
	
		// Questo metodo stabilisce una connessione gRPC server-side streaming con il servizio RoomsService sul server 
		// e avvia un thread dedicato che rimane in ascolto degli aggiornamenti relativi alle stanze a cui l'utente
		// autenticato appartiene.
		// Importante: questo metodo deve essere chiamato una sola volta subito dopo il completamento dell'autenticazione dell'utente.
		void Subscribe(ClientID client_id);
		
		// Il metodo Stop() deve essere chiamato prima della chiusura dell'applicazione client
		// per terminare correttamente il thread di ascolto e chiudere lo stream gRPC.
		void Stop();
		
		const auto& GetRoomsSnapshot() const { return m_rooms; }
			
	private:
		std::shared_ptr<RoomsServiceInterface::Stub> m_stub;
		
		std::thread m_thread;
		bool m_thread_running{ false };
		
		std::vector<RoomInfo> m_rooms;
		std::mutex m_rooms_mutex;
};