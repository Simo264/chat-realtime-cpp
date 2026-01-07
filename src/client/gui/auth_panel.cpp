#include "auth_panel.hpp"
#include "imgui.h"

#include "grpcpp/support/status.h"
#include "../auth_service_connector.hpp"
#include "../../common.hpp"

#include <array>
#include <algorithm>
#include <print>
#include <string_view>

static auto username = std::array<char, max_len_username>{};
static auto password = std::array<char, max_len_password>{};
static auto auth_message = std::array<char, max_len_auth_message>{};
static auto client_id = ClientID{ invalid_client_id };

using auth_service::AuthRequest;
using auth_service::AuthResponse;

// ========================================
// Private functions
// ========================================

static void on_submit(AuthServiceConnector& connector, bool login_mode)
{
 	auto request = AuthRequest{};
	request.set_username(username.data());
	request.set_password(password.data());
	auto response = AuthResponse{};
	auto context = grpc::ClientContext{};
	auto status = grpc::Status{};
	
	auth_message.fill(0);
	if(login_mode)
		status = connector.LoginProcedure(request, response, context);		
	else
		status = connector.SignupProcedure(request, response, context);
	
	if(!status.ok())
	{		
		std::println("gRPC error {}: {} - {}", static_cast<int>(status.error_code()), status.error_message(), status.error_details());
		exit(EXIT_FAILURE);	
	}

	auto message = std::string_view{ response.auth_message() };
	if(!message.empty())
		std::println("{}", message);
	
	if(response.auth_success())
		client_id = response.client_id();
	else
		std::copy_n(message.begin(), max_len_auth_message, auth_message.begin());
}

// ========================================
// Public interface
// ========================================

namespace gui
{
	namespace auth_panel
	{
		ClientID render_login(bool& login_mode, AuthServiceConnector& connector)
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
        ImGui::InputText("##username", username.data(), max_len_username);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", password.data(), max_len_password, ImGuiInputTextFlags_Password);
        ImGui::Spacing();
        ImGui::Spacing();
        
        if(!auth_message.empty())
        	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", auth_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        	::on_submit(connector, login_mode);
        
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
          username.fill(0);
         	password.fill(0);
          auth_message.fill(0);
        }
        ImGui::PopStyleColor();
      }
	    ImGui::EndGroup();
	    ImGui::End();
			return client_id;
		}
		
		ClientID render_signup(bool& login_mode, AuthServiceConnector& connector)
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
        ImGui::InputText("##username", username.data(), max_len_username);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", password.data(), max_len_password, ImGuiInputTextFlags_Password);
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
        
        if(!auth_message.empty())
        	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", auth_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        	::on_submit(connector, login_mode);
        
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
          username.fill(0);
         	password.fill(0);
          auth_message.fill(0);
        }
        ImGui::PopStyleColor();
      }
	    ImGui::EndGroup();
	    ImGui::End();
			return client_id;
		}
		
	} // auth_panel
} // gui