#include "globals.hpp"
#include <array>

ClientID g_client_id = ClientID{ invalid_client_id };
std::array<char, max_len_username> g_client_username{};