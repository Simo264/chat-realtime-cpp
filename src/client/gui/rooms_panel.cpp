#include "rooms_panel.hpp"

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
     
   		ImGui::End();
		}
	}
}