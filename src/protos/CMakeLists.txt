# Raccoglie tutti i file .proto
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")

set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/sources")
file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# Liste per i file generati
set(PROTO_SRCS "")
set(PROTO_HDRS "")
set(GRPC_SRCS "")
set(GRPC_HDRS "")

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    list(APPEND PROTO_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc")
    list(APPEND PROTO_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h")
    list(APPEND GRPC_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
    list(APPEND GRPC_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h")
endforeach()

# Usa i target forniti da FetchContent
add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND $<TARGET_FILE:protobuf::protoc>
        --cpp_out=${PROTO_OUTPUT_DIR}
        --grpc_out=${PROTO_OUTPUT_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:grpc_cpp_plugin>
        -I${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILES}
    DEPENDS ${PROTO_FILES} protobuf::protoc grpc_cpp_plugin
    COMMENT "Generazione file protobuf e gRPC..."
    VERBATIM
)

# Libreria con i file generati
add_library(protos_lib STATIC
  ${PROTO_SRCS}
  ${GRPC_SRCS}
)

target_link_libraries(protos_lib PUBLIC
  grpc++
  protobuf::libprotobuf
)

target_include_directories(protos_lib PUBLIC
  ${PROTO_OUTPUT_DIR}
)
