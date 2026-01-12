#include "../common.hpp"

#include <array>
#include <vector>
#include <shared_mutex>

extern ClientID g_client_id;
extern std::array<char, max_len_username> g_client_username;

extern std::vector<RoomInfo> g_all_room_vector;
extern std::shared_mutex g_mutex_all_room_vector;

extern std::vector<RoomInfo> g_joined_room_vector;