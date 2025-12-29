add_executable(chat_client 
	client.cpp
)

target_link_libraries(chat_client
  PRIVATE
  grpc++
  grpc
  protobuf::libprotobuf
)

target_compile_features(chat_client PRIVATE cxx_std_23)