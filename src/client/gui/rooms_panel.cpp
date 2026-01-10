#include "rooms_panel.hpp"

#include <array>
#include <array>
#include <imgui.h>
#include <print>
#include <vector>

#include "../rooms_service_connector.hpp"
#include "../globals.hpp"

static auto s_all_room_vector = std::vector<RoomInfo>{};
static auto s_error_message = std::array<char, max_len_error_message>{};
static auto s_buff_room_name = std::array<char, max_len_room_name>{};

// ========================================
// Private functions
// ========================================
 
static void render_joined_rooms_list(RoomsServiceConnector& connector)
{
	ImGui::TextDisabled("You haven't joined any rooms yet.");
}

static void render_all_rooms_table(RoomsServiceConnector& connector, float table_height)
{
 	ImGui::Text("Available rooms");
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
      for (const auto& room : s_all_room_vector) 
			{
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("%u", room.room_id);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%u", room.creator_id);
        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(room.room_name.data());
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%zu", room.client_set.size());
        ImGui::TableSetColumnIndex(4);
        ImGui::PushID(room.room_id);
        if (ImGui::Button("Join", ImVec2(90.0f, 0))) 
        {}
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
	    for (const auto& room : s_all_room_vector) 
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
        ImGui::Text("%zu", room.client_set.size());
        ImGui::TableSetColumnIndex(4);
	      ImGui::PushID(room.room_id + 10000);
	      if (ImGui::Button("Delete", ImVec2(90.0f, 0))) 
	      {
					auto success = connector.CallRemoteDeleteRoomProcedure(room.room_id, g_client_id, s_error_message);
					if(success)
					{
						std::println("Room '{}' deleted and removed from vector", room.room_name.data());
			
						auto removed_count = std::erase_if(s_all_room_vector, [&](const RoomInfo& elem) {
					    return elem.room_id == room.room_id;
						});
						if (removed_count > 0)
							std::println("Room '{}' removed from vector", room.room_name.data());
					}
					else
					{
						std::println("Error on deleting room '{}'", room.room_name.data());
					}
				}
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
  ImGui::InputTextWithHint("##NewRoomName", "Enter room name...", s_buff_room_name.begin(), max_len_room_name);
  ImGui::SameLine();

  if (ImGui::Button("Create", ImVec2(button_width, 0))) 
  {
    auto client_id = g_client_id;
    auto success = connector.CallRemoteCreateRoomProcedure(client_id, s_buff_room_name.begin(), s_error_message);
    if (success) 
    {
      std::println("Room '{}' created!", s_buff_room_name.data());
      success = connector.CallRemoteListRoomsProcedure(s_all_room_vector, s_error_message);
      if (!success) 
        std::println("Error on retrieving list of rooms. {}", s_error_message.data());
    } 
    else 
      std::println("Error on creating room '{}'. {}", s_buff_room_name.data(), s_error_message.data());
  }
}

static void render_explore_button(RoomsServiceConnector& connector)
{
	if (ImGui::Button("Explore", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
	{
		auto success = connector.CallRemoteListRoomsProcedure(s_all_room_vector, s_error_message);
		if(!success)
			std::println("Error on retrieving list of rooms. {}", s_error_message.data());
		else
			ImGui::OpenPopup("ExploreRoomsModal");
	} 
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
      // To completely clear a vector explicitly
      s_all_room_vector.clear();
      s_all_room_vector.shrink_to_fit();
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
      ::render_explore_button(connector);
      // Explore modal
      ::render_explore_modal(connector);
   		ImGui::End();
		}
	}
}