add_executable(chat_server 
	server.cpp
	services/auth_service_impl.cpp
	services/rooms_service_impl.cpp
)
target_link_libraries(chat_server PRIVATE 
	protos_lib 
	csv_parser
	grpc++_reflection 
)
target_compile_features(chat_server PRIVATE cxx_std_23)