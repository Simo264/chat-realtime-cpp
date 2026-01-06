#include "rooms_panel.hpp"

#include <array>
#include <format>
#include <string>
#include <array>
#include <format>
#include <imgui.h>

#include "../rooms_service_connector.hpp"

namespace gui
{
	namespace rooms_panel
	{
		void render_panel(RoomsServiceConnector& connector)
		{
			constexpr auto window_flags = ImGuiWindowFlags_NoCollapse | 
                                		ImGuiWindowFlags_NoTitleBar | 
                                  	ImGuiWindowFlags_NoMove;
                                   
   		ImGui::Begin("Stanze", nullptr, window_flags);
    	auto& client_rooms = connector.GetRoomsSnapshot();
     	for (const auto& room : client_rooms) 
     	{
      	auto& room_name = room.room_name;
       	auto room_id = room.room_id;
        auto room_user_count = room.user_count;
      	ImGui::Text("%d, %s (%d users)", room_id, room_name.data(), room_user_count);
     	}
   		ImGui::End();
		}
	}
}