#include "globals.hpp"

ClientID g_client_id = ClientID{ invalid_client_id };
std::array<char, max_len_username> g_client_username = std::array<char, max_len_username>{};

std::vector<RoomInfo> g_all_room_vector = std::vector<RoomInfo>{};
std::shared_mutex g_mutex_all_room_vector = std::shared_mutex{};

std::vector<RoomInfo> g_joined_room_vector = std::vector<RoomInfo>{};
