#include "header_panel.hpp"
#include <imgui.h>

#include "../globals.hpp"

namespace gui
{
	namespace header_panel
	{
		void render_panel()
		{
			ImGui::Begin("Header", nullptr, ImGuiWindowFlags_NoDecoration);
	    ImGui::Text("Client username: %s", g_client_username.data());
	    ImGui::SameLine();
	    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 150); 
	    ImGui::Text("Client id: %u", g_client_id);
	    ImGui::Text("Current room id: %u", g_current_room_id.load());
			ImGui::End();
		}
	}
}