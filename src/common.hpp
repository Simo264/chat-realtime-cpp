#pragma once

#include <cstdint>
#include <array>
#include <set>
#include <filesystem>

const auto db_users = std::filesystem::current_path() / "database/users.csv";
const auto db_rooms = std::filesystem::current_path() / "database/rooms.csv";
//const auto db_users = std::filesystem::path("/home/simone/Desktop/chat-realtime-cpp/database/users.csv");
//const auto db_rooms = std::filesystem::path("/home/simone/Desktop/chat-realtime-cpp/database/rooms.csv");

constexpr auto max_len_username = 32;
constexpr auto max_len_password = 16;
constexpr auto max_len_error_message = 256;
constexpr auto max_len_room_name = 32;

using RoomID = uint32_t;
constexpr RoomID invalid_room_id = static_cast<RoomID>(0xFFFFFFFF);

using ClientID = uint32_t;
constexpr ClientID invalid_client_id = static_cast<ClientID>(0xFFFFFFFF);

struct ServerRoomInfo
{
	std::set<ClientID> client_set{};
	std::array<char, max_len_room_name> room_name{};
	RoomID room_id{ invalid_room_id };
	ClientID creator_id{ invalid_client_id };
};

struct ClientRoomInfo
{
	std::array<char, max_len_room_name> room_name{};
	uint32_t user_count;
	RoomID room_id{ invalid_room_id };
	ClientID creator_id{ invalid_client_id };
};