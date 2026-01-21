add_executable(worker
    src/main.cpp
    # src/worker_server.cpp
    # src/inference_engine.cpp
)

target_include_directories(worker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(worker PRIVATE
    # proto_lib
    Threads::Threads
)

install(TARGETS worker
    RUNTIME DESTINATION bin
)
