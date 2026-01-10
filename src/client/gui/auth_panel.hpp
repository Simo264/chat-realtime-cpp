#pragma once

class AuthServiceConnector;

namespace gui
{
	namespace auth_panel
	{
		// return auth success
		bool render_login(bool& login_mode, AuthServiceConnector& connector);
		bool render_signup(bool& login_mode, AuthServiceConnector& connector);
	}
}