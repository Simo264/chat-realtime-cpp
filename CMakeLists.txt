cmake_minimum_required(VERSION 3.20)

project(
  chat_realtime_cpp
  VERSION 0.1.0
  DESCRIPTION "Distributed real-time chat in C++23 using gRPC and ImGui"
  LANGUAGES CXX
)

# ==============================
# C++ settings
# ==============================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================
# Output directories
# ==============================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ==============================
# Dependencies
# ==============================
# Qui in futuro potrai aggiungere:
# - gRPC
# - Protobuf
# - OpenGL / GLFW / SDL
# - ImGui

# ==============================
# Subdirectories
# ==============================
add_subdirectory(server)
add_subdirectory(client)