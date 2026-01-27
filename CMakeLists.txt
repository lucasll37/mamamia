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


# ============================================
# MIXR Setup
# ============================================
set(MIXR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr")
set(MIXR_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr-3rdparty")

if(EXISTS "${MIXR_ROOT}/lib" AND EXISTS "${MIXR_3RDPARTY}/lib")
    message(STATUS "Found MIXR installation")
    message(STATUS "  MIXR Root: ${MIXR_ROOT}")
    message(STATUS "  MIXR 3rd Party: ${MIXR_3RDPARTY}")
    
    # Find all MIXR libraries
    file(GLOB MIXR_LIBS "${MIXR_ROOT}/lib/*.a")
    file(GLOB MIXR_3RD_LIBS "${MIXR_3RDPARTY}/lib/*.a")
    
    # Create MIXR target
    add_library(mixr INTERFACE)
    
    target_include_directories(mixr INTERFACE
        ${MIXR_ROOT}/include
        ${MIXR_3RDPARTY}/include
    )
    
    target_link_directories(mixr INTERFACE
        ${MIXR_ROOT}/lib
        ${MIXR_3RDPARTY}/lib
    )
    
    target_link_libraries(mixr INTERFACE
        ${MIXR_LIBS}
        ${MIXR_3RD_LIBS}
    )
    
    # Find system dependencies for MIXR
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
    find_package(Freetype REQUIRED)
    
    target_link_libraries(mixr INTERFACE
        OpenGL::GL
        GLUT::GLUT
        Freetype::Freetype
        ftgl
        fontconfig
        X11
        Xi
        dl
        pthread
        m
    )
    
    set(MIXR_FOUND TRUE)
    message(STATUS "MIXR target created - ready to use")
else()
    message(STATUS "MIXR not found (install with: ./scripts/install_mixr.sh)")
    set(MIXR_FOUND FALSE)
endif()

# ============================================
# ONNX Runtime Detection
# ============================================

# Try to find ONNX Runtime in system paths or custom location
set(ONNXRUNTIME_ROOT_DIR "" CACHE PATH "ONNX Runtime installation directory")

if(ONNXRUNTIME_ROOT_DIR)
    message(STATUS "Looking for ONNX Runtime in: ${ONNXRUNTIME_ROOT_DIR}")
    set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include")
    set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_DIR}/lib")
else()
    # Try standard system paths
    find_path(ONNXRUNTIME_INCLUDE_DIR
        NAMES onnxruntime_cxx_api.h
        PATHS
            /usr/local/include
            /usr/include
            /opt/onnxruntime/include
    )
    
    find_library(ONNXRUNTIME_LIBRARY
        NAMES onnxruntime
        PATHS
            /usr/local/lib
            /usr/lib
            /opt/onnxruntime/lib
    )
    
    if(ONNXRUNTIME_LIBRARY)
        get_filename_component(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_LIBRARY}" DIRECTORY)
    endif()
endif()

if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIB_DIR)
    message(STATUS "Found ONNX Runtime:")
    message(STATUS "  Include: ${ONNXRUNTIME_INCLUDE_DIR}")
    message(STATUS "  Library: ${ONNXRUNTIME_LIB_DIR}")
    
    add_library(onnxruntime SHARED IMPORTED)
    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.so"
        INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
    )
    
    set(ONNXRUNTIME_FOUND TRUE)
else()
    message(WARNING "ONNX Runtime not found!")
    message(WARNING "The worker will be built without ONNX support (mock inference only)")
    message(WARNING "To enable ONNX Runtime:")
    message(WARNING "  1. Download from: https://github.com/microsoft/onnxruntime/releases")
    message(WARNING "  2. Extract to /opt/onnxruntime or set ONNXRUNTIME_ROOT_DIR")
    message(WARNING "  3. Re-run cmake")
    set(ONNXRUNTIME_FOUND FALSE)
endif()

# Get protoc executable
get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
if(NOT PROTOBUF_PROTOC_EXECUTABLE)
    get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)
endif()
message(STATUS "Found protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")

# Get grpc_cpp_plugin
get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
    get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
endif()
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
        VERBATIM
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
)

# Build Worker
if(BUILD_WORKER)
    add_subdirectory(cpp/worker)
endif()

# Build Client
if(BUILD_CLIENT)
    add_subdirectory(cpp/client)
endif()


# cmake_minimum_required(VERSION 3.20)
# project(MLInferenceSystem VERSION 1.0.0 LANGUAGES CXX)

# # Set C++ standard
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# # Export compile commands for tooling
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# # Build type
# if(NOT CMAKE_BUILD_TYPE)
#     set(CMAKE_BUILD_TYPE Release)
# endif()

# # Options
# option(BUILD_WORKER "Build Worker Server" ON)
# option(BUILD_CLIENT "Build C++ Client" ON)
# option(ENABLE_GPU "Enable GPU support (CUDA)" OFF)

# # Output directories
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# # Find Conan-generated dependencies
# find_package(Threads REQUIRED)
# find_package(protobuf REQUIRED)
# find_package(gRPC REQUIRED)

# message(STATUS "Found Protobuf via Conan")
# message(STATUS "Found gRPC via Conan")

# # Get protoc executable - CORREÇÃO AQUI
# get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
# if(NOT PROTOBUF_PROTOC_EXECUTABLE)
#     get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)
# endif()
# message(STATUS "Found protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")

# # Get grpc_cpp_plugin - CORREÇÃO AQUI
# get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
# if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
#     get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
# endif()
# message(STATUS "Found grpc_cpp_plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# # Generate protobuf and gRPC files
# set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
# set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
# file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# set(PROTO_FILES
#     ${PROTO_DIR}/common.proto
#     ${PROTO_DIR}/worker.proto
# )

# set(GENERATED_PROTO_SRCS)
# set(GENERATED_PROTO_HDRS)
# set(GENERATED_GRPC_SRCS)
# set(GENERATED_GRPC_HDRS)

# foreach(PROTO_FILE ${PROTO_FILES})
#     get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
#     list(APPEND GENERATED_PROTO_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc")
#     list(APPEND GENERATED_PROTO_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h")
#     list(APPEND GENERATED_GRPC_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
#     list(APPEND GENERATED_GRPC_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h")
    
#     # CORREÇÃO: Usar variáveis em vez de generator expressions
#     add_custom_command(
#         OUTPUT "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc"
#                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h"
#                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc"
#                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h"
#         COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
#         ARGS --cpp_out=${PROTO_OUTPUT_DIR}
#              --grpc_out=${PROTO_OUTPUT_DIR}
#              --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
#              -I${PROTO_DIR}
#              ${PROTO_FILE}
#         DEPENDS ${PROTO_FILE}
#         COMMENT "Generating protobuf and gRPC files for ${PROTO_NAME}"
#         VERBATIM
#     )
# endforeach()

# # Create proto library
# add_library(proto_lib STATIC
#     ${GENERATED_PROTO_SRCS}
#     ${GENERATED_GRPC_SRCS}
# )

# target_include_directories(proto_lib PUBLIC
#     ${PROTO_OUTPUT_DIR}
# )

# target_link_libraries(proto_lib PUBLIC
#     protobuf::libprotobuf
#     gRPC::grpc++
#     gRPC::grpc++_reflection
# )

# # Compiler warnings
# add_compile_options(
#     -Wall -Wextra -Wpedantic
# )

# # Build Worker
# if(BUILD_WORKER)
#     add_subdirectory(cpp/worker)
# endif()

# # Build Client
# if(BUILD_CLIENT)
#     add_subdirectory(cpp/client)
# endif()