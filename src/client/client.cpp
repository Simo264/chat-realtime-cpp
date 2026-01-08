#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <print>
#include <cassert>
#include <string_view>
#include <filesystem>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <vector>

#include "auth_service_connector.hpp"
#include "rooms_service_connector.hpp"
#include "gui/imgui_manager.hpp"

#include "../common.hpp"

constexpr auto SERVER_ADDRESS = "localhost:9090";
 
static auto initialize_window_manager(std::string_view title, int width, int height)
{
	glfwSetErrorCallback([](int error, const char* description){ 	std::println("GLFW error: {}", description); });
	auto result = glfwInit();
	assert(result == GLFW_TRUE && "Error on init GLFW");
  
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  auto window = glfwCreateWindow(width, height, title.data(), NULL, NULL);
  assert(window != nullptr && "Error on create window");
  
  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);
  glfwSwapInterval(1);
  return window;
}

int main()
{
	auto auth_service_connector = AuthServiceConnector{ grpc::CreateChannel(SERVER_ADDRESS, grpc::InsecureChannelCredentials()) };
	auto rooms_service_connector = RoomsServiceConnector{ grpc::CreateChannel(SERVER_ADDRESS, grpc::InsecureChannelCredentials()) };
	
	auto window_manager = initialize_window_manager("Chat distribuita", 1080, 720);
  gui::initialize_imgui_context(window_manager);
  gui::set_custom_styling();
  gui::set_custom_font(std::filesystem::current_path() / "resources/fonts/RedHatDisplay-Medium.ttf");
  
  auto client_id = ClientID{ invalid_client_id };    
	while (!glfwWindowShouldClose(window_manager))
	{
  	glfwPollEvents();
    gui::start_new_frame();
    
    auto view_auth_page = (client_id == invalid_client_id);
    if(view_auth_page)
    {
   		client_id = gui::render_auth_page(auth_service_connector);
    	//client_id = 0;
     	if(client_id != invalid_client_id)
        rooms_service_connector.WatchRooms(client_id);
    }
    else 
		{
			gui::set_docking_layout();
			gui::render_header("guest", client_id);
			gui::render_rooms_panel(rooms_service_connector);
      gui::render_chat_panel();
      gui::render_users_panel();
	  }
    
    gui::rendering(window_manager);
		glfwSwapBuffers(window_manager);
    glfwPollEvents();
	}
	
	rooms_service_connector.Stop();
	
 	// Cleanup Imgui context
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  // Cleanup GLFW context
	glfwDestroyWindow(window_manager);
  glfwTerminate();
  return 0;
}
