cmake_minimum_required(VERSION 3.20)

project(
  chat_realtime_cpp
  VERSION 0.1.0
  DESCRIPTION "Distributed real-time chat in C++23 using gRPC and ImGui"
  LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external)
set(FETCHCONTENT_QUIET OFF)

# ==============================
# gRPC and Protobuffer
# ==============================
set(gRPC_INSTALL OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_NODE_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_PHP_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_RUBY_PLUGIN OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	grpc
  GIT_REPOSITORY https://github.com/grpc/grpc.git
  GIT_TAG v1.68.0  # Versione stabile più recente (dicembre 2024)
  GIT_SHALLOW TRUE
  GIT_SUBMODULES_RECURSE TRUE
  DOWNLOAD_PROGRESS ON
)
FetchContent_MakeAvailable(grpc)

# ==============================
# Glfw3
# ==============================
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE) 
FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git 
	GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

add_library(glad STATIC external/glad/glad/gl.c)
target_include_directories(glad
	PUBLIC
	${CMAKE_SOURCE_DIR}/external/glad
)

add_library(csv_parser INTERFACE)
target_include_directories(csv_parser INTERFACE 
  ${CMAKE_SOURCE_DIR}/external/csv_parser
)

# ==============================
# Subdirectories
# ==============================
add_subdirectory(external/imgui)
add_subdirectory(src/protos)
add_subdirectory(src/server)
add_subdirectory(src/client)