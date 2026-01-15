# Documentazione – applicazione di chat distribuita client–server

## Descrizione dei requisiti e delle funzionalità

L’obiettivo del progetto è di realizzare una applicazione di chat distribuita in
tempo reale, stile Discord/Slack, sviluppata in C++ e basata su un’architettura
client–server.
Il sistema consente a più utenti di comunicare all’interno di stanze di chat,
con aggiornamenti in tempo reale e un’interfaccia grafica.
La comunicazione tra client e server è implementata tramite gRPC, mentre
l’interfaccia grafica lato client è sviluppata utilizzando ImGui.

L'utente prima di utilizzare le funzionalità di messaggistica dovrà prima
autenticarsi, quindi effetturare l'accesso (login) oppure iscriversi. Se
l'autenticazione ha successo, il server associa al client un identificativo
numerico (*client_id*). Per validare/registrare gli utenti, il server dovrà
fare uso di un database.

Una volta che l'autenticazione è avvenuta con successo, l'utente può:

- creare nuove stanze ed eliminare una stanza solo se lui è il creatore di
quella stanza
- entrare/uscire da qualsiasi stanza
- visualizzare in tempo reale la lista aggiornata di tutte le stanze disponibili
- visualizzazione il numero di utenti presenti per ogni stanza
- fare parte di più stanze nello stesso momento
- visualizzazione in tempo reale la lista aggiornata degli utenti che sono
presenti all'interno della stanza
- inviare/ricevere messaggi testuali in tempo reale

> Nota: non ci sono vincoli sul numero massimo di utenti che possono entrare in una
stanza.

## Descrizione dell’architettura

Il sistema adotta un’architettura client–server basato su RPC (Remote Procedure
Call) tramite gRPC.
Il server rappresenta il punto di coordinamento globale del sistema, mentre i
client non comunicano mai direttamente tra loro, ma esclusivamente tramite il
server.

Non sono presenti comunicazioni di tipo peer-to-peer.

La definizione dei servizi, dei messaggi e dei tipi di dati avviene tramite
**Protocol Buffers** (file .proto). Il server espone tre servizi gRPC distinti, 
ognuno con responsabilità ben definite.

Il primo servizio è AuthService (definito nel file auth_service.proto) che è 
responsabile della gestione della autenticazione degli uenti.
Questo servizio espone i due metodi:

```bash
service AuthService:
rpc LoginProcedure(AuthRequest) returns (AuthResponse);
rpc SignupProcedure(AuthRequest) returns (AuthResponse);

message AuthRequest:
	string username;
	string password;

message AuthResponse:
	uint32 client_id;
```

Sono entrambi due metodi **Simple RPC**, ovvero il client invia una singola
richiesta e riceve una singola risposta dal server.
Il client invia al server un oggetto AuthRequest che contiene il proprio
username e la password. Il server risponde con un oggetto AuthResponse
che contiene un identificativo numerico (*client_id*) valido in caso di 
autenticazione riuscita, oppure un valore speciale (*invalid_client_id*) in caso
di errore.


Il secondo servizio è RoomsService (definito nel file rooms_service.proto) che è
responsabile della gestione delle stanze.
Permette ai client di creare, eliminare, entrare e uscire dalle stanze, oltre a
ricevere aggiornamenti in tempo reale sullo stato delle stanze e sugli utenti
connessi. Il servizio espone i seguenti metodi:

```bash
service RoomsService:
rpc CreateRoomProcedure(CreateRoomProcedureRequest)  returns (CreateRoomProcedureResponse);
rpc DeleteRoomProcedure(DeleteRoomProcedureRequest) returns (DeleteRoomProcedureResponse);
rpc JoinRoomProcedure(JoinRoomProcedureRequest) returns (JoinRoomProcedureResponse);
rpc LeaveRoomProcedure(LeaveRoomProcedureRequest) returns (LeaveRoomProcedureResponse);
rpc WatchRoomsStreaming(WatchRoomsStreamingRequest) returns (stream WatchRoomsStreamingResponse);
rpc WatchRoomUsersStreaming(WatchRoomUsersStreamingRequest) returns (stream WatchRoomUsersStreamingResponse);
```

I primi quattro sono metodi **Simple RPC** sincroni semplici utilizzati per operazioni
atomiche di gestione delle stanze.
Gli ultimi due metodi sono metodi **server-side streaming RPC** per consentire
aggiornamenti continui e in tempo reale senza ricorrere al polling.
In particolare *WatchRoomsStreaming* permette al client di ricevere
aggiornamenti relative alla creazione, eliminazione, numero utenti delle
stanze. Mentre il metodo *WatchRoomUsersStreaming* consente di ricevere
aggiornamenti sulla lista degli utenti presenti in una specifica stanza.


Il terzo servizio è ChatService (definito nel file chat_service.proto) che è
responsabile della gestione della comunicazione testuale e del routing dei
messaggi tra i client appartenenti alla stessa stanza.
Il servizio espone il seguente metodo:

```bash
service ChatService:
rpc ChatStream(stream ChatStreamRequest) returns (stream ChatStreamResponse);

enum ChatMessageType:
  CHAT_MESSAGE_TYPE_UNSPECIFIED = 0;
  CHAT_MESSAGE_TYPE_REGISTER = 1;
  CHAT_MESSAGE_TYPE_TEXT = 2;

message ChatStreamRequest 
	ChatMessageType type;
  uint32 room_id;
  uint32 sender_id;
  string sender_name;
  string content;

message ChatStreamResponse 
  uint32 room_id;
  uint32 sender_id;
  string sender_name;
  string content;
```

Si tratta di un metodo **bidirectional streaming RPC** che permette al client e
al server di inviare e ricevere messaggi in modo asincrono e continuo.


## Descrizione dei protocolli

Il sistema utilizza un protocollo di comunicazione client–server, basato su
gRPC (Google Remote Procedure Call). gRPC è costruito sopra HTTP/2 e utilizza
Protocol Buffers come meccanismo di serializzazione dei messaggi.
I servizi gRPC, i messaggi e i tipi di dati vengono definiti tramite file proto.
A partire da questi file, il compilatore Protocol Buffers (*protoc*)
permette di generare automaticamente il codice C++ per client e server.

Esempio di generazione del codice C++:

```bash
protoc --cpp_out=. --grpc_out=. \
  --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` auth_service.proto
```

Lo stesso processo viene applicato ai file *rooms_service.proto* e
*chat_service.proto*.

Lato server, ogni servizio gRPC viene implementato creando una classe che
eredita dalla classe Service generata automaticamente da gRPC.
Ad esempio, per il servizio di autenticazione:

```cpp
class AuthServiceImpl : public auth_service::AuthService::Service
{
public:
  grpc::Status LoginProcedure(grpc::ServerContext* context,
    													const auth_service::AuthRequest* request,
                 							auth_service::AuthResponse* response) override;

  grpc::Status SignupProcedure(grpc::ServerContext* context,
  														const auth_service::AuthRequest* request,
                							auth_service::AuthResponse* response) override;
};
```

Ogni metodo restituisce un oggetto grpc::Status che indica l’esito della
chiamata.

Il server viene inizializzato utilizzando la classe grpc::ServerBuilder.
Tutti i servizi vengono registrati presso il server prima dell’avvio.
Esempio di inizializzazione del server:

```cpp
int main()
{
  auto auth_service = AuthServiceImpl{};
  auto rooms_service = RoomsServiceImpl{};
  auto chat_service = ChatServiceImpl{};
  grpc::ServerBuilder builder;
  builder.RegisterService(&auth_service);
  builder.RegisterService(&rooms_service);
  builder.RegisterService(&chat_service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
  return 0;
}
```

Il server rimane in ascolto su un indirizzo e una porta specifici, gestendo
tutte le richieste RPC provenienti dai client.

Lato client, la comunicazione con il server avviene tramite uno stub generato
automaticamente da gRPC. Per incapsulare l’utilizzo dello stub, viene utilizzata
una classe connettore, come nel caso del servizio di autenticazione:

```cpp
class AuthServiceConnector
{
public:
  AuthServiceConnector(std::shared_ptr<grpc::Channel> channel)
    : m_stub{ auth_service::AuthService::NewStub(channel) } {}

  ClientID CallRemoteLoginProcedure(std::string_view username,
																    std::string_view password,
																    std::array<char, max_len_error_message>& out_error_message);

  ClientID CallRemoteSignupProcedure(std::string_view username,
																    std::string_view password,
																    std::array<char, max_len_error_message>& out_error_message);

private:
  std::shared_ptr<auth_service::AuthService::Stub> m_stub;
};
```

Lo stub consente di invocare i metodi RPC come se fossero funzioni locali.
Il client crea un canale di comunicazione verso il server e utilizza il
connettore per effettuare le chiamate RPC. Esempio di creazione del connettore:

```cpp
auto auth_service_connector = AuthServiceConnector{ 
																grpc::CreateChannel(SERVER_ADDRESS, grpc::InsecureChannelCredentials())};
```

Esempio di utilizzo durante la fase di autenticazione:

```cpp
static bool on_submit(AuthServiceConnector& connector, bool login_mode)
{
  auto client_id = ClientID{ invalid_client_id };
  if (login_mode)
    client_id = connector.CallRemoteLoginProcedure(s_field_username.data(), 
	    																						s_field_password.data(),
																						      s_auth_error_message);
  else
    client_id = connector.CallRemoteSignupProcedure(s_field_username.data(),
																							      s_field_password.data(),
																							      s_auth_error_message);
}
```

In questo modo il client può invocare i metodi remoti in maniera trasparente,
ricevendo le risposte dal server e integrandole direttamente nella logica
dell’applicazione.
