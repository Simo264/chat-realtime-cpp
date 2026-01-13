add_executable(chat_client 
	client.cpp
	globals.cpp
	auth_service_connector.cpp
	rooms_service_connector.cpp
	chat_service_connector.cpp
	gui/imgui_manager.cpp
	gui/auth_panel.cpp
	gui/rooms_panel.cpp
	gui/chat_panel.cpp
	${CMAKE_SOURCE_DIR}/external/glad/glad/gl.c
)

target_link_libraries(chat_client 
	PRIVATE 
	protos_lib
	glfw
	glad
	imgui
)

target_compile_features(chat_client PRIVATE cxx_std_23)