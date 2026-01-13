#include "imgui_manager.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "../auth_service_connector.hpp"
#include "../rooms_service_connector.hpp"
#include "../chat_service_connector.hpp"
#include "../globals.hpp"
#include "auth_panel.hpp"
#include "rooms_panel.hpp"
#include "chat_panel.hpp"

namespace gui
{
	void initialize_imgui_context(GLFWwindow* window_manager)
	{
	  IMGUI_CHECKVERSION();
	  ImGui::CreateContext();
	  auto& io = ImGui::GetIO(); (void)io;
	  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
				
	  ImGui_ImplGlfw_InitForOpenGL(window_manager, true);
	  ImGui_ImplOpenGL3_Init("#version 460");
	}

	void set_custom_styling()
	{
	  ImGui::StyleColorsDark();
		auto style = &ImGui::GetStyle();
		auto colors = style->Colors;
		
		// Base colors for a pleasant and modern dark theme with dark accents
		colors[ImGuiCol_Text] = ImVec4(0.92f, 0.93f, 0.94f, 1.00f);                  // Light grey text for readability
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.52f, 0.54f, 1.00f);          // Subtle grey for disabled text
		colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);              // Dark background with a hint of blue
		colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);               // Slightly lighter for child elements
		colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);               // Popup background
		colors[ImGuiCol_Border] = ImVec4(0.28f, 0.29f, 0.30f, 0.60f);                // Soft border color
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);          // No border shadow
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);               // Frame background
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);        // Frame hover effect
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.26f, 0.28f, 1.00f);         // Active frame background
		colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);               // Title background
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);         // Active title background
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);      // Collapsed title background
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);             // Menu bar background
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);           // Scrollbar background
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.26f, 0.28f, 1.00f);         // Dark accent for scrollbar grab
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.30f, 0.32f, 1.00f);  // Scrollbar grab hover
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.32f, 0.34f, 0.36f, 1.00f);   // Scrollbar grab active
		colors[ImGuiCol_CheckMark] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);             // Dark blue checkmark
		colors[ImGuiCol_SliderGrab] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);            // Dark blue slider grab
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);      // Active slider grab
		colors[ImGuiCol_Button] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);                // Dark blue button
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);         // Button hover effect
		colors[ImGuiCol_ButtonActive] = ImVec4(0.32f, 0.42f, 0.52f, 1.00f);          // Active button
		colors[ImGuiCol_Header] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);                // Header color similar to button
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);         // Header hover effect
		colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.42f, 0.52f, 1.00f);          // Active header
		colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);             // Separator color
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);      // Hover effect for separator
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);       // Active separator
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);            // Resize grip
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);     // Hover effect for resize grip
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.44f, 0.54f, 0.64f, 1.00f);      // Active resize grip
		colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);                   // Inactive tab
		colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.38f, 0.48f, 1.00f);            // Hover effect for tab
		colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);             // Active tab color
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);          // Unfocused tab
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.24f, 0.34f, 0.44f, 1.00f);    // Active but unfocused tab
		colors[ImGuiCol_PlotLines] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);             // Plot lines
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);      // Hover effect for plot lines
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.36f, 0.46f, 0.56f, 1.00f);         // Histogram color
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);  // Hover effect for histogram
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);         // Table header background
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.28f, 0.29f, 0.30f, 1.00f);     // Strong border for tables
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.24f, 0.25f, 0.26f, 1.00f);      // Light border for tables
		colors[ImGuiCol_TableRowBg] = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);            // Table row background
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.22f, 0.24f, 0.26f, 1.00f);         // Alternate row background
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.24f, 0.34f, 0.44f, 0.35f);        // Selected text background
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.46f, 0.56f, 0.66f, 0.90f);        // Drag and drop target
		colors[ImGuiCol_NavHighlight] = ImVec4(0.46f, 0.56f, 0.66f, 1.00f);          // Navigation highlight
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f); // Windowing highlight
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);     // Dim background for windowing
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);      // Dim background for modal windows
		
		// Style adjustments
		style->WindowPadding = ImVec2(8.00f, 8.00f);
		style->FramePadding = ImVec2(5.00f, 2.00f);
		style->CellPadding = ImVec2(6.00f, 6.00f);
		style->ItemSpacing = ImVec2(6.00f, 6.00f);
		style->ItemInnerSpacing = ImVec2(6.00f, 6.00f);
		style->TouchExtraPadding = ImVec2(0.00f, 0.00f);
		style->IndentSpacing = 25;
		style->ScrollbarSize = 11;
		style->GrabMinSize = 10;
		style->WindowBorderSize = 1;
		style->ChildBorderSize = 1;
		style->PopupBorderSize = 1;
		style->FrameBorderSize = 1;
		style->TabBorderSize = 1;
		style->WindowRounding = 7;
		style->ChildRounding = 4;
		style->FrameRounding = 3;
		style->PopupRounding = 4;
		style->ScrollbarRounding = 9;
		style->GrabRounding = 3;
		style->LogSliderDeadzone = 4;
		style->TabRounding = 4;	
	}
	
	void set_custom_font(const std::filesystem::path& file_path)
	{
		auto& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF(file_path.string().c_str());
	}
	
	void start_new_frame()
	{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
	}
	
	void set_docking_layout()
	{
    auto viewport = ImGui::GetMainViewport();
    auto dockspace_id = ImGui::GetID("MainDockspace");
    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    static auto first_time = true;
    if (!first_time) 
    	return;
   	
    first_time = false;

    constexpr auto flags = static_cast<ImGuiDockNodeFlagsPrivate_>(ImGuiDockNodeFlags_PassthruCentralNode) | ImGuiDockNodeFlags_DockSpace;
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, flags);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    // Split in alto (10%)
    // dock_top prenderà il 10% superiore, dockspace_id conterrà il restante 90%
    auto dock_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.10f, nullptr, &dockspace_id);
    // Split a sinistra (20% del rimanente 90%)
    auto dock_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.20f, nullptr, &dockspace_id);
    // Split a destra (20% del rimanente)
    auto dock_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.20f, nullptr, &dockspace_id);
    // Centro (quello che resta dopo i tagli sopra, sinistra e destra)
    auto dock_center = dockspace_id;

    ImGui::DockBuilderDockWindow("Header", dock_top);
    ImGui::DockBuilderDockWindow("Rooms", dock_left);
    ImGui::DockBuilderDockWindow("Chat", dock_center);
    ImGui::DockBuilderDockWindow("Users", dock_right);
    ImGui::DockBuilderFinish(dockspace_id);
	}
	
	bool render_auth_page(AuthServiceConnector& connector)
	{
		static auto login_mode = true;
		if(login_mode)
		 	return gui::auth_panel::render_login(login_mode, connector);
		else
			return gui::auth_panel::render_signup(login_mode, connector);
	}
	
	void render_header()
	{
		ImGui::Begin("Header", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::Text("Client username: %s", g_client_username.data());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 150); 
    ImGui::Text("Client id: %d", g_client_id);
		ImGui::End();
	}
	
	void render_rooms_panel(RoomsServiceConnector& connector)
	{
		gui::rooms_panel::render_panel(connector);
	}
	
	void render_chat_panel(ChatServiceConnector& connector)
	{
		gui::chat_panel::render_panel(connector);
	}
	
	void render_users_panel()
	{
		ImGui::Begin("Users", nullptr, ImGuiWindowFlags_NoDecoration);
		ImGui::End();
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
    
    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      auto backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
	}
}