#include "users_panel.hpp"

#include <imgui.h>

#include "../globals.hpp"

namespace gui 
{
	namespace users_panel
	{
		void render_panel()
		{
			ImGui::Begin("Users", nullptr, ImGuiWindowFlags_NoDecoration);
			
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Connected Users");
      ImGui::Separator();	
      
      // Lock in lettura per accedere a g_room_users in sicurezza
      {				
				if (ImGui::BeginChild("UsersList")) 
				{
          for (auto client_id : g_room_users) 
           	ImGui::Text("client_id %u", client_id );
          ImGui::EndChild();
        }
      }
      
			ImGui::End();
		}
	}
}