cmake_minimum_required(VERSION 3.20)

# Client library
add_library(inference_client STATIC
    src/main.cpp
    # src/inference_client.cpp
)

target_include_directories(inference_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/proto
)

target_link_libraries(inference_client PUBLIC
    proto_lib
    gRPC::grpc++
    Threads::Threads
)

# Client example executable
add_executable(client_example
    src/main.cpp
)

target_include_directories(client_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(client_example PRIVATE
    inference_client
)

# Set compile options
target_compile_options(inference_client PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

target_compile_options(client_example PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
)

# Install
install(TARGETS inference_client client_example
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)