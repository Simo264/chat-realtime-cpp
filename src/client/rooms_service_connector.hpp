#pragma once

#include <memory>
#include <mutex>
#include <string_view>
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
	
		// Avvia il monitoraggio in tempo reale delle stanze dell'utente.
		// Stabilisce uno stream gRPC (server-side streaming) e lancia un nuovo thread worker
		// per processare gli aggiornamenti asincroni dal server.
		// Note: da chiamare esclusivamente una volta dopo l'avvenuta autenticazione.
		// void CallRemoteWatchRoomsProcedure(ClientID client_id);
		
		// Il metodo Stop() deve essere chiamato prima della chiusura dell'applicazione client
		// per terminare correttamente il thread di ascolto e chiudere lo stream gRPC.
		// void Stop();
		
		// Ritorna la lista di tutte le stanze esistenti
		// void CallRemoteGetAllRoomsProcedure(std::vector<RoomInfo>& out_vector) const;
		
		// Ritorna la lista delle stanze in cui un utente fa parte 
		// const auto& GetJoinedRoomVector() const { return m_joined_room_vector; }
	
	
		// Esegue la chiamata RPC remota per la creazione di una nuova stanza.
		// @param client_id L'ID del client che richiede la creazione (diventerà il creator_id).
		// @param room_name  Il nome della stanza da creare.
		// @param error_message Buffer in cui verrà scritto il messaggio d'errore in caso di fallimento.
		void CallRemoteCreateRoomProcedure(ClientID client_id, 
																			std::string_view room_name,
																			std::array<char, max_len_error_message>& error_message);
		
		// Esegue la chiamata RPC remota per eliminare (soft-delete) una stanza esistente.
		// L'operazione andrà a buon fine solo se il client_id fornito corrisponde al creator_id 
		// registrato nel database.
		// @param room_id L'ID univoco della stanza da rimuovere.
		// @param client_id L'ID del client che richiede l'eliminazione (usato per il controllo permessi).
		// @param error_message Buffer che ospiterà la descrizione dell'errore in caso di fallimento
		void CallRemoteDeleteRoomProcedure(RoomID room_id,
																			ClientID client_id, 
																			std::array<char, max_len_error_message>& error_message);
	
		void CallRemoteListRoomsProcedure(std::vector<RoomInfo>& out_vector,
																			std::array<char, max_len_error_message>& error_message);
		
	private:
		std::shared_ptr<RoomsServiceInterface::Stub> m_stub;
		
		std::thread m_thread;
		bool m_thread_running{ false };
		
		std::vector<RoomInfo> m_joined_room_vector;
};