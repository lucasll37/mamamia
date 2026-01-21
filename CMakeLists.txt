cmake_minimum_required(VERSION 3.20)
project(MLInferenceSystem VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_WORKER "Build Worker Server" ON)
option(BUILD_CLIENT "Build C++ Client" ON)
option(ENABLE_GPU "Enable GPU support (CUDA)" OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Conan-generated dependencies
find_package(Threads REQUIRED)
find_package(protobuf REQUIRED)
find_package(gRPC REQUIRED)

message(STATUS "Found Protobuf via Conan")
message(STATUS "Found gRPC via Conan")

# Get protoc executable
get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc LOCATION)
message(STATUS "Found protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")

# Get grpc_cpp_plugin
get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin LOCATION)
message(STATUS "Found grpc_cpp_plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# Generate protobuf and gRPC files
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

set(PROTO_FILES
    ${PROTO_DIR}/common.proto
    ${PROTO_DIR}/worker.proto
)

set(GENERATED_PROTO_SRCS)
set(GENERATED_PROTO_HDRS)
set(GENERATED_GRPC_SRCS)
set(GENERATED_GRPC_HDRS)

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    list(APPEND GENERATED_PROTO_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc")
    list(APPEND GENERATED_PROTO_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h")
    list(APPEND GENERATED_GRPC_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
    list(APPEND GENERATED_GRPC_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h")
    
    add_custom_command(
        OUTPUT "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc"
               "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h"
               "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc"
               "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h"
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --cpp_out=${PROTO_OUTPUT_DIR}
             --grpc_out=${PROTO_OUTPUT_DIR}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
             -I${PROTO_DIR}
             ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating protobuf and gRPC files for ${PROTO_NAME}"
    )
endforeach()

# Create proto library
add_library(proto_lib STATIC
    ${GENERATED_PROTO_SRCS}
    ${GENERATED_GRPC_SRCS}
)

target_include_directories(proto_lib PUBLIC
    ${PROTO_OUTPUT_DIR}
)

target_link_libraries(proto_lib PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
)

# Compiler warnings
add_compile_options(
    -Wall -Wextra -Wpedantic
    # $<$<CONFIG:Debug>:-g -O0>
    # $<$<CONFIG:Release>:-O3>
)

# Build Worker
if(BUILD_WORKER)
    add_subdirectory(cpp/worker)
endif()

# # Build Client
# if(BUILD_CLIENT)
#     add_subdirectory(cpp/client)
# endif()

# # Print configuration summary
# message(STATUS "")
# message(STATUS "ML Inference System Configuration:")
# message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
# message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
# message(STATUS "  Build worker: ${BUILD_WORKER}")
# message(STATUS "  Build client: ${BUILD_CLIENT}")
# message(STATUS "  GPU support: ${ENABLE_GPU}")
# message(STATUS "")