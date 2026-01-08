#include "rooms_panel.hpp"

#include <array>
#include <format>
#include <string>
#include <array>
#include <format>
#include <imgui.h>
#include <vector>

#include "../rooms_service_connector.hpp"

static auto s_all_room_vector = std::vector<RoomInfo>{};

// ========================================
// Private functions
// ========================================
static void render_rooms_list(RoomsServiceConnector& connector)
{
	auto footer_height = ImGui::GetFrameHeightWithSpacing() * 2.5f; 
  if (ImGui::BeginChild("RoomsList", ImVec2(0, -footer_height), false)) 
  {
   // auto& client_rooms = connector.GetJoinedRoomVector();
   // for (const auto& room : client_rooms) 
   // {     
   // 	char label[128];
   //   sprintf(label, "%s##%d", room.room_name.data(), room.room_id);
  
   //   if (ImGui::Selectable(label, false)) {}
   // 
   //   if (ImGui::BeginPopupContextItem()) 
   //   {
   //     if (ImGui::MenuItem("Invita")) 
   //     {}
   //     
   //     ImGui::Separator();
   //     
   //     ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
   //     if (ImGui::MenuItem("Esci dalla stanza", nullptr, false, true)) 
   //     {}
   //     ImGui::PopStyleColor();
   //     ImGui::EndPopup();
   //   }
   //   
   //   // Conteggio utenti 
   //   ImGui::SameLine(ImGui::GetWindowWidth() - 40);
   //   ImGui::TextDisabled("%d", room.user_count);
   // }
    ImGui::EndChild();
  }
}

static void render_add_room_button() 
{
 	if (ImGui::Button("Create/Join", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
    ImGui::OpenPopup("AddRoomModal");
}

static void render_add_room_modal() 
{
 	if (ImGui::BeginPopupModal("AddRoomModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
  {
    ImGui::Text("Crea una nuova stanza");
    auto buff = std::array<char, max_len_room_name>{};
    ImGui::InputText("Nome", buff.begin(), max_len_room_name - 1);
    
    if (ImGui::Button("Crea", ImVec2(120, 0))) 
    {
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Annulla", ImVec2(120, 0))) 
    { 
     	ImGui::CloseCurrentPopup(); 
    }
    ImGui::EndPopup();
  }
}

static void render_explore_button(RoomsServiceConnector& connector)
{
	if (ImGui::Button("Esplore", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
	{
		//connector.CallRemoteGetAllRoomsProcedure(s_all_room_vector);
		ImGui::OpenPopup("ExploreRoomsModal");
	} 
}

static void render_explore_modal() 
{
  ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal("ExploreRoomsModal", nullptr, ImGuiWindowFlags_NoMove)) 
  {
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Seleziona una stanza per unirti");
    ImGui::Separator();

    if (ImGui::BeginChild("ExploreList", ImVec2(0, -40), true)) 
    {
      if (s_all_room_vector.empty()) 
      {
        ImGui::TextDisabled("Nessuna stanza disponibile al momento.");
      }
      else 
      {
        for (const auto& room : s_all_room_vector) 
        {
        	auto label = std::array<char, 64>{};
         	std::format_to_n(label.begin(), 64, "{} (id {})", room.room_name.data(), room.room_id);
          if (ImGui::Selectable(label.data())) 
          {
           	// TODO
            ImGui::CloseCurrentPopup();
          }
        }
      }
      ImGui::EndChild();
    }

    ImGui::Separator();
    if (ImGui::Button("Chiudi", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
    { 
      ImGui::CloseCurrentPopup(); 
    }
    
    ImGui::EndPopup();
  }
}


// ========================================
// Public interface
// ========================================

namespace gui
{
	namespace rooms_panel
	{
		void render_panel(RoomsServiceConnector& connector)
		{
			constexpr auto window_flags = ImGuiWindowFlags_NoDecoration;
   		ImGui::Begin("Stanze", nullptr, window_flags);
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "LE TUE STANZE");
      ImGui::Separator();
      
      // render rooms
      ::render_rooms_list(connector);
      
      ImGui::Separator();

      // Create/Join button
      ::render_add_room_button();
      // Explore button
      ::render_explore_button(connector);
      // Create/Joing modal
      ::render_add_room_modal();
      // Explore modal
      ::render_explore_modal();
   		ImGui::End();
		}
	}
}