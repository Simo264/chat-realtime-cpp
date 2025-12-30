#include "imgui_manager.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

namespace GUI
{
	void initialize_imgui_context(GLFWwindow* window_manager)
	{
	  IMGUI_CHECKVERSION();
	  ImGui::CreateContext();
	  ImGuiIO& io = ImGui::GetIO(); (void)io;
	  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	  ImGui::StyleColorsDark();
	  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	  {
	    ImGuiStyle& style = ImGui::GetStyle();
	    style.WindowRounding = 0.0f;
	    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	  }
	  ImGui_ImplGlfw_InitForOpenGL(window_manager, true);
	  ImGui_ImplOpenGL3_Init("#version 460");
	}
	
	void start_new_frame()
	{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
	}
	
	void set_docking_layout()
	{
  	ImGuiViewport* viewport = ImGui::GetMainViewport();
    static ImGuiID dockspace_id = ImGui::GetID("Dockspace");
    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_None);
		
	  // Eseguiamo la configurazione del layout solo una volta (o quando necessario)
    static bool first_time = true;
    if (first_time)
    {
      first_time = false;
  
      // Puliamo eventuali layout precedenti per questo ID
      ImGui::DockBuilderRemoveNode(dockspace_id); 
      ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);
      ImGuiID dock_id_left, dock_id_right;
      ImGuiID dock_id_right_top, dock_id_right_bottom;
      // 1. Dividiamo lo spazio principale in Orizzontale (Sinistra 40% - Destra 60%)
      ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.40f, &dock_id_left, &dock_id_right);
      // 2. Dividiamo la parte destra in Verticale (Top Viewport - Bottom Input)
      // Usiamo circa 0.70f per la viewport (70% sopra, 30% sotto per l'input)
      ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.70f, &dock_id_right_top, &dock_id_right_bottom);
      // 3. Assegniamo le finestre ai nodi creati tramite i loro nomi
      ImGui::DockBuilderDockWindow("left_panel", dock_id_left);
      ImGui::DockBuilderDockWindow("main_panel", dock_id_right_top);
      ImGui::DockBuilderDockWindow("input_panel", dock_id_right_bottom);
      ImGui::DockBuilderFinish(dockspace_id);
    }
	}
	
	void rendering(GLFWwindow* window_manager)
	{
		ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window_manager, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
	}
	
}