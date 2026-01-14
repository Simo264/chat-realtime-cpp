#include "globals.hpp"
#include <array>
#include <vector>

ClientID g_client_id = ClientID{ invalid_client_id };
std::array<char, max_len_username> g_client_username = std::array<char, max_len_username>{};
std::atomic<RoomID> g_current_room_id{ invalid_client_id };

std::vector<ClientRoomInfo> g_all_room_vector = std::vector<ClientRoomInfo>{};
std::shared_mutex g_mutex_all_room_vector = std::shared_mutex{};

std::set<RoomID> g_joined_room_vector = std::set<RoomID>{};
std::shared_mutex g_mutex_joined_room_vector = std::shared_mutex{};

std::map<RoomID, std::vector<ChatMessage>> g_chat_messages;
std::shared_mutex g_mutex_chat_messages;

// Insieme degli utenti presenti nella stanza corrente
std::set<ClientID> g_room_users;
std::shared_mutex g_mutex_room_users;