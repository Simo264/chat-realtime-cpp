#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <print>
#include <memory>
#include <cassert>
#include <string_view>

#include <grpcpp/grpcpp.h>
#include "server_service.grpc.pb.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "GUI/imgui_manager.hpp"

using server_service::ServerServiceInterface;
using server_service::MessageRequest;
using server_service::MessageResponse;

constexpr auto SERVER_ADDRESS = "localhost:9090";
 
class ServerServiceClient
{
	public:
		ServerServiceClient(std::shared_ptr<grpc::Channel> channel) : m_stub{ ServerServiceInterface::NewStub(channel) } {}
		
	private:
		std::unique_ptr<ServerServiceInterface::Stub> m_stub;
};

static void glfw_error_callback(int error, const char* description)
{
	std::println("Error: {}", description);
}

static auto initialize_window_manager(std::string_view title, int width, int height)
{
	glfwSetErrorCallback(glfw_error_callback);
	int result = glfwInit();
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
	auto window_manager = initialize_window_manager("Chat distribuita", 1080, 720);
	
  GUI::initialize_imgui_context(window_manager);
  
	while (!glfwWindowShouldClose(window_manager))
	{
  	glfwPollEvents();
    //if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
    //{
    //  ImGui_ImplGlfw_Sleep(10);
    //  continue;
    //}
    
    GUI::start_new_frame();
    // Set layout
    GUI::set_docking_layout();

    // Definizione delle finestre
    ImGui::Begin("Sinistra");
    ImGui::Text("Contenuto 40%%");
    ImGui::End();
    ImGui::Begin("Viewport");
    ImGui::Text("Qui va la scena 3D");
    ImGui::End();
    ImGui::Begin("Input Utente");
    ImGui::Text("Qui vanno i comandi");
    ImGui::End();
    
    // Rendering
    GUI::rendering(window_manager);
       
		glfwSwapBuffers(window_manager);
    glfwPollEvents();
	}
		
	// ServerServiceClient rg(grpc::CreateChannel(SERVER_ADDRESS, grpc::InsecureChannelCredentials()));
	// std::string reply = greeter.SayHello("world");
	// std::println("Greeter received: {}", reply);
	// reply = greeter.SayHelloAgain("world");
	// std::println("Greeter received: {}", reply);
	
 	// Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
	
	glfwDestroyWindow(window_manager);
  glfwTerminate();
  return 0;
}
