cmake_minimum_required(VERSION 3.20)
project(MLInferenceSystem VERSION 1.0.0 LANGUAGES CXX)

# ============================================
# CRITICAL: Block ALL external paths
# CMake was finding Anaconda's protoc instead of Conan's
# ============================================
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY OFF)
set(CMAKE_FIND_USE_PACKAGE_REGISTRY OFF)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands
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

# ============================================
# Find Conan packages ONLY (no system search)
# ============================================
find_package(Threads REQUIRED)
find_package(protobuf REQUIRED CONFIG NO_DEFAULT_PATH)
find_package(gRPC REQUIRED CONFIG NO_DEFAULT_PATH)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Conan Package Configuration")
message(STATUS "========================================")

# ============================================
# Get Protobuf tools from Conan MANUALLY
# Don't trust target properties - search in Conan paths directly
# ============================================

# Get Conan package path for protobuf
get_target_property(_protobuf_include protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
get_filename_component(CONAN_PROTOBUF_ROOT "${_protobuf_include}" DIRECTORY)

message(STATUS "Protobuf:")
message(STATUS "  Version: ${protobuf_VERSION}")
message(STATUS "  Root: ${CONAN_PROTOBUF_ROOT}")

# Search for protoc in Conan protobuf package
find_program(Protobuf_PROTOC_EXECUTABLE
    NAMES protoc
    PATHS ${CONAN_PROTOBUF_ROOT}/bin
    NO_DEFAULT_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
)

if(NOT Protobuf_PROTOC_EXECUTABLE)
    message(FATAL_ERROR "Could not find protoc in Conan protobuf package!\n  Searched in: ${CONAN_PROTOBUF_ROOT}/bin")
endif()

if(NOT EXISTS "${Protobuf_PROTOC_EXECUTABLE}")
    message(FATAL_ERROR "protoc not found at: ${Protobuf_PROTOC_EXECUTABLE}")
endif()

# Verify it's from Conan (not Anaconda or system)
if(Protobuf_PROTOC_EXECUTABLE MATCHES "anaconda")
    message(FATAL_ERROR "ERROR: Found Anaconda's protoc instead of Conan's!\n  Path: ${Protobuf_PROTOC_EXECUTABLE}")
endif()

if(NOT Protobuf_PROTOC_EXECUTABLE MATCHES "\.conan2")
    message(FATAL_ERROR "ERROR: protoc is not from Conan!\n  Path: ${Protobuf_PROTOC_EXECUTABLE}")
endif()

message(STATUS "  Protoc: ${Protobuf_PROTOC_EXECUTABLE}")

# ============================================
# Get gRPC plugin from Conan MANUALLY
# ============================================

# Get Conan package path for gRPC
get_target_property(_grpc_lib gRPC::grpc++ INTERFACE_LINK_LIBRARIES)
string(REGEX REPLACE ".*gRPC::grpc\\+\\+.*" "" _dummy "${_grpc_lib}")
get_target_property(_grpc_include gRPC::grpc++ INTERFACE_INCLUDE_DIRECTORIES)
get_filename_component(CONAN_GRPC_ROOT "${_grpc_include}" DIRECTORY)

message(STATUS "gRPC:")
message(STATUS "  Version: ${gRPC_VERSION}")
message(STATUS "  Root: ${CONAN_GRPC_ROOT}")

# Search for grpc_cpp_plugin in Conan gRPC package
find_program(gRPC_CPP_PLUGIN_EXECUTABLE
    NAMES grpc_cpp_plugin
    PATHS ${CONAN_GRPC_ROOT}/bin
    NO_DEFAULT_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
)

if(NOT gRPC_CPP_PLUGIN_EXECUTABLE)
    message(FATAL_ERROR "Could not find grpc_cpp_plugin in Conan gRPC package!\n  Searched in: ${CONAN_GRPC_ROOT}/bin")
endif()

if(NOT EXISTS "${gRPC_CPP_PLUGIN_EXECUTABLE}")
    message(FATAL_ERROR "grpc_cpp_plugin not found at: ${gRPC_CPP_PLUGIN_EXECUTABLE}")
endif()

# Verify it's from Conan
if(gRPC_CPP_PLUGIN_EXECUTABLE MATCHES "anaconda")
    message(FATAL_ERROR "ERROR: Found Anaconda's grpc_cpp_plugin instead of Conan's!\n  Path: ${gRPC_CPP_PLUGIN_EXECUTABLE}")
endif()

if(NOT gRPC_CPP_PLUGIN_EXECUTABLE MATCHES "\.conan2")
    message(FATAL_ERROR "ERROR: grpc_cpp_plugin is not from Conan!\n  Path: ${gRPC_CPP_PLUGIN_EXECUTABLE}")
endif()

message(STATUS "  Plugin: ${gRPC_CPP_PLUGIN_EXECUTABLE}")
message(STATUS "========================================")
message(STATUS "")

# ============================================
# MIXR Setup (Optional)
# ============================================
set(MIXR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr")
set(MIXR_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr-3rdparty")

if(EXISTS "${MIXR_ROOT}/lib" AND EXISTS "${MIXR_3RDPARTY}/lib")
    file(GLOB MIXR_LIBS "${MIXR_ROOT}/lib/*.a")
    file(GLOB MIXR_3RD_LIBS "${MIXR_3RDPARTY}/lib/*.a")
    
    add_library(mixr INTERFACE)
    target_include_directories(mixr INTERFACE ${MIXR_ROOT}/include ${MIXR_3RDPARTY}/include)
    target_link_directories(mixr INTERFACE ${MIXR_ROOT}/lib ${MIXR_3RDPARTY}/lib)
    target_link_libraries(mixr INTERFACE ${MIXR_LIBS} ${MIXR_3RD_LIBS})
    
    find_package(OpenGL QUIET)
    find_package(GLUT QUIET)
    find_package(Freetype QUIET)
    
    if(OpenGL_FOUND AND GLUT_FOUND AND Freetype_FOUND)
        target_link_libraries(mixr INTERFACE OpenGL::GL GLUT::GLUT Freetype::Freetype ftgl fontconfig X11 Xi dl pthread m)
        set(MIXR_FOUND TRUE)
        message(STATUS "MIXR: Enabled")
    else()
        set(MIXR_FOUND FALSE)
        message(STATUS "MIXR: Disabled (missing OpenGL)")
    endif()
else()
    set(MIXR_FOUND FALSE)
    message(STATUS "MIXR: Not found")
endif()

# ============================================
# ONNX Runtime
# ============================================
set(ONNXRUNTIME_ROOT_DIR "" CACHE PATH "ONNX Runtime installation directory")

if(ONNXRUNTIME_ROOT_DIR)
    set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include")
    set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_DIR}/lib")
else()
    find_path(ONNXRUNTIME_INCLUDE_DIR NAMES onnxruntime_cxx_api.h PATHS /usr/local/include /usr/include /opt/onnxruntime/include)
    find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime PATHS /usr/local/lib /usr/lib /opt/onnxruntime/lib)
    if(ONNXRUNTIME_LIBRARY)
        get_filename_component(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_LIBRARY}" DIRECTORY)
    endif()
endif()

if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIB_DIR)
    add_library(onnxruntime SHARED IMPORTED)
    set_target_properties(onnxruntime PROPERTIES
        IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.so"
        INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
    )
    set(ONNXRUNTIME_FOUND TRUE)
    message(STATUS "ONNX Runtime: Found")
else()
    set(ONNXRUNTIME_FOUND FALSE)
    message(STATUS "ONNX Runtime: Not found")
endif()

# ============================================
# Generate Protobuf/gRPC Code
# ============================================
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

message(STATUS "Protobuf Code Generation:")
message(STATUS "  Input:  ${PROTO_DIR}")
message(STATUS "  Output: ${PROTO_OUTPUT_DIR}")
message(STATUS "")

set(PROTO_FILES
    ${PROTO_DIR}/common.proto
    ${PROTO_DIR}/worker.proto
)

set(GENERATED_SOURCES)

foreach(proto_file ${PROTO_FILES})
    get_filename_component(proto_name ${proto_file} NAME_WE)
    
    set(proto_src "${PROTO_OUTPUT_DIR}/${proto_name}.pb.cc")
    set(proto_hdr "${PROTO_OUTPUT_DIR}/${proto_name}.pb.h")
    set(grpc_src "${PROTO_OUTPUT_DIR}/${proto_name}.grpc.pb.cc")
    set(grpc_hdr "${PROTO_OUTPUT_DIR}/${proto_name}.grpc.pb.h")
    
    list(APPEND GENERATED_SOURCES ${proto_src} ${grpc_src})
    
    message(STATUS "  Generating: ${proto_name}.proto")
    
    add_custom_command(
        OUTPUT ${proto_src} ${proto_hdr} ${grpc_src} ${grpc_hdr}
        COMMAND "${Protobuf_PROTOC_EXECUTABLE}"
            --cpp_out=${PROTO_OUTPUT_DIR}
            --grpc_out=${PROTO_OUTPUT_DIR}
            --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}"
            -I${PROTO_DIR}
            ${proto_file}
        DEPENDS ${proto_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating C++ from ${proto_name}.proto"
        VERBATIM
    )
endforeach()

message(STATUS "")

# Create proto library
add_library(proto_lib STATIC ${GENERATED_SOURCES})
set_source_files_properties(${GENERATED_SOURCES} PROPERTIES GENERATED TRUE)
target_include_directories(proto_lib PUBLIC ${PROTO_OUTPUT_DIR})
target_link_libraries(proto_lib PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
)

# Force generation before compilation
add_custom_target(generate_protos ALL DEPENDS ${GENERATED_SOURCES})
add_dependencies(proto_lib generate_protos)

# Compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Build components
if(BUILD_WORKER)
    add_subdirectory(cpp/worker)
endif()

if(BUILD_CLIENT)
    add_subdirectory(cpp/client)
endif()

# Summary
message(STATUS "========================================")
message(STATUS "Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Components:")
message(STATUS "  Worker:     ${BUILD_WORKER}")
message(STATUS "  Client:     ${BUILD_CLIENT}")
message(STATUS "")
message(STATUS "Dependencies (ALL FROM CONAN):")
message(STATUS "  Protobuf:   ${protobuf_VERSION}")
message(STATUS "  gRPC:       ${gRPC_VERSION}")
message(STATUS "  ONNX:       ${ONNXRUNTIME_FOUND}")
message(STATUS "  MIXR:       ${MIXR_FOUND}")
message(STATUS "========================================")
message(STATUS "")

# cmake_minimum_required(VERSION 3.20)
# project(MLInferenceSystem VERSION 1.0.0 LANGUAGES CXX)

# # ============================================
# # CRITICAL: Block system package search
# # Force CMake to ONLY use Conan packages
# # ============================================
# set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
# set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY OFF)
# set(CMAKE_FIND_USE_PACKAGE_REGISTRY OFF)

# # Only search in Conan paths
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# # Set C++ standard
# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

# # Export compile commands
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

# # ============================================
# # Find Conan packages - NO_DEFAULT_PATH forces Conan only
# # ============================================
# find_package(Threads REQUIRED)

# # CRITICAL: NO_DEFAULT_PATH prevents system package lookup
# find_package(protobuf REQUIRED CONFIG NO_DEFAULT_PATH)
# find_package(gRPC REQUIRED CONFIG NO_DEFAULT_PATH)

# message(STATUS "")
# message(STATUS "========================================")
# message(STATUS "Conan Packages Found:")
# message(STATUS "========================================")
# message(STATUS "Protobuf: ${protobuf_VERSION}")
# message(STATUS "  Dir: ${protobuf_DIR}")

# # Get protobuf include directories
# get_target_property(_protobuf_include protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
# message(STATUS "  Include: ${_protobuf_include}")

# # Get protoc executable from Conan
# get_target_property(_protoc_exe protobuf::protoc IMPORTED_LOCATION_RELEASE)
# if(NOT _protoc_exe)
#     get_target_property(_protoc_exe protobuf::protoc IMPORTED_LOCATION_RELWITHDEBINFO)
# endif()
# if(NOT _protoc_exe)
#     get_target_property(_protoc_exe protobuf::protoc IMPORTED_LOCATION_DEBUG)
# endif()
# if(NOT _protoc_exe)
#     get_target_property(_protoc_exe protobuf::protoc IMPORTED_LOCATION)
# endif()

# if(NOT _protoc_exe OR NOT EXISTS "${_protoc_exe}")
#     message(FATAL_ERROR "protoc not found in Conan package!")
# endif()

# # Verify it's from Conan
# if(NOT _protoc_exe MATCHES "\.conan2")
#     message(FATAL_ERROR "ERROR: protoc is from system, not Conan!\n  Path: ${_protoc_exe}")
# endif()

# set(Protobuf_PROTOC_EXECUTABLE "${_protoc_exe}" CACHE FILEPATH "Protoc from Conan" FORCE)
# message(STATUS "  Protoc: ${Protobuf_PROTOC_EXECUTABLE}")

# message(STATUS "gRPC: ${gRPC_VERSION}")
# message(STATUS "  Dir: ${gRPC_DIR}")

# # Get grpc_cpp_plugin from Conan
# get_target_property(_grpc_plugin gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
# if(NOT _grpc_plugin)
#     get_target_property(_grpc_plugin gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELWITHDEBINFO)
# endif()
# if(NOT _grpc_plugin)
#     get_target_property(_grpc_plugin gRPC::grpc_cpp_plugin IMPORTED_LOCATION_DEBUG)
# endif()
# if(NOT _grpc_plugin)
#     get_target_property(_grpc_plugin gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
# endif()

# if(NOT _grpc_plugin OR NOT EXISTS "${_grpc_plugin}")
#     message(FATAL_ERROR "grpc_cpp_plugin not found in Conan package!")
# endif()

# # Verify it's from Conan
# if(NOT _grpc_plugin MATCHES "\.conan2")
#     message(FATAL_ERROR "ERROR: grpc_cpp_plugin is from system, not Conan!\n  Path: ${_grpc_plugin}")
# endif()

# set(gRPC_CPP_PLUGIN_EXECUTABLE "${_grpc_plugin}" CACHE FILEPATH "gRPC plugin from Conan" FORCE)
# message(STATUS "  Plugin: ${gRPC_CPP_PLUGIN_EXECUTABLE}")
# message(STATUS "========================================")
# message(STATUS "")

# # ============================================
# # MIXR Setup (Optional)
# # ============================================
# set(MIXR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr")
# set(MIXR_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr-3rdparty")

# if(EXISTS "${MIXR_ROOT}/lib" AND EXISTS "${MIXR_3RDPARTY}/lib")
#     message(STATUS "MIXR: Found")
    
#     file(GLOB MIXR_LIBS "${MIXR_ROOT}/lib/*.a")
#     file(GLOB MIXR_3RD_LIBS "${MIXR_3RDPARTY}/lib/*.a")
    
#     add_library(mixr INTERFACE)
    
#     target_include_directories(mixr INTERFACE
#         ${MIXR_ROOT}/include
#         ${MIXR_3RDPARTY}/include
#     )
    
#     target_link_directories(mixr INTERFACE
#         ${MIXR_ROOT}/lib
#         ${MIXR_3RDPARTY}/lib
#     )
    
#     target_link_libraries(mixr INTERFACE
#         ${MIXR_LIBS}
#         ${MIXR_3RD_LIBS}
#     )
    
#     find_package(OpenGL QUIET)
#     find_package(GLUT QUIET)
#     find_package(Freetype QUIET)
    
#     if(OpenGL_FOUND AND GLUT_FOUND AND Freetype_FOUND)
#         target_link_libraries(mixr INTERFACE
#             OpenGL::GL GLUT::GLUT Freetype::Freetype
#             ftgl fontconfig X11 Xi dl pthread m
#         )
#         set(MIXR_FOUND TRUE)
#         message(STATUS "MIXR: Enabled with OpenGL")
#     else()
#         message(STATUS "MIXR: Disabled (missing OpenGL)")
#         set(MIXR_FOUND FALSE)
#     endif()
# else()
#     message(STATUS "MIXR: Not found")
#     set(MIXR_FOUND FALSE)
# endif()

# # ============================================
# # ONNX Runtime
# # ============================================
# set(ONNXRUNTIME_ROOT_DIR "" CACHE PATH "ONNX Runtime installation directory")

# if(ONNXRUNTIME_ROOT_DIR)
#     set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include")
#     set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_DIR}/lib")
# else()
#     find_path(ONNXRUNTIME_INCLUDE_DIR
#         NAMES onnxruntime_cxx_api.h
#         PATHS /usr/local/include /usr/include /opt/onnxruntime/include
#     )
    
#     find_library(ONNXRUNTIME_LIBRARY
#         NAMES onnxruntime
#         PATHS /usr/local/lib /usr/lib /opt/onnxruntime/lib
#     )
    
#     if(ONNXRUNTIME_LIBRARY)
#         get_filename_component(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_LIBRARY}" DIRECTORY)
#     endif()
# endif()

# if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIB_DIR)
#     add_library(onnxruntime SHARED IMPORTED)
#     set_target_properties(onnxruntime PROPERTIES
#         IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.so"
#         INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
#     )
#     set(ONNXRUNTIME_FOUND TRUE)
#     message(STATUS "ONNX Runtime: Found")
# else()
#     set(ONNXRUNTIME_FOUND FALSE)
#     message(STATUS "ONNX Runtime: Not found (using mock)")
# endif()

# # ============================================
# # Generate Protobuf/gRPC Files
# # ============================================
# set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
# set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
# file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# message(STATUS "")
# message(STATUS "Protobuf Code Generation:")
# message(STATUS "  Input:     ${PROTO_DIR}")
# message(STATUS "  Output:    ${PROTO_OUTPUT_DIR}")
# message(STATUS "  Protoc:    ${Protobuf_PROTOC_EXECUTABLE}")
# message(STATUS "  Plugin:    ${gRPC_CPP_PLUGIN_EXECUTABLE}")
# message(STATUS "")

# set(PROTO_FILES
#     ${PROTO_DIR}/common.proto
#     ${PROTO_DIR}/worker.proto
# )

# set(GENERATED_SOURCES)

# foreach(proto_file ${PROTO_FILES})
#     get_filename_component(proto_name ${proto_file} NAME_WE)
    
#     set(proto_src "${PROTO_OUTPUT_DIR}/${proto_name}.pb.cc")
#     set(proto_hdr "${PROTO_OUTPUT_DIR}/${proto_name}.pb.h")
#     set(grpc_src "${PROTO_OUTPUT_DIR}/${proto_name}.grpc.pb.cc")
#     set(grpc_hdr "${PROTO_OUTPUT_DIR}/${proto_name}.grpc.pb.h")
    
#     list(APPEND GENERATED_SOURCES ${proto_src} ${grpc_src})
    
#     message(STATUS "  Generating: ${proto_name}.proto")
    
#     add_custom_command(
#         OUTPUT
#             ${proto_src}
#             ${proto_hdr}
#             ${grpc_src}
#             ${grpc_hdr}
#         COMMAND
#             "${Protobuf_PROTOC_EXECUTABLE}"
#             --cpp_out=${PROTO_OUTPUT_DIR}
#             --grpc_out=${PROTO_OUTPUT_DIR}
#             --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}"
#             -I${PROTO_DIR}
#             ${proto_file}
#         DEPENDS
#             ${proto_file}
#         WORKING_DIRECTORY
#             ${CMAKE_CURRENT_SOURCE_DIR}
#         COMMENT
#             "Generating C++ code from ${proto_name}.proto"
#         VERBATIM
#     )
# endforeach()

# # Create proto library
# add_library(proto_lib STATIC ${GENERATED_SOURCES})

# set_source_files_properties(${GENERATED_SOURCES} PROPERTIES GENERATED TRUE)

# target_include_directories(proto_lib PUBLIC ${PROTO_OUTPUT_DIR})

# # CRITICAL: Link ONLY Conan libraries
# target_link_libraries(proto_lib PUBLIC
#     protobuf::libprotobuf
#     gRPC::grpc++
#     gRPC::grpc++_reflection
# )

# # Force generation
# add_custom_target(generate_protos ALL DEPENDS ${GENERATED_SOURCES})
# add_dependencies(proto_lib generate_protos)

# # Compiler warnings
# add_compile_options(-Wall -Wextra -Wpedantic)

# # Build components
# if(BUILD_WORKER)
#     add_subdirectory(cpp/worker)
# endif()

# if(BUILD_CLIENT)
#     add_subdirectory(cpp/client)
# endif()

# # Summary
# message(STATUS "")
# message(STATUS "========================================")
# message(STATUS "Build Configuration")
# message(STATUS "========================================")
# message(STATUS "Build Type:   ${CMAKE_BUILD_TYPE}")
# message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
# message(STATUS "")
# message(STATUS "Components:")
# message(STATUS "  Worker:     ${BUILD_WORKER}")
# message(STATUS "  Client:     ${BUILD_CLIENT}")
# message(STATUS "")
# message(STATUS "Dependencies:")
# message(STATUS "  Protobuf:   ${protobuf_VERSION} (Conan)")
# message(STATUS "  gRPC:       ${gRPC_VERSION} (Conan)")
# message(STATUS "  ONNX:       ${ONNXRUNTIME_FOUND}")
# message(STATUS "  MIXR:       ${MIXR_FOUND}")
# message(STATUS "========================================")
# message(STATUS "")

# # cmake_minimum_required(VERSION 3.20)
# # project(MLInferenceSystem VERSION 1.0.0 LANGUAGES CXX)

# # # Set C++ standard
# # set(CMAKE_CXX_STANDARD 17)
# # set(CMAKE_CXX_STANDARD_REQUIRED ON)
# # set(CMAKE_CXX_EXTENSIONS OFF)

# # # Export compile commands for tooling
# # set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# # # Build type
# # if(NOT CMAKE_BUILD_TYPE)
# #     set(CMAKE_BUILD_TYPE Release)
# # endif()

# # # Options
# # option(BUILD_WORKER "Build Worker Server" ON)
# # option(BUILD_CLIENT "Build C++ Client" ON)
# # option(ENABLE_GPU "Enable GPU support (CUDA)" OFF)

# # # Output directories
# # set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# # set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# # set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# # # ============================================
# # # CRITICAL: Find Conan packages with CONFIG
# # # ============================================
# # find_package(Threads REQUIRED)
# # find_package(protobuf REQUIRED CONFIG)
# # find_package(gRPC REQUIRED CONFIG)

# # message(STATUS "========================================")
# # message(STATUS "Protobuf Configuration:")
# # message(STATUS "  Version: ${protobuf_VERSION}")

# # # Get protoc executable - try all possible locations
# # get_target_property(_protoc_location protobuf::protoc IMPORTED_LOCATION_RELEASE)
# # if(NOT _protoc_location)
# #     get_target_property(_protoc_location protobuf::protoc IMPORTED_LOCATION_RELWITHDEBINFO)
# # endif()
# # if(NOT _protoc_location)
# #     get_target_property(_protoc_location protobuf::protoc IMPORTED_LOCATION_DEBUG)
# # endif()
# # if(NOT _protoc_location)
# #     get_target_property(_protoc_location protobuf::protoc IMPORTED_LOCATION)
# # endif()

# # # Validate protoc
# # if(NOT _protoc_location)
# #     message(FATAL_ERROR "Could not get protoc location from protobuf::protoc target!")
# # endif()

# # if(NOT EXISTS "${_protoc_location}")
# #     message(FATAL_ERROR "protoc not found at: ${_protoc_location}")
# # endif()

# # # Set as cache variable so it persists
# # set(PROTOBUF_PROTOC_EXECUTABLE "${_protoc_location}" CACHE FILEPATH "Protoc executable from Conan" FORCE)

# # message(STATUS "  Protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")

# # # Get grpc_cpp_plugin - try all possible locations
# # message(STATUS "gRPC Configuration:")
# # message(STATUS "  Version: ${gRPC_VERSION}")

# # get_target_property(_grpc_plugin_location gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
# # if(NOT _grpc_plugin_location)
# #     get_target_property(_grpc_plugin_location gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELWITHDEBINFO)
# # endif()
# # if(NOT _grpc_plugin_location)
# #     get_target_property(_grpc_plugin_location gRPC::grpc_cpp_plugin IMPORTED_LOCATION_DEBUG)
# # endif()
# # if(NOT _grpc_plugin_location)
# #     get_target_property(_grpc_plugin_location gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
# # endif()

# # # Validate plugin
# # if(NOT _grpc_plugin_location)
# #     message(FATAL_ERROR "Could not get grpc_cpp_plugin location from gRPC::grpc_cpp_plugin target!")
# # endif()

# # if(NOT EXISTS "${_grpc_plugin_location}")
# #     message(FATAL_ERROR "grpc_cpp_plugin not found at: ${_grpc_plugin_location}")
# # endif()

# # # Set as cache variable
# # set(GRPC_CPP_PLUGIN_EXECUTABLE "${_grpc_plugin_location}" CACHE FILEPATH "gRPC C++ plugin from Conan" FORCE)

# # message(STATUS "  Plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")
# # message(STATUS "========================================")

# # # ============================================
# # # MIXR Setup (Optional)
# # # ============================================
# # set(MIXR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr")
# # set(MIXR_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr-3rdparty")

# # if(EXISTS "${MIXR_ROOT}/lib" AND EXISTS "${MIXR_3RDPARTY}/lib")
# #     message(STATUS "Found MIXR installation")
    
# #     file(GLOB MIXR_LIBS "${MIXR_ROOT}/lib/*.a")
# #     file(GLOB MIXR_3RD_LIBS "${MIXR_3RDPARTY}/lib/*.a")
    
# #     add_library(mixr INTERFACE)
    
# #     target_include_directories(mixr INTERFACE
# #         ${MIXR_ROOT}/include
# #         ${MIXR_3RDPARTY}/include
# #     )
    
# #     target_link_directories(mixr INTERFACE
# #         ${MIXR_ROOT}/lib
# #         ${MIXR_3RDPARTY}/lib
# #     )
    
# #     target_link_libraries(mixr INTERFACE
# #         ${MIXR_LIBS}
# #         ${MIXR_3RD_LIBS}
# #     )
    
# #     find_package(OpenGL QUIET)
# #     find_package(GLUT QUIET)
# #     find_package(Freetype QUIET)
    
# #     if(OpenGL_FOUND AND GLUT_FOUND AND Freetype_FOUND)
# #         target_link_libraries(mixr INTERFACE
# #             OpenGL::GL
# #             GLUT::GLUT
# #             Freetype::Freetype
# #             ftgl fontconfig X11 Xi dl pthread m
# #         )
# #         set(MIXR_FOUND TRUE)
# #         message(STATUS "✓ MIXR enabled")
# #     else()
# #         message(STATUS "⚠ MIXR disabled (missing OpenGL deps)")
# #         set(MIXR_FOUND FALSE)
# #     endif()
# # else()
# #     set(MIXR_FOUND FALSE)
# # endif()

# # # ============================================
# # # ONNX Runtime Detection
# # # ============================================
# # set(ONNXRUNTIME_ROOT_DIR "" CACHE PATH "ONNX Runtime installation directory")

# # if(ONNXRUNTIME_ROOT_DIR)
# #     set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include")
# #     set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_DIR}/lib")
# # else()
# #     find_path(ONNXRUNTIME_INCLUDE_DIR
# #         NAMES onnxruntime_cxx_api.h
# #         PATHS /usr/local/include /usr/include /opt/onnxruntime/include
# #     )
    
# #     find_library(ONNXRUNTIME_LIBRARY
# #         NAMES onnxruntime
# #         PATHS /usr/local/lib /usr/lib /opt/onnxruntime/lib
# #     )
    
# #     if(ONNXRUNTIME_LIBRARY)
# #         get_filename_component(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_LIBRARY}" DIRECTORY)
# #     endif()
# # endif()

# # if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIB_DIR)
# #     add_library(onnxruntime SHARED IMPORTED)
# #     set_target_properties(onnxruntime PROPERTIES
# #         IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.so"
# #         INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
# #     )
# #     set(ONNXRUNTIME_FOUND TRUE)
# #     message(STATUS "ONNX Runtime: Found")
# # else()
# #     set(ONNXRUNTIME_FOUND FALSE)
# #     message(WARNING "ONNX Runtime: Not found (mock inference only)")
# # endif()

# # # ============================================
# # # Generate protobuf and gRPC files - FIXED VERSION
# # # ============================================
# # set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
# # set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
# # file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# # message(STATUS "")
# # message(STATUS "Protobuf Generation:")
# # message(STATUS "  Input:  ${PROTO_DIR}")
# # message(STATUS "  Output: ${PROTO_OUTPUT_DIR}")
# # message(STATUS "  Tool:   ${PROTOBUF_PROTOC_EXECUTABLE}")
# # message(STATUS "  Plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# # # Define proto files
# # set(PROTO_FILES
# #     ${PROTO_DIR}/common.proto
# #     ${PROTO_DIR}/worker.proto
# # )

# # # Lists for generated files
# # set(GENERATED_PROTO_SRCS)
# # set(GENERATED_GRPC_SRCS)

# # # Generate for each proto file
# # foreach(PROTO_FILE ${PROTO_FILES})
# #     get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
# #     set(PROTO_SRC "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc")
# #     set(PROTO_HDR "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h")
# #     set(GRPC_SRC "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
# #     set(GRPC_HDR "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h")
    
# #     list(APPEND GENERATED_PROTO_SRCS ${PROTO_SRC})
# #     list(APPEND GENERATED_GRPC_SRCS ${GRPC_SRC})
    
# #     message(STATUS "  Configuring: ${PROTO_NAME}.proto")
    
# #     # CRITICAL FIX: Use absolute paths and explicit variables
# #     add_custom_command(
# #         OUTPUT 
# #             ${PROTO_SRC}
# #             ${PROTO_HDR}
# #             ${GRPC_SRC}
# #             ${GRPC_HDR}
# #         COMMAND 
# #             ${CMAKE_COMMAND} -E echo "Generating ${PROTO_NAME}..."
# #         COMMAND
# #             "${PROTOBUF_PROTOC_EXECUTABLE}"
# #             --cpp_out=${PROTO_OUTPUT_DIR}
# #             --grpc_out=${PROTO_OUTPUT_DIR}
# #             --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_EXECUTABLE}"
# #             -I ${PROTO_DIR}
# #             ${PROTO_FILE}
# #         DEPENDS 
# #             ${PROTO_FILE}
# #             ${PROTOBUF_PROTOC_EXECUTABLE}
# #             ${GRPC_CPP_PLUGIN_EXECUTABLE}
# #         WORKING_DIRECTORY 
# #             ${CMAKE_CURRENT_SOURCE_DIR}
# #         COMMENT 
# #             "Running protoc on ${PROTO_NAME}.proto"
# #         VERBATIM
# #     )
# # endforeach()

# # # Create proto library
# # add_library(proto_lib STATIC
# #     ${GENERATED_PROTO_SRCS}
# #     ${GENERATED_GRPC_SRCS}
# # )

# # # Make sure proto files are generated before compiling
# # set_source_files_properties(
# #     ${GENERATED_PROTO_SRCS}
# #     ${GENERATED_GRPC_SRCS}
# #     PROPERTIES GENERATED TRUE
# # )

# # target_include_directories(proto_lib PUBLIC
# #     ${PROTO_OUTPUT_DIR}
# # )

# # # CRITICAL: Link against Conan protobuf and gRPC
# # target_link_libraries(proto_lib PUBLIC
# #     protobuf::libprotobuf
# #     gRPC::grpc++
# #     gRPC::grpc++_reflection
# # )

# # # Force generation before compilation
# # add_custom_target(generate_proto_files ALL
# #     DEPENDS 
# #         ${GENERATED_PROTO_SRCS}
# #         ${GENERATED_GRPC_SRCS}
# # )

# # add_dependencies(proto_lib generate_proto_files)

# # # Compiler warnings
# # add_compile_options(-Wall -Wextra -Wpedantic)

# # # Build Worker
# # if(BUILD_WORKER)
# #     add_subdirectory(cpp/worker)
# # endif()

# # # Build Client
# # if(BUILD_CLIENT)
# #     add_subdirectory(cpp/client)
# # endif()

# # # Summary
# # message(STATUS "")
# # message(STATUS "========================================")
# # message(STATUS "Configuration Summary")
# # message(STATUS "========================================")
# # message(STATUS "Build Type:      ${CMAKE_BUILD_TYPE}")
# # message(STATUS "Worker:          ${BUILD_WORKER}")
# # message(STATUS "Client:          ${BUILD_CLIENT}")
# # message(STATUS "ONNX Runtime:    ${ONNXRUNTIME_FOUND}")
# # message(STATUS "MIXR:            ${MIXR_FOUND}")
# # message(STATUS "Protobuf:        ${protobuf_VERSION}")
# # message(STATUS "gRPC:            ${gRPC_VERSION}")
# # message(STATUS "========================================")
# # message(STATUS "")

# # # cmake_minimum_required(VERSION 3.20)
# # # project(MLInferenceSystem VERSION 1.0.0 LANGUAGES CXX)

# # # # Set C++ standard
# # # set(CMAKE_CXX_STANDARD 17)
# # # set(CMAKE_CXX_STANDARD_REQUIRED ON)
# # # set(CMAKE_CXX_EXTENSIONS OFF)

# # # # Export compile commands for tooling
# # # set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# # # # Build type
# # # if(NOT CMAKE_BUILD_TYPE)
# # #     set(CMAKE_BUILD_TYPE Release)
# # # endif()

# # # # Options
# # # option(BUILD_WORKER "Build Worker Server" ON)
# # # option(BUILD_CLIENT "Build C++ Client" ON)
# # # option(ENABLE_GPU "Enable GPU support (CUDA)" OFF)

# # # # Output directories
# # # set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# # # set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# # # set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# # # # ============================================
# # # # CRITICAL: Find Conan-generated dependencies FIRST
# # # # This prevents CMake from finding system packages
# # # # ============================================
# # # find_package(Threads REQUIRED)
# # # find_package(protobuf REQUIRED CONFIG)
# # # find_package(gRPC REQUIRED CONFIG)

# # # message(STATUS "========================================")
# # # message(STATUS "Protobuf Configuration:")
# # # message(STATUS "  Version: ${protobuf_VERSION}")

# # # # CRITICAL: Get protoc executable from Conan package
# # # get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
# # # if(NOT PROTOBUF_PROTOC_EXECUTABLE)
# # #     get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)
# # # endif()
# # # if(NOT PROTOBUF_PROTOC_EXECUTABLE)
# # #     get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_DEBUG)
# # # endif()

# # # if(NOT PROTOBUF_PROTOC_EXECUTABLE OR NOT EXISTS ${PROTOBUF_PROTOC_EXECUTABLE})
# # #     message(FATAL_ERROR "Could not find protoc executable from Conan!")
# # # endif()

# # # message(STATUS "  Protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")

# # # message(STATUS "gRPC Configuration:")
# # # message(STATUS "  Version: ${gRPC_VERSION}")

# # # # CRITICAL: Get grpc_cpp_plugin from Conan package
# # # get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
# # # if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
# # #     get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
# # # endif()
# # # if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
# # #     get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_DEBUG)
# # # endif()

# # # if(NOT GRPC_CPP_PLUGIN_EXECUTABLE OR NOT EXISTS ${GRPC_CPP_PLUGIN_EXECUTABLE})
# # #     message(FATAL_ERROR "Could not find grpc_cpp_plugin from Conan!")
# # # endif()

# # # message(STATUS "  Plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")
# # # message(STATUS "========================================")

# # # # ============================================
# # # # MIXR Setup (Optional)
# # # # ============================================
# # # set(MIXR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr")
# # # set(MIXR_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr-3rdparty")

# # # if(EXISTS "${MIXR_ROOT}/lib" AND EXISTS "${MIXR_3RDPARTY}/lib")
# # #     message(STATUS "Found MIXR installation")
# # #     message(STATUS "  MIXR Root: ${MIXR_ROOT}")
# # #     message(STATUS "  MIXR 3rd Party: ${MIXR_3RDPARTY}")
    
# # #     # Find all MIXR libraries
# # #     file(GLOB MIXR_LIBS "${MIXR_ROOT}/lib/*.a")
# # #     file(GLOB MIXR_3RD_LIBS "${MIXR_3RDPARTY}/lib/*.a")
    
# # #     # Create MIXR target
# # #     add_library(mixr INTERFACE)
    
# # #     target_include_directories(mixr INTERFACE
# # #         ${MIXR_ROOT}/include
# # #         ${MIXR_3RDPARTY}/include
# # #     )
    
# # #     target_link_directories(mixr INTERFACE
# # #         ${MIXR_ROOT}/lib
# # #         ${MIXR_3RDPARTY}/lib
# # #     )
    
# # #     target_link_libraries(mixr INTERFACE
# # #         ${MIXR_LIBS}
# # #         ${MIXR_3RD_LIBS}
# # #     )
    
# # #     # Find system dependencies for MIXR - QUIET
# # #     find_package(OpenGL QUIET)
# # #     find_package(GLUT QUIET)
# # #     find_package(Freetype QUIET)
    
# # #     if(OpenGL_FOUND AND GLUT_FOUND AND Freetype_FOUND)
# # #         target_link_libraries(mixr INTERFACE
# # #             OpenGL::GL
# # #             GLUT::GLUT
# # #             Freetype::Freetype
# # #             ftgl
# # #             fontconfig
# # #             X11
# # #             Xi
# # #             dl
# # #             pthread
# # #             m
# # #         )
        
# # #         set(MIXR_FOUND TRUE)
# # #         message(STATUS "✓ MIXR enabled with OpenGL support")
# # #     else()
# # #         message(STATUS "⚠ MIXR found but OpenGL missing - MIXR disabled")
# # #         set(MIXR_FOUND FALSE)
# # #     endif()
# # # else()
# # #     message(STATUS "MIXR not found")
# # #     set(MIXR_FOUND FALSE)
# # # endif()

# # # # ============================================
# # # # ONNX Runtime Detection
# # # # ============================================
# # # set(ONNXRUNTIME_ROOT_DIR "" CACHE PATH "ONNX Runtime installation directory")

# # # if(ONNXRUNTIME_ROOT_DIR)
# # #     set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include")
# # #     set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_DIR}/lib")
# # # else()
# # #     find_path(ONNXRUNTIME_INCLUDE_DIR
# # #         NAMES onnxruntime_cxx_api.h
# # #         PATHS /usr/local/include /usr/include /opt/onnxruntime/include
# # #     )
    
# # #     find_library(ONNXRUNTIME_LIBRARY
# # #         NAMES onnxruntime
# # #         PATHS /usr/local/lib /usr/lib /opt/onnxruntime/lib
# # #     )
    
# # #     if(ONNXRUNTIME_LIBRARY)
# # #         get_filename_component(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_LIBRARY}" DIRECTORY)
# # #     endif()
# # # endif()

# # # if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIB_DIR)
# # #     add_library(onnxruntime SHARED IMPORTED)
# # #     set_target_properties(onnxruntime PROPERTIES
# # #         IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.so"
# # #         INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
# # #     )
# # #     set(ONNXRUNTIME_FOUND TRUE)
# # #     message(STATUS "ONNX Runtime: ${ONNXRUNTIME_LIB_DIR}")
# # # else()
# # #     set(ONNXRUNTIME_FOUND FALSE)
# # #     message(WARNING "ONNX Runtime not found - worker will use mock inference")
# # # endif()

# # # # ============================================
# # # # Generate protobuf and gRPC files
# # # # ============================================
# # # set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
# # # set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
# # # file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# # # set(PROTO_FILES
# # #     ${PROTO_DIR}/common.proto
# # #     ${PROTO_DIR}/worker.proto
# # # )

# # # set(GENERATED_PROTO_SRCS)
# # # set(GENERATED_PROTO_HDRS)
# # # set(GENERATED_GRPC_SRCS)
# # # set(GENERATED_GRPC_HDRS)

# # # message(STATUS "Generating protobuf files with:")
# # # message(STATUS "  protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")
# # # message(STATUS "  plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# # # foreach(PROTO_FILE ${PROTO_FILES})
# # #     get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
# # #     list(APPEND GENERATED_PROTO_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc")
# # #     list(APPEND GENERATED_PROTO_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h")
# # #     list(APPEND GENERATED_GRPC_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
# # #     list(APPEND GENERATED_GRPC_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h")
    
# # #     add_custom_command(
# # #         OUTPUT "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc"
# # #                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h"
# # #                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc"
# # #                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h"
# # #         COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
# # #         ARGS --cpp_out=${PROTO_OUTPUT_DIR}
# # #              --grpc_out=${PROTO_OUTPUT_DIR}
# # #              --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
# # #              -I${PROTO_DIR}
# # #              ${PROTO_FILE}
# # #         DEPENDS ${PROTO_FILE}
# # #         COMMENT "Generating ${PROTO_NAME}"
# # #         VERBATIM
# # #     )
# # # endforeach()

# # # # Create proto library
# # # add_library(proto_lib STATIC
# # #     ${GENERATED_PROTO_SRCS}
# # #     ${GENERATED_GRPC_SRCS}
# # # )

# # # target_include_directories(proto_lib PUBLIC
# # #     ${PROTO_OUTPUT_DIR}
# # # )

# # # target_link_libraries(proto_lib PUBLIC
# # #     protobuf::libprotobuf
# # #     gRPC::grpc++
# # #     gRPC::grpc++_reflection
# # # )

# # # # Compiler warnings
# # # add_compile_options(-Wall -Wextra -Wpedantic)

# # # # Build Worker
# # # if(BUILD_WORKER)
# # #     add_subdirectory(cpp/worker)
# # # endif()

# # # # Build Client
# # # if(BUILD_CLIENT)
# # #     add_subdirectory(cpp/client)
# # # endif()

# # # # Summary
# # # message(STATUS "")
# # # message(STATUS "========================================")
# # # message(STATUS "Configuration Summary")
# # # message(STATUS "========================================")
# # # message(STATUS "Build Type:      ${CMAKE_BUILD_TYPE}")
# # # message(STATUS "Build Worker:    ${BUILD_WORKER}")
# # # message(STATUS "Build Client:    ${BUILD_CLIENT}")
# # # message(STATUS "ONNX Runtime:    ${ONNXRUNTIME_FOUND}")
# # # message(STATUS "MIXR Support:    ${MIXR_FOUND}")
# # # message(STATUS "Protobuf:        ${protobuf_VERSION}")
# # # message(STATUS "gRPC:            ${gRPC_VERSION}")
# # # message(STATUS "========================================")

# # # # cmake_minimum_required(VERSION 3.20)
# # # # project(MLInferenceSystem VERSION 1.0.0 LANGUAGES CXX)

# # # # # Set C++ standard
# # # # set(CMAKE_CXX_STANDARD 17)
# # # # set(CMAKE_CXX_STANDARD_REQUIRED ON)
# # # # set(CMAKE_CXX_EXTENSIONS OFF)

# # # # # Export compile commands for tooling
# # # # set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# # # # # Build type
# # # # if(NOT CMAKE_BUILD_TYPE)
# # # #     set(CMAKE_BUILD_TYPE Release)
# # # # endif()

# # # # # Options
# # # # option(BUILD_WORKER "Build Worker Server" ON)
# # # # option(BUILD_CLIENT "Build C++ Client" ON)
# # # # option(ENABLE_GPU "Enable GPU support (CUDA)" OFF)

# # # # # Output directories
# # # # set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# # # # set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# # # # set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# # # # # Find Conan-generated dependencies
# # # # find_package(Threads REQUIRED)
# # # # find_package(protobuf REQUIRED)
# # # # find_package(gRPC REQUIRED)

# # # # message(STATUS "Found Protobuf via Conan")
# # # # message(STATUS "Found gRPC via Conan")


# # # # # # ============================================
# # # # # # MIXR Setup
# # # # # # ============================================
# # # # # set(MIXR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr")
# # # # # set(MIXR_3RDPARTY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/mixr_dev/mixr-3rdparty")

# # # # # if(EXISTS "${MIXR_ROOT}/lib" AND EXISTS "${MIXR_3RDPARTY}/lib")
# # # # #     message(STATUS "Found MIXR installation")
# # # # #     message(STATUS "  MIXR Root: ${MIXR_ROOT}")
# # # # #     message(STATUS "  MIXR 3rd Party: ${MIXR_3RDPARTY}")
    
# # # # #     # Find all MIXR libraries
# # # # #     file(GLOB MIXR_LIBS "${MIXR_ROOT}/lib/*.a")
# # # # #     file(GLOB MIXR_3RD_LIBS "${MIXR_3RDPARTY}/lib/*.a")
    
# # # # #     # Create MIXR target
# # # # #     add_library(mixr INTERFACE)
    
# # # # #     target_include_directories(mixr INTERFACE
# # # # #         ${MIXR_ROOT}/include
# # # # #         ${MIXR_3RDPARTY}/include
# # # # #     )
    
# # # # #     target_link_directories(mixr INTERFACE
# # # # #         ${MIXR_ROOT}/lib
# # # # #         ${MIXR_3RDPARTY}/lib
# # # # #     )
    
# # # # #     target_link_libraries(mixr INTERFACE
# # # # #         ${MIXR_LIBS}
# # # # #         ${MIXR_3RD_LIBS}
# # # # #     )
    
# # # # #     # Find system dependencies for MIXR
# # # # #     find_package(OpenGL REQUIRED)
# # # # #     find_package(GLUT REQUIRED)
# # # # #     find_package(Freetype REQUIRED)
    
# # # # #     target_link_libraries(mixr INTERFACE
# # # # #         OpenGL::GL
# # # # #         GLUT::GLUT
# # # # #         Freetype::Freetype
# # # # #         ftgl
# # # # #         fontconfig
# # # # #         X11
# # # # #         Xi
# # # # #         dl
# # # # #         pthread
# # # # #         m
# # # # #     )
    
# # # # #     set(MIXR_FOUND TRUE)
# # # # #     message(STATUS "MIXR target created - ready to use")
# # # # # else()
# # # # #     message(STATUS "MIXR not found (install with: ./scripts/install_mixr.sh)")
# # # # #     set(MIXR_FOUND FALSE)
# # # # # endif()

# # # # # ============================================
# # # # # ONNX Runtime Detection
# # # # # ============================================

# # # # # Try to find ONNX Runtime in system paths or custom location
# # # # set(ONNXRUNTIME_ROOT_DIR "" CACHE PATH "ONNX Runtime installation directory")

# # # # if(ONNXRUNTIME_ROOT_DIR)
# # # #     message(STATUS "Looking for ONNX Runtime in: ${ONNXRUNTIME_ROOT_DIR}")
# # # #     set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT_DIR}/include")
# # # #     set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT_DIR}/lib")
# # # # else()
# # # #     # Try standard system paths
# # # #     find_path(ONNXRUNTIME_INCLUDE_DIR
# # # #         NAMES onnxruntime_cxx_api.h
# # # #         PATHS
# # # #             /usr/local/include
# # # #             /usr/include
# # # #             /opt/onnxruntime/include
# # # #     )
    
# # # #     find_library(ONNXRUNTIME_LIBRARY
# # # #         NAMES onnxruntime
# # # #         PATHS
# # # #             /usr/local/lib
# # # #             /usr/lib
# # # #             /opt/onnxruntime/lib
# # # #     )
    
# # # #     if(ONNXRUNTIME_LIBRARY)
# # # #         get_filename_component(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_LIBRARY}" DIRECTORY)
# # # #     endif()
# # # # endif()

# # # # if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIB_DIR)
# # # #     message(STATUS "Found ONNX Runtime:")
# # # #     message(STATUS "  Include: ${ONNXRUNTIME_INCLUDE_DIR}")
# # # #     message(STATUS "  Library: ${ONNXRUNTIME_LIB_DIR}")
    
# # # #     add_library(onnxruntime SHARED IMPORTED)
# # # #     set_target_properties(onnxruntime PROPERTIES
# # # #         IMPORTED_LOCATION "${ONNXRUNTIME_LIB_DIR}/libonnxruntime.so"
# # # #         INTERFACE_INCLUDE_DIRECTORIES "${ONNXRUNTIME_INCLUDE_DIR}"
# # # #     )
    
# # # #     set(ONNXRUNTIME_FOUND TRUE)
# # # # else()
# # # #     message(WARNING "ONNX Runtime not found!")
# # # #     message(WARNING "The worker will be built without ONNX support (mock inference only)")
# # # #     message(WARNING "To enable ONNX Runtime:")
# # # #     message(WARNING "  1. Download from: https://github.com/microsoft/onnxruntime/releases")
# # # #     message(WARNING "  2. Extract to /opt/onnxruntime or set ONNXRUNTIME_ROOT_DIR")
# # # #     message(WARNING "  3. Re-run cmake")
# # # #     set(ONNXRUNTIME_FOUND FALSE)
# # # # endif()

# # # # # Get protoc executable
# # # # get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
# # # # if(NOT PROTOBUF_PROTOC_EXECUTABLE)
# # # #     get_target_property(PROTOBUF_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)
# # # # endif()
# # # # message(STATUS "Found protoc: ${PROTOBUF_PROTOC_EXECUTABLE}")

# # # # # Get grpc_cpp_plugin
# # # # get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION_RELEASE)
# # # # if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
# # # #     get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin IMPORTED_LOCATION)
# # # # endif()
# # # # message(STATUS "Found grpc_cpp_plugin: ${GRPC_CPP_PLUGIN_EXECUTABLE}")

# # # # # Generate protobuf and gRPC files
# # # # set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
# # # # set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
# # # # file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# # # # set(PROTO_FILES
# # # #     ${PROTO_DIR}/common.proto
# # # #     ${PROTO_DIR}/worker.proto
# # # # )

# # # # set(GENERATED_PROTO_SRCS)
# # # # set(GENERATED_PROTO_HDRS)
# # # # set(GENERATED_GRPC_SRCS)
# # # # set(GENERATED_GRPC_HDRS)

# # # # foreach(PROTO_FILE ${PROTO_FILES})
# # # #     get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
# # # #     list(APPEND GENERATED_PROTO_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc")
# # # #     list(APPEND GENERATED_PROTO_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h")
# # # #     list(APPEND GENERATED_GRPC_SRCS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc")
# # # #     list(APPEND GENERATED_GRPC_HDRS "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h")
    
# # # #     add_custom_command(
# # # #         OUTPUT "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc"
# # # #                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h"
# # # #                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc"
# # # #                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h"
# # # #         COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
# # # #         ARGS --cpp_out=${PROTO_OUTPUT_DIR}
# # # #              --grpc_out=${PROTO_OUTPUT_DIR}
# # # #              --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
# # # #              -I${PROTO_DIR}
# # # #              ${PROTO_FILE}
# # # #         DEPENDS ${PROTO_FILE}
# # # #         COMMENT "Generating protobuf and gRPC files for ${PROTO_NAME}"
# # # #         VERBATIM
# # # #     )
# # # # endforeach()

# # # # # Create proto library
# # # # add_library(proto_lib STATIC
# # # #     ${GENERATED_PROTO_SRCS}
# # # #     ${GENERATED_GRPC_SRCS}
# # # # )

# # # # target_include_directories(proto_lib PUBLIC
# # # #     ${PROTO_OUTPUT_DIR}
# # # # )

# # # # target_link_libraries(proto_lib PUBLIC
# # # #     protobuf::libprotobuf
# # # #     gRPC::grpc++
# # # #     gRPC::grpc++_reflection
# # # # )

# # # # # Compiler warnings
# # # # add_compile_options(
# # # #     -Wall -Wextra -Wpedantic
# # # # )

# # # # # Build Worker
# # # # if(BUILD_WORKER)
# # # #     add_subdirectory(cpp/worker)
# # # # endif()

# # # # # Build Client
# # # # if(BUILD_CLIENT)
# # # #     add_subdirectory(cpp/client)
# # # # endif()