#include "globals.hpp"

ClientID g_client_id = ClientID{ invalid_client_id };
std::array<char, max_len_username> g_client_username = std::array<char, max_len_username>{};

std::vector<ClientRoomInfo> g_all_room_vector = std::vector<ClientRoomInfo>{};
std::shared_mutex g_mutex_all_room_vector = std::shared_mutex{};

std::set<RoomID> g_joined_room_vector = std::set<RoomID>{};
std::shared_mutex g_mutex_joined_room_vector = std::shared_mutex{};