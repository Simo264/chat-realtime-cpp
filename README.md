# Chat Room Distribuita

## Descrizione del Progetto

Applicazione di chat distribuita client-server sviluppata in C++ che implementa
un sistema di comunicazione in tempo reale ispirato a Discord.
Il progetto utilizza tecnologie a oggetti distribuiti (gRPC) per la comunicazione 
tra client e server, con un'interfaccia grafica realizzata tramite ImGui.

## Architettura

### Server

Il server centrale gestisce:

- Autenticazione degli utenti
- Creazione e gestione delle stanze di chat
- Routing dei messaggi tra i client
- Notifiche in tempo reale (utenti che entrano/escono)
- Lista degli utenti connessi per ogni stanza

### Client

Ogni client fornisce:

- Interfaccia grafica intuitiva tramite ImGui
- Connessione al server tramite gRPC
- Ricezione messaggi in tempo reale via streaming
- Gestione multi-stanza
- Visualizzazione lista utenti online

## Funzionalità Principali

- **Login/Logout**: Accesso con username univoco
- **Gestione Stanze**: Visualizzazione, creazione e accesso alle stanze disponibili
- **Chat in Tempo Reale**: Invio e ricezione messaggi istantanei
- **Notifiche**: Avvisi quando utenti entrano o escono dalle stanze
- **Multi-Stanza**: Possibilità di passare tra diverse stanze di chat


## Introduzione su gRPC, Protocol Buffer e servizi

Mi sono basato sulla documentazione ufficiale: <https://grpc.io/docs/what-is-grpc/introduction/>

### Che cos'è gRPC?

In gRPC, un'applicazione client può chiamare direttamente un metodo su
un'applicazione server che si trova su una macchina diversa, come se fosse
un oggetto locale.
gRPC si basa sull'idea di definire un servizio, specificando i metodi che
possono essere chiamati remotamente con i relativi parametri e tipi di ritorno.
Lato **server**, il server implementa questa interfaccia ed esegue un server gRPC
per gestire le chiamate dei client.
Lato **client**, il client dispone di uno stub che fornisce gli stessi metodi
esposti dal server.

### Cosa sono i Protocol Buffers?

gRPC utilizza i Protocol Buffers, il meccanismo open source maturo di Google
per la serializzazione di dati strutturati.
Il primo passo quando si lavora con i protocol buffers è definire la struttura
dei dati che si desidera serializzare in un file proto:
si tratta di un normale file di testo con estensione "**.proto**".
I dati dei protocol buffer sono strutturati come messaggi, dove ogni messaggio
rappresenta un piccolo record logico di informazioni contenente una serie
di coppie nome-valore chiamate campi. Ecco un semplice esempio:

```code
message Person {
  int32 id = 1;
  string name = 2;
  string email = 3;
}
```

Una volta specificate le strutture dati, si utilizza il compilatore dei
protocol buffer (protoc) per generare le classi di accesso ai dati.
Queste forniscono semplici metodi di accesso per ogni campo, come name() e
set_name(), oltre a metodi per serializzare e deserializzare l'intera
struttura da/a byte grezzi.
Eseguendo il compilatore sull'esempio sopra verrà generata una classe chiamata
Person. È quindi possibile utilizzare questa classe nell'applicazione per
popolare, serializzare e recuperare messaggi protocol buffer di tipo Person.

### Come di definiscono i servizi?

I servizi gRPC vengono definiti in normali file proto, con i parametri dei
metodi RPC e i tipi di ritorno specificati come messaggi protocol buffer,
ad esempio:

```code
message HelloRequest {
  string name = 1;
}
message HelloResponse {
  string message = 1;
}
service GreetService {
  rpc SayHello (HelloRequest) returns (HelloResponse);
}
```

gRPC consente di definire quattro tipi di metodi di servizio:

- RPC unarie: il client invia una singola richiesta al server e riceve una
  singola risposta, proprio come una normale chiamata di funzione.

	```code
  rpc SayHello(HelloRequest) returns (HelloResponse);
  ```

- RPC con streaming dal server: il client invia una singola richiesta al
  server e riceve un flusso di messaggi come risposta.

   ```code
  rpc SayHello(HelloRequest) returns (stream HelloResponse);
   ```

- RPC con streaming dal client: il client invia un flusso di messaggi al
  server e riceve una singola risposta.

	```code
  rpc SayHello(stream HelloRequest) returns (HelloResponse);
  ```

- RPC bidirezionali con streaming: sia il client che il server possono
  inviare flussi di messaggi l'uno all'altro.  

  ```code
  rpc SayHello(stream HelloRequest) returns (stream HelloResponse);
  ```

## Compilazione ed Esecuzione

> **Attenzione:** il progetto è stato sviluppato, eseguito e testato in ambiente Linux, sulla distribuzione Fedora 43. Il progetto dovrebbe essere indipendente dalla piattaforma.

Usa CMake nella directory root per generare i file necessari alla compilazione
e generare la directory build/:

```bash
cmake -S . -B build/
```

Compila i sorgenti utilizzando GNU Make:

```bash
make -C ./build
```

Una volta completata la compilazione, i file eseguibili si troveranno
all'interno della cartella build/.
Per avviare il server bisogna andare nella directory root ed eseguire:

```bash
./build/chat_server
```

Per avviare il client bisogna andare nella directory root ed eseguire:

```bash
./build/chat_client
```
