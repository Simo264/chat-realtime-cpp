#pragma once

#include <filesystem>

struct GLFWwindow;
class AuthServiceConnector;

namespace gui
{
	// Setup Dear ImGui context
	void initialize_imgui_context(GLFWwindow* window_manager);
	
	void set_custom_styling();
	
	void set_custom_font(const std::filesystem::path& file_path);
	
	// Start the Dear ImGui frame
	void start_new_frame();

	void render_auth_page(AuthServiceConnector& auth_service_connector);

	// void set_docking_layout();

	void rendering(GLFWwindow* window_manager);
}