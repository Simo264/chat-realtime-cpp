#include "auth_panel.hpp"
#include "imgui.h"

#include "../auth_service_connector.hpp"
#include "../../common.hpp"
#include "../globals.hpp"

#include <array>
#include <format>
#include <print>

static auto s_field_username = std::array<char, max_len_username>{};
static auto s_field_password = std::array<char, max_len_password>{};
static auto s_auth_error_message = std::array<char, max_len_error_message>{};
static auto s_auth_success = false;

// ========================================
// Private functions
// ========================================

static bool on_submit(AuthServiceConnector& connector, bool login_mode)
{
	auto client_id = ClientID{ invalid_client_id };	
	if(login_mode)
		client_id = connector.CallRemoteLoginProcedure(s_field_username.data(), s_field_password.data(), s_auth_error_message);
	else
		client_id = connector.CallRemoteSignupProcedure(s_field_username.data(), s_field_password.data(), s_auth_error_message);

	if(client_id != invalid_client_id)
	{
		g_client_id = client_id;
		g_client_username = s_field_username;
		s_auth_error_message.fill(0);
		std::println("Authentication successful! Your client_id: {}", client_id);
		return true;
	}
	
	std::println("Authentication failed. {}", s_auth_error_message.data());
	return false;
}

// ========================================
// Public interface
// ========================================

namespace gui
{
	namespace auth_panel
	{
		bool render_login(bool& login_mode, AuthServiceConnector& connector)
		{
      auto viewport = ImGui::GetMainViewport();
      auto work_size = viewport->WorkSize;
      auto work_pos = viewport->WorkPos;
      ImGui::SetNextWindowPos(work_pos);
      ImGui::SetNextWindowSize(work_size);
      
      constexpr auto window_flags = ImGuiWindowFlags_NoDecoration | 
      															ImGuiWindowFlags_NoMove | 
                    								ImGuiWindowFlags_NoSavedSettings;

      constexpr auto INPUT_WIDTH = 250.0f;
      constexpr auto login_page_str = "Login page";
      
      ImGui::Begin("LoginScreen", nullptr, window_flags);
      auto title_y = work_size.y * 0.20f;
      auto group_y = (work_size.y - 200.0f) * 0.5f;
      auto title_x = (work_size.x - ImGui::CalcTextSize(login_page_str).x) * 0.5f;
      ImGui::SetCursorPos(ImVec2(title_x, title_y));
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.5f, 1.0f, 1.0f));
      ImGui::Text("%s", login_page_str);
      ImGui::PopStyleColor();
      
      auto window_center_x = (work_size.x - INPUT_WIDTH) * 0.5f;
      ImGui::SetCursorPos(ImVec2(window_center_x, group_y));
      ImGui::BeginGroup();
      {
        ImGui::Text("Username");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##username", s_field_username.data(), max_len_username);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", s_field_password.data(), max_len_password, ImGuiInputTextFlags_Password);
        ImGui::Spacing();
        ImGui::Spacing();
        
        if(std::strlen(s_auth_error_message.data()) > 0)
        	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", s_auth_error_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        	s_auth_success = ::on_submit(connector, login_mode);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Don't have an account?"); 
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
        ImGui::Text("Sign up");
        if (ImGui::IsItemHovered()) 
        {
          ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); // Cambia il cursore in una manina
          ImVec2 min = ImGui::GetItemRectMin();
          ImVec2 max = ImGui::GetItemRectMax();
          min.y = max.y; 
          ImGui::GetWindowDrawList()->AddLine(min, max, ImGui::GetColorU32(ImGuiCol_Text));
        }
        if (ImGui::IsItemClicked()) 
        {
          login_mode = false;
          s_field_username.fill(0);
         	s_field_password.fill(0);
          s_auth_error_message.fill(0);
        }
        ImGui::PopStyleColor();
      }
	    ImGui::EndGroup();
	    ImGui::End();
			return s_auth_success;
		}
		
		bool render_signup(bool& login_mode, AuthServiceConnector& connector)
		{
			auto viewport = ImGui::GetMainViewport();
      auto work_size = viewport->WorkSize;
      auto work_pos = viewport->WorkPos;
      ImGui::SetNextWindowPos(work_pos);
      ImGui::SetNextWindowSize(work_size);
      
      constexpr auto window_flags = 
      	ImGuiWindowFlags_NoDecoration | 
      	ImGuiWindowFlags_NoMove | 
       	ImGuiWindowFlags_NoSavedSettings;
      
      constexpr auto INPUT_WIDTH = 250.0f;
      constexpr auto login_page_str = "Signup page";
      
      ImGui::Begin("SignupScreen", nullptr, window_flags);
      auto title_y = work_size.y * 0.20f;
      auto group_y = (work_size.y - 200.0f) * 0.5f;
      auto title_x = (work_size.x - ImGui::CalcTextSize(login_page_str).x) * 0.5f;
      ImGui::SetCursorPos(ImVec2(title_x, title_y));
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.5f, 1.0f, 1.0f));
      ImGui::Text("%s", login_page_str);
      ImGui::PopStyleColor();
      
      auto window_center_x = (work_size.x - INPUT_WIDTH) * 0.5f;
      ImGui::SetCursorPos(ImVec2(window_center_x, group_y));
      ImGui::BeginGroup();
      {
        ImGui::Text("Username");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##username", s_field_username.data(), max_len_username);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", s_field_password.data(), max_len_password, ImGuiInputTextFlags_Password);
        ImGui::Spacing();
        ImGui::TextDisabled("Password must contain:"); 
        ImGui::Indent(10.0f); 
        ImGui::BulletText("8-16 characters");
        ImGui::BulletText("At least one uppercase");
        ImGui::BulletText("At least one lowercase");
        ImGui::BulletText("At least one number");
        ImGui::BulletText("At least one special char: (!@#$%%^&*_-+=?)");
        ImGui::Unindent(10.0f);
        ImGui::Spacing();
        ImGui::Spacing();
        
        if(std::strlen(s_auth_error_message.data()) > 0)
        	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", s_auth_error_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        	s_auth_success = ::on_submit(connector, login_mode);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Text("Dou you already have an account?"); 
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
        ImGui::Text("Log in");
        if (ImGui::IsItemHovered()) 
        {
          ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
          ImVec2 min = ImGui::GetItemRectMin();
          ImVec2 max = ImGui::GetItemRectMax();
          min.y = max.y; 
          ImGui::GetWindowDrawList()->AddLine(min, max, ImGui::GetColorU32(ImGuiCol_Text));
        }
        if (ImGui::IsItemClicked()) 
        {
          login_mode = true;
          s_field_username.fill(0);
         	s_field_password.fill(0);
          s_auth_error_message.fill(0);
        }
        ImGui::PopStyleColor();
      }
	    ImGui::EndGroup();
	    ImGui::End();
			return s_auth_success;
		}
		
	} // auth_panel
} // gui