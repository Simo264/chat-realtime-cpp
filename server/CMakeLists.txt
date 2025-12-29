add_executable(chat_server 
	server.cpp
)

target_link_libraries(chat_server
  PRIVATE
  grpc++
  grpc
  protobuf::libprotobuf
)

target_compile_features(chat_server PRIVATE cxx_std_23)