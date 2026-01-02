#include "auth_panel.hpp"
#include "imgui.h"

#include "../auth_service_connector.hpp"

#include <array>
#include <algorithm>
#include <print>
#include <string_view>

static auto username = std::array<char, 32>{};
static auto password = std::array<char, 32>{};
static auto auth_message = std::array<char, 128>{};

using auth_service::AuthRequest;
using auth_service::AuthResponse;

namespace gui
{
	namespace auth_panel
	{
		void render_login(bool& login_mode, AuthServiceConnector& auth_service_connector)
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
      
      ImGui::Begin("LoginScreen", nullptr, window_flags);
      // Posizione del titolo (al 20% dell'altezza dello schermo)
      auto title_y = work_size.y * 0.20f;
      // Posizione del gruppo input (centrato verticalmente)
      auto group_y = (work_size.y - 200.0f) * 0.5f;
      constexpr auto login_page_str = "Login page";
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
        ImGui::InputText("##username", username.data(), username.size()-1);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", password.data(), password.size()-1, ImGuiInputTextFlags_Password);
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        if(!auth_message.empty())
        	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", auth_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        {
	        auto request = AuthRequest{};
		      request.set_username(username.data());
		      request.set_password(password.data());
		      auto response = AuthResponse{};
		      auto context = grpc::ClientContext{};
		      auto status = auth_service_connector.LoginProcedure(request, response, context);
					if(status.ok())
					{
						auto message = std::string_view{ response.auth_message() };
						std::println("{}", message);
						if(!response.auth_success())
						{
							auth_message.fill(0);
							std::copy_n(message.begin(), auth_message.size() - 1, auth_message.begin());
						}
					}
					else
					{
						std::println("gRPC error {}: {} - {}", static_cast<int>(status.error_code()), status.error_message(), status.error_details());
					 	exit(EXIT_FAILURE);	
					}
					
       		username.fill(0);
         	password.fill(0);
        }
        
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
		}
		
		void render_signup(bool& login_mode, AuthServiceConnector& auth_service_connector)
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
      
      ImGui::Begin("SignupScreen", nullptr, window_flags);
      
      // Posizione del titolo (al 20% dell'altezza dello schermo)
      auto title_y = work_size.y * 0.20f;
      // Posizione del gruppo input (centrato verticalmente)
      auto group_y = (work_size.y - 200.0f) * 0.5f;
      constexpr auto login_page_str = "Signup page";
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
        ImGui::InputText("##username", username.data(), username.size()-1);
        ImGui::Spacing();
        ImGui::Text("Password");
        ImGui::SetNextItemWidth(INPUT_WIDTH);
        ImGui::InputText("##password", password.data(), password.size()-1, ImGuiInputTextFlags_Password);
       
        ImGui::Spacing();
        ImGui::TextDisabled("Password must contain:"); // Intestazione in grigio
        
        ImGui::Indent(10.0f); 
        ImGui::BulletText("8-16 characters");
        ImGui::BulletText("At least one uppercase");
        ImGui::BulletText("At least one lowercase");
        ImGui::BulletText("At least one number");
        ImGui::BulletText("At least one special char: (!@#$%%^&*_-+=?)");
        ImGui::Unindent(10.0f);
        ImGui::Spacing();
        ImGui::Spacing();
        
        // if(!error_message.empty())
        // 	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", error_message.data());
        
        if (ImGui::Button("Submit", ImVec2(INPUT_WIDTH, 40)))
        {
        	
        }
        
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
		}
		
	} // auth_panel
} // gui