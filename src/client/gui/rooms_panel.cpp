#include "rooms_panel.hpp"

#include <array>
#include <array>
#include <imgui.h>
#include <mutex>
#include <print>
#include <shared_mutex>
#include <vector>

#include "../rooms_service_connector.hpp"
#include "../globals.hpp"

static auto s_error_message = std::array<char, max_len_error_message>{};
static auto s_buffer_input_room_name = std::array<char, max_len_room_name>{};

// ========================================
// Private functions
// ========================================
 
static void on_join_button(RoomsServiceConnector& connector, ClientRoomInfo& room)
{
	auto success = connector.CallRemoteJoinRoomProcedure(room.room_id, g_client_id, s_error_message);
 	if(!success)
  {
   	std::println("Error on joining the room '{}'. {}", room.room_name.data(), s_error_message.data());
  }
}

static void on_leave_button(RoomsServiceConnector& connector, ClientRoomInfo& room)
{
	auto success = connector.CallRemoteLeaveRoomProcedure(room.room_id, g_client_id, s_error_message);	
 	if(!success)
  {
 		std::println("Error on leaving the room '{}'. {}", room.room_name.data(), s_error_message.data());
  	return;
  }
  
 	if (g_current_room_id == room.room_id)
  {
    {
      std::lock_guard lock_chat(g_mutex_chat_messages);
      g_chat_messages.erase(room.room_id);
    }
  
   	if (g_joined_room_vector.empty()) 
   	{
      // Non ci sono più stanze
      g_current_room_id = invalid_room_id;
    } 
    else 
    {
      // Selezioniamo la prima stanza disponibile nel set
      std::shared_lock lock(g_mutex_joined_room_vector);        
      g_current_room_id =  *g_joined_room_vector.begin();
    }
  }
}

static void on_delete_room(RoomsServiceConnector& connector, ClientRoomInfo& room)
{
	auto success = connector.CallRemoteDeleteRoomProcedure(room.room_id, g_client_id, s_error_message);
	if(!success)
	{
		std::println("Error on deleting room '{}' ({})", room.room_name.data(), room.room_id);
	}
}

static void on_create_room(RoomsServiceConnector& connector)
{
 	auto client_id = g_client_id;
  auto success = connector.CallRemoteCreateRoomProcedure(client_id, s_buffer_input_room_name.begin(), s_error_message);
  if (!success)
  {
	  std::println("Error on creating room '{}'. {}", s_buffer_input_room_name.data(), s_error_message.data());
  }
}

static void render_joined_rooms_list(RoomsServiceConnector& connector)
{
	// Acquisiamo entrambi i lock perché leggiamo da entrambi i contenitori
  std::shared_lock lock_joined(g_mutex_joined_room_vector);
  std::shared_lock lock_all(g_mutex_all_room_vector);
  if(g_joined_room_vector.empty())
  	return;

  for (auto joined_id : g_joined_room_vector) 
  {
    auto it = std::find_if(g_all_room_vector.begin(), g_all_room_vector.end(), [joined_id](const ClientRoomInfo& r) { 
     	return r.room_id == joined_id; });

    if (it == g_all_room_vector.end()) 
     	continue;

    auto& room = *it;
  
    auto label = std::array<char, max_len_room_name + 64>{};
    std::format_to(label.begin(), "{} ({})", room.room_name.data(), room.user_count);
    auto is_selected = (g_current_room_id == room.room_id);
    
    ImGui::PushID(room.room_id);
    if (ImGui::Selectable(label.data(), is_selected)) 
    {
    	g_current_room_id = room.room_id;
      std::println("Room '{}' selected (ID: {})", room.room_name.data(), room.room_id);
    }
    
		if (ImGui::IsItemHovered()) 
		{
      ImGui::BeginTooltip();
      ImGui::Text("Nome: %s", room.room_name.data());
      ImGui::Text("Room ID: %u", room.room_id);
      ImGui::Text("Creator ID: %u", room.creator_id);
      ImGui::Text("User count: %u", room.user_count);
      ImGui::EndTooltip();
    }

	  ImGui::PopID();
  }
}

static void render_all_rooms_table(RoomsServiceConnector& connector, float table_height)
{
 	ImGui::Text("Available rooms (%zu)", g_all_room_vector.size());
  if (ImGui::BeginChild("AllRoomsList", ImVec2(0, table_height), true, ImGuiWindowFlags_NoResize))
  {
	  if (ImGui::BeginTable("RoomsTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY)) 
    {
    	ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
     	ImGui::TableSetupColumn("Creator", ImGuiTableColumnFlags_WidthFixed, 60.0f);
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Users", ImGuiTableColumnFlags_WidthFixed, 40.0f);
      ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 100.0f);
      ImGui::TableHeadersRow();
     	
      // Acquisiamo i lock in lettura per entrambi i contenitori
      std::shared_lock lock_all(g_mutex_all_room_vector);
      std::shared_lock lock_joined(g_mutex_joined_room_vector);
      for (auto& room : g_all_room_vector) 
			{
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%u", room.room_id);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%u", room.creator_id);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(room.room_name.data());
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%u", room.user_count);
        ImGui::TableSetColumnIndex(4);
        ImGui::PushID(room.room_id);
        
        // Verifica se l'ID della stanza è presente nel set delle stanze "joined"
        auto is_already_joined = (g_joined_room_vector.find(room.room_id) != g_joined_room_vector.end());
        if (!is_already_joined) 
        {
          if (ImGui::Button("Join", ImVec2(90.0f, 0))) 
            ::on_join_button(connector, room);
        }
        else 
        {
        	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
          if (ImGui::Button("Leave", ImVec2(90.0f, 0)))
            ::on_leave_button(connector, room);
          ImGui::PopStyleColor();
        }
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  ImGui::EndChild();
}

static void render_my_rooms_table(RoomsServiceConnector& connector, float table_height)
{
	ImGui::Text("My rooms (creator)");
	if (ImGui::BeginChild("MyRoomsList", ImVec2(0, table_height), true, ImGuiWindowFlags_NoResize))
	{
		if (ImGui::BeginTable("MyRoomsTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY)) 
	  {
			ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
			ImGui::TableSetupColumn("Creator", ImGuiTableColumnFlags_WidthFixed, 60.0f);
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Users", ImGuiTableColumnFlags_WidthFixed, 40.0f);
      ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 100.0f);
      
      // Acquisiamo il lock in lettura per la lista globale g_all_room_vector
      std::shared_lock lock(g_mutex_all_room_vector);
      
      auto id_to_delete = invalid_room_id;
	    for (auto& room : g_all_room_vector)
			{
	      if (room.creator_id != g_client_id)
       		continue;
	      
	      ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%u", room.room_id);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%u", room.creator_id);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(room.room_name.data());
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%u", room.user_count);
        ImGui::TableSetColumnIndex(4);
	      ImGui::PushID(room.room_id + 10000);
	      if (ImGui::Button("Delete", ImVec2(90.0f, 0)))
					::on_delete_room(connector, room);
	      ImGui::PopID();
	    }
	    ImGui::EndTable();
	  }
	}
	ImGui::EndChild();
}

static void render_form_create_room(RoomsServiceConnector& connector) 
{
  ImGui::Text("Create a new room:");
  constexpr auto button_width = 64.0f;
  auto input_width = ImGui::GetContentRegionAvail().x - (button_width + ImGui::GetStyle().ItemSpacing.x);
  ImGui::SetNextItemWidth(input_width);
  ImGui::InputTextWithHint("##NewRoomName", "Enter room name...", s_buffer_input_room_name.begin(), max_len_room_name);
  ImGui::SameLine();

  if (ImGui::Button("Create", ImVec2(button_width, 0))) 
  	::on_create_room(connector);
}

static void render_explore_modal(RoomsServiceConnector& connector) 
{
  ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_Appearing);
  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal("ExploreRoomsModal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) 
  {
  	constexpr auto table_height = 200.f;
  
    ::render_all_rooms_table(connector, table_height);
    ImGui::Spacing();
    
    ::render_my_rooms_table(connector, table_height);
    ImGui::Spacing();
    ImGui::Separator();
    
    if (std::strlen(s_error_message.data()) > 0)
    {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
      ImGui::TextWrapped("Error: %s", s_error_message.data());
      ImGui::PopStyleColor();
    }
    
    auto footer_needed_height = ImGui::GetFrameHeightWithSpacing() * 3.0f + ImGui::GetStyle().ItemSpacing.y * 2;
    auto footer_y_pos = ImGui::GetContentRegionMax().y - footer_needed_height;
    ImGui::SetCursorPosY(footer_y_pos);
    
    ::render_form_create_room(connector);
    ImGui::Spacing();
    
    ImGui::Separator();
    if (ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
    {
      s_error_message.fill(0);
      s_buffer_input_room_name.fill(0);
      // To completely clear a vector explicitly
      // g_all_room_vector.clear();
      // g_all_room_vector.shrink_to_fit();
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
   		ImGui::Begin("Rooms", nullptr, ImGuiWindowFlags_NoDecoration);
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Joined rooms");
      ImGui::Separator();
      
      auto footer_height = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y;
      auto list_height = ImGui::GetContentRegionAvail().y - footer_height;
      
      // render rooms
      if (ImGui::BeginChild("JoinedRoomsList", ImVec2(0, list_height), false))
        ::render_joined_rooms_list(connector);
      ImGui::EndChild();
      
      ImGui::Separator();

      // Explore button
      ImGui::SetCursorPosY(ImGui::GetWindowHeight() - footer_height);
     	if (ImGui::Button("Explore", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
      	ImGui::OpenPopup("ExploreRoomsModal");
      
      // Explore modal
      ::render_explore_modal(connector);
   		ImGui::End();
		}
	}
}