#pragma once

class AuthServiceConnector;

namespace gui
{
	namespace auth_panel
	{
		void render_login(bool& login_mode, AuthServiceConnector& auth_service_connector);
		void render_signup(bool& login_mode, AuthServiceConnector& auth_service_connector);
	}
}