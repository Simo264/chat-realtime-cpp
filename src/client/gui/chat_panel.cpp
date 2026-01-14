#include "chat_panel.hpp"

#include <imgui.h>
#include <array>
#include <cstring>
#include <shared_mutex>

#include "../chat_service_connector.hpp"
#include "../globals.hpp"

static auto s_input_buffer = std::array<char, max_len_chat_message>{};

// ==================================
// Private functions 
// ==================================

static void on_send(ChatServiceConnector& connector)
{
	if(std::strlen(s_input_buffer.data()) == 0)
		return;
	
	connector.SendMessage(g_current_room_id, g_client_id, s_input_buffer.data());
	s_input_buffer.fill(0);
}

// ==================================
// Public functions
// ==================================

namespace gui
{
	namespace chat_panel
	{
		void render_panel(ChatServiceConnector& connector)
		{
			ImGui::Begin("Chat", nullptr, ImGuiWindowFlags_NoDecoration);
			// Se non ci sono stanze selezionate
    	if (g_current_room_id == invalid_room_id) 
     	{
        ImGui::TextDisabled("No room selected.");
        ImGui::End();
        return;
      }

			constexpr auto input_height = 45.0f;
			ImGui::BeginChild("ChatMessages", ImVec2(0, -input_height), true, ImGuiWindowFlags_HorizontalScrollbar);
			{
				std::shared_lock lock(g_mutex_chat_messages);
				if (auto it = g_chat_messages.find(g_current_room_id); it != g_chat_messages.end()) 
				{
          for (const auto& msg : it->second) 
					{
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s:", msg.sender_name.data());
            ImGui::SameLine();
            ImGui::TextUnformatted(msg.content.data());
          }
        }
			}
			
			// Auto-scroll verso il basso quando arrivano nuovi messaggi
    	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
			ImGui::EndChild();

			ImGui::Separator();
	    ImGui::PushItemWidth(-70.0f);
	    
	    // Se premiamo invio nel campo di testo chiamiamo on_send
	    if (ImGui::InputText("##ChatInput", s_input_buffer.data(), s_input_buffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) 
			{
       	::on_send(connector);
        ImGui::SetKeyboardFocusHere(-1);
	    }
	    
	    ImGui::SameLine();
	    if (ImGui::Button("Send", ImVec2(60, 0)))
        ::on_send(connector);
			
			ImGui::End();
		}
	}
}