add_library(imgui STATIC
  imgui.cpp
  imgui_draw.cpp
  imgui_widgets.cpp
  imgui_tables.cpp
  imgui_demo.cpp
  imgui_impl_glfw.cpp
  imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(imgui 
	PUBLIC
	GLFW_INCLUDE_NONE
	IMGUI_IMPL_GLFW_DISABLE_X11
	IMGUI_IMPL_GLFW_DISABLE_NATIVE_ACCESS
	IMGUI_IMPL_GLFW_USE_WAYLAND
)
target_link_libraries(imgui PUBLIC glfw glad)