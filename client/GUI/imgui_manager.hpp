#pragma once

struct GLFWwindow;

namespace GUI
{
	// Setup Dear ImGui context
	void initialize_imgui_context(GLFWwindow* window_manager);
	
	// Start the Dear ImGui frame
	void start_new_frame();
	
	void set_docking_layout();

	void rendering(GLFWwindow* window_manager);
}