#include "users_panel.hpp"

#include <imgui.h>

namespace gui 
{
	namespace users_panel
	{
		void render_panel()
		{
			ImGui::Begin("Users", nullptr, ImGuiWindowFlags_NoDecoration);
			ImGui::End();
		}
	}
}