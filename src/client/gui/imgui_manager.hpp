#pragma once

#include <filesystem>
#include "../../common.hpp"

struct GLFWwindow;
class AuthServiceConnector;
class RoomsServiceConnector;

namespace gui
{
	// Setup Dear ImGui context
	void initialize_imgui_context(GLFWwindow* window_manager);
	void set_custom_styling();
	void set_custom_font(const std::filesystem::path& file_path);
	void start_new_frame();
	void set_docking_layout();
	
	// Return auth success
	ClientID render_auth_page(AuthServiceConnector& connector);
	void render_rooms_panel(RoomsServiceConnector& connector);
	void render_chat_panel();
	void render_users_panel();

	void rendering(GLFWwindow* window_manager);
}