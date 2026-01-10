#pragma once

#include <cstdint>
#include <array>
#include <set>
#include <filesystem>

const auto db_users = std::filesystem::current_path() / "database/users.csv";
const auto db_rooms = std::filesystem::current_path() / "database/rooms.csv";

constexpr auto max_len_username = 32;
constexpr auto max_len_password = 16;
constexpr auto max_len_error_message = 256;
constexpr auto max_len_room_name = 32;

using RoomID = uint32_t;
constexpr RoomID invalid_room_id = static_cast<RoomID>(0xFFFFFFFF);

using ClientID = uint32_t;
constexpr ClientID invalid_client_id = static_cast<ClientID>(0xFFFFFFFF);

struct RoomInfo
{
	std::set<ClientID> client_set{};
	std::array<char, max_len_room_name> room_name{};
	RoomID room_id{ invalid_room_id };
	ClientID creator_id{ invalid_client_id };
};