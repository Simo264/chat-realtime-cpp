#pragma once

#include <cstdint>
#include <array>
#include <filesystem>

const auto db_users = std::filesystem::current_path() / "database/users.csv";
const auto db_rooms = std::filesystem::current_path() / "database/rooms.csv";

constexpr auto max_len_username = 32;
constexpr auto max_len_password = 16;
constexpr auto max_len_auth_message = 256;
constexpr auto max_len_room_name = 30;
constexpr auto max_num_clients_per_room = 8;

using RoomID = uint8_t;
constexpr RoomID invalid_room_id = static_cast<RoomID>(0xFFFF);

using ClientID = uint8_t;
constexpr ClientID invalid_client_id = static_cast<ClientID>(0xFFFF);

struct RoomInfo
{
	std::array<char, max_len_room_name> room_name{};	
	std::array<ClientID, max_num_clients_per_room> clients{};
	RoomID room_id{ invalid_room_id };
	uint8_t user_count{};
};