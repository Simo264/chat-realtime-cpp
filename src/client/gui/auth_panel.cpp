#include "auth_panel.hpp"
#include "imgui.h"

#include "grpcpp/support/status.h"
#include "../auth_service_connector.hpp"
#include "../../common.hpp"

#include <array>
#include <format>
#include <print>

static auto s_username = std::array<char, max_len_username>{};
static auto s_password = std::array<char, max_len_password>{};
static auto s_error_message = std::array<char, max_len_error_message>{};
static auto s_client_id = ClientID{ invalid_client_id };

using auth_service::AuthRequest;
using auth_service::AuthResponse;

// ========================================
// Private functions
// ========================================

static void on_submit(AuthServiceConnector& connector, bool login_mode, std::array<char, max_len_username>& out_username)
{
 	auto request = AuthRequest{};
	request.set_username(s_username.data());
	request.set_password(s_password.data());
	auto response = AuthResponse{};
	auto context = grpc::ClientContext{};
	
	auto status = grpc::Status{};
	if(login_mode)
		status = connector.CallRemoteLoginProcedure(request, response, context);		
	else
		status = connector.CallRemoteSignupProcedure(request, response, context);
	
	if(status.ok()) 
	{
    s_client_id = response.client_id();
    out_username = s_username;
    std::println("Authentication successful! Your client_id: {}", s_client_id);
    return; 
  }
	
	s_error_message.fill(0);
	switch (status.error_code())
	{
		case grpc::StatusCode::UNAUTHENTICATED:
			std::format_to_n(s_error_message.begin(), max_len_error_message, "Login failed: Invalid credentials.");
			break;
		case grpc::StatusCode::INVALID_ARGUMENT: 
			std::format_to_n(s_error_message.begin(), max_len_error_message, "Error: {}", status.error_message());
			break;
		case grpc::StatusCode::ALREADY_EXISTS: 
		std::format_to_n(s_error_message.begin(), max_len_error_message, "Username already in use. Choose another.");
			break;
		case grpc::StatusCode::UNAVAILABLE:
			std::format_to_n(s_error_message.begin(), max_len_error_message, "Server unreachable. Check the connection.");
	    break;
		case grpc::StatusCode::DEADLINE_EXCEEDED:
			std::format_to_n(s_error_message.begin(), max_len_error_message, "The server took too long to respond");
			break;
		default:
			std::format_to_n(s_error_message.begin(), max_len_error_message, "Unexpected error: {}", static_cast<int>(status.error_code()));
			break;
	}
	std::println("{}", s_error_message.data());
}

// ========================================
// Public interface
// ========================================

namespace gui
{
	namespace auth_panel
	{
		ClientID render_login(bool& login_mode, AuthServiceConnector& connector, std::array<char, max_len_username>& out_username)
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
        ImGui::InputText("##username", s_username.data(), max_len_username);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", s_password.data(), max_len_password, ImGuiInputTextFlags_Password);
        ImGui::Spacing();
        ImGui::Spacing();
        
        if(!s_error_message.empty())
        	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", s_error_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        	::on_submit(connector, login_mode, out_username);
        
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
          s_username.fill(0);
         	s_password.fill(0);
          s_error_message.fill(0);
        }
        ImGui::PopStyleColor();
      }
	    ImGui::EndGroup();
	    ImGui::End();
			return s_client_id;
		}
		
		ClientID render_signup(bool& login_mode, AuthServiceConnector& connector, std::array<char, max_len_username>& out_username)
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
        ImGui::InputText("##username", s_username.data(), max_len_username);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", s_password.data(), max_len_password, ImGuiInputTextFlags_Password);
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
        
        if(!s_error_message.empty())
        	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", s_error_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        	::on_submit(connector, login_mode, out_username);
        
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
          s_username.fill(0);
         	s_password.fill(0);
          s_error_message.fill(0);
        }
        ImGui::PopStyleColor();
      }
	    ImGui::EndGroup();
	    ImGui::End();
			return s_client_id;
		}
		
	} // auth_panel
} // gui