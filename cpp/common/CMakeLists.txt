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

# # Compiler warnings
# include(cmake/CompilerWarnings.cmake)

# # Options
# option(BUILD_MANAGER "Build Manager Node" ON)
# option(BUILD_WORKER "Build Worker Node" ON)
# option(BUILD_CLIENT "Build C++ Client Library" ON)
# option(BUILD_TESTS "Build tests" OFF)
# option(ENABLE_GPU "Enable GPU support (CUDA)" ON)
# option(USE_ASAN "Enable AddressSanitizer" OFF)
# option(USE_TSAN "Enable ThreadSanitizer" OFF)

# # Output directories
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# # Add cmake modules path
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# # Find dependencies
# find_package(Threads REQUIRED)
# find_package(Protobuf REQUIRED)
# find_package(gRPC REQUIRED)

# # ONNX Runtime
# if(BUILD_WORKER)
#     find_package(ONNXRuntime REQUIRED)
#     if(ENABLE_GPU)
#         find_package(CUDA REQUIRED)
#     endif()
# endif()

# # Additional dependencies
# find_package(spdlog REQUIRED)
# find_package(yaml-cpp REQUIRED)
# find_package(OpenSSL REQUIRED)

# # Generate protobuf and gRPC files
# set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto")
# set(PROTO_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
# file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

# set(PROTO_FILES
#     ${PROTO_DIR}/common.proto
#     ${PROTO_DIR}/manager.proto
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
    
#     add_custom_command(
#         OUTPUT "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.cc"
#                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.pb.h"
#                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.cc"
#                "${PROTO_OUTPUT_DIR}/${PROTO_NAME}.grpc.pb.h"
#         COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
#         ARGS --cpp_out=${PROTO_OUTPUT_DIR}
#              --grpc_out=${PROTO_OUTPUT_DIR}
#              --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
#              -I${PROTO_DIR}
#              ${PROTO_FILE}
#         DEPENDS ${PROTO_FILE}
#         COMMENT "Generating protobuf and gRPC files for ${PROTO_NAME}"
#     )
# endforeach()

# # Create proto library
# add_library(proto_lib STATIC
#     ${GENERATED_PROTO_SRCS}
#     ${GENERATED_GRPC_SRCS}
# )

# target_include_directories(proto_lib PUBLIC
#     ${PROTO_OUTPUT_DIR}
#     ${PROTOBUF_INCLUDE_DIRS}
# )

# target_link_libraries(proto_lib PUBLIC
#     protobuf::libprotobuf
#     gRPC::grpc++
#     gRPC::grpc++_reflection
# )

# # Sanitizers
# if(USE_ASAN)
#     add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
#     add_link_options(-fsanitize=address)
# endif()

# if(USE_TSAN)
#     add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
#     add_link_options(-fsanitize=thread)
# endif()

# # Subdirectories
# add_subdirectory(common)

# if(BUILD_MANAGER)
#     add_subdirectory(manager)
# endif()

# if(BUILD_WORKER)
#     add_subdirectory(worker)
# endif()

# if(BUILD_CLIENT)
#     add_subdirectory(client)
# endif()

# if(BUILD_TESTS)
#     enable_testing()
#     add_subdirectory(tests)
# endif()

# # Print configuration summary
# message(STATUS "")
# message(STATUS "ML Inference System Configuration:")
# message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
# message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
# message(STATUS "  Build manager: ${BUILD_MANAGER}")
# message(STATUS "  Build worker: ${BUILD_WORKER}")
# message(STATUS "  Build client: ${BUILD_CLIENT}")
# message(STATUS "  Build tests: ${BUILD_TESTS}")
# message(STATUS "  GPU support: ${ENABLE_GPU}")
# message(STATUS "  Address Sanitizer: ${USE_ASAN}")
# message(STATUS "  Thread Sanitizer: ${USE_TSAN}")
# message(STATUS "")

# # Common library
# add_library(common STATIC
#     src/types.cpp
#     src/logger.cpp
#     src/config.cpp
# )

# target_include_directories(common PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# target_link_libraries(common PUBLIC
#     spdlog::spdlog
#     yaml-cpp
#     OpenSSL::SSL
#     OpenSSL::Crypto
#     Threads::Threads
# )

# # Set compile options
# target_compile_options(common PRIVATE
#     -Wall -Wextra -Wpedantic
#     $<$<CONFIG:Debug>:-g -O0>
#     $<$<CONFIG:Release>:-O3>
# )

# # Manager executable
# add_executable(manager
#     src/main.cpp
#     src/manager_server.cpp
#     src/worker_registry.cpp
#     src/load_balancer.cpp
#     src/api_key_manager.cpp
# )

# target_include_directories(manager PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# target_link_libraries(manager PRIVATE
#     common
#     proto_lib
#     gRPC::grpc++
#     gRPC::grpc++_reflection
#     Threads::Threads
# )

# # Set compile options
# target_compile_options(manager PRIVATE
#     -Wall -Wextra -Wpedantic
#     $<$<CONFIG:Debug>:-g -O0>
#     $<$<CONFIG:Release>:-O3>
# )

# # Install
# install(TARGETS manager
#     RUNTIME DESTINATION bin
# )

# # Worker executable
# add_executable(worker
#     src/main.cpp
#     src/worker_server.cpp
#     src/inference_engine.cpp
# )

# target_include_directories(worker PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# target_link_libraries(worker PRIVATE
#     common
#     proto_lib
#     gRPC::grpc++
#     gRPC::grpc++_reflection
#     Threads::Threads
# )

# # ONNX Runtime (if available)
# if(BUILD_WORKER AND ONNXRuntime_FOUND)
#     target_link_libraries(worker PRIVATE ${ONNXRuntime_LIBRARIES})
#     target_include_directories(worker PRIVATE ${ONNXRuntime_INCLUDE_DIRS})
# endif()

# # CUDA (if enabled)
# if(ENABLE_GPU AND CUDA_FOUND)
#     target_link_libraries(worker PRIVATE CUDA::cudart)
#     target_compile_definitions(worker PRIVATE ENABLE_GPU)
# endif()

# # Set compile options
# target_compile_options(worker PRIVATE
#     -Wall -Wextra -Wpedantic
#     $<$<CONFIG:Debug>:-g -O0>
#     $<$<CONFIG:Release>:-O3>
# )

# # Install
# install(TARGETS worker
#     RUNTIME DESTINATION bin
# )