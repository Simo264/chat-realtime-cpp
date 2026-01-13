#pragma once

#include <memory>
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

class RoomsServiceConnector
{
	public:
		RoomsServiceConnector(std::shared_ptr<grpc::Channel> channel) 
			: m_stub{ rooms_service::RoomsService::NewStub(channel) } {}
	
		// Esegue la chiamata RPC remota per la creazione di una nuova stanza.
		// @return Un valore booleano che rappresenta il successo della chiamata
		// @param client_id L'ID del client che richiede la creazione (diventerà il creator_id).
		// @param room_name  Il nome della stanza da creare.
		// @param error_message Buffer in cui verrà scritto il messaggio d'errore in caso di fallimento.
		[[nodiscard]] bool CallRemoteCreateRoomProcedure(ClientID client_id, 
																										std::string_view room_name,
																										std::array<char, max_len_error_message>& error_message);
		
		// Esegue la chiamata RPC remota per eliminare (soft-delete) una stanza esistente.
		// L'operazione andrà a buon fine solo se il client_id fornito corrisponde al creator_id 
		// registrato nel database.
		// @return Un valore booleano che rappresenta il successo della chiamata
		// @param room_id L'ID univoco della stanza da rimuovere.
		// @param client_id L'ID del client che richiede l'eliminazione (usato per il controllo permessi).
		// @param error_message Buffer che ospiterà la descrizione dell'errore in caso di fallimento
		[[nodiscard]] bool CallRemoteDeleteRoomProcedure(RoomID room_id,
																										ClientID client_id, 
																										std::array<char, max_len_error_message>& error_message);
		
		// Esegue la chiamata RPC remota per recuperare la lista completa delle stanze disponibili dal server.
		// @return Un valore booleano che rappresenta il successo della chiamata
		// @param out_vector Riferimento a un vettore che verrà svuotato e riempito con le informazioni delle stanze ricevute.
		// @param error_message Buffer in cui verrà scritto il messaggio d'errore in caso di problemi di connessione o fallimento lato server.
		[[nodiscard]] bool CallRemoteListRoomsProcedure(std::vector<ClientRoomInfo>& out_vector,
																										std::array<char, max_len_error_message>& error_message);
		
		// Esegue la chiamata RPC remota per entrare nella stanza.
		// @return Un valore booleano che rappresenta il successo della chiamata
		// @param room_id L'ID della stanza in cui entrare.
		// @param client_id L'ID del client che richiede di entrare.
		// @param error_message Buffer in cui verrà scritto il messaggio d'errore in caso di problemi di connessione o fallimento lato server.
		[[nodiscard]] bool CallRemoteJoinRoomProcedure(RoomID room_id, 
																									ClientID client_id, 
																									std::array<char, max_len_error_message>& error_message);
	
		// Esegue la chiamata RPC remota per lasciare una stanza.
		// @return Un valore booleano che rappresenta il successo della chiamata
		// @param room_id L'ID della stanza da cui uscire.
		// @param client_id L'ID del client che richiede di uscire.
		// @param error_message Buffer in cui verrà scritto il messaggio d'errore in caso di problemi di connessione o fallimento lato server.
		[[nodiscard]] bool CallRemoteLeaveRoomProcedure(RoomID room_id, 
																										ClientID client_id, 
																										std::array<char, max_len_error_message>& error_message);
		
		// Inizia una sottoscrizione Server-Side Streaming per ricevere notifiche in tempo reale.
		// Questo metodo apre un canale di comunicazione persistente con il server gRPC.
		// A differenza delle chiamate unarie, non restituisce un valore immediato, ma avvia
		// un thread di background che rimane in ascolto di eventi riguardanti le stanze.
		// Il thread ricevente aggiorna in modo asincrono i dati,
		// garantendo che l'interfaccia ImGui rifletta sempre lo stato globale del server.
		// @param client_id L'ID del client che richiede la sottoscrizione.
		// @note Questo metodo deve essere chiamato una sola volta all'avvio dell'applicazione.
		void StartWatchRoomsStream(ClientID client_id);
		
	private:
		std::shared_ptr<rooms_service::RoomsService::Stub> m_stub;
		std::thread m_thread_streaming;
		
		ClientRoomInfo create_empty_room_info(RoomID room_id, ClientID creator, std::string_view room_name);
		
		void on_room_event_type_room_create(rooms_service::WatchRoomsStreamingResponse& response);
		void on_room_event_type_room_deleted(rooms_service::WatchRoomsStreamingResponse& response);
		void on_room_event_type_user_joined(rooms_service::WatchRoomsStreamingResponse& response);
		void on_room_event_type_user_left(rooms_service::WatchRoomsStreamingResponse& response);
};