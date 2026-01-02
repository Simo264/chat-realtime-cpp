add_executable(chat_client 
	client.cpp
	auth_service_connector.cpp
	GUI/imgui_manager.cpp
	GUI/auth_panel.cpp
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