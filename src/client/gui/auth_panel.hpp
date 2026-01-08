#pragma once

#include "../../common.hpp"

class AuthServiceConnector;

namespace gui
{
	namespace auth_panel
	{
		// Return the client id
		ClientID render_login(bool& login_mode, 
													AuthServiceConnector& connector, 
													std::array<char, max_len_username>& out_username);
		
		ClientID render_signup(bool& login_mode, 
													AuthServiceConnector& connector, 
													std::array<char, max_len_username>& out_username);
	}
}