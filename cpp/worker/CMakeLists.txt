add_executable(worker
    src/main.cpp
    src/worker_server.cpp
    src/inference_engine.cpp
)

target_include_directories(worker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(worker PRIVATE
    proto_lib
    Threads::Threads
)

# Link ONNX Runtime if found
if(ONNXRUNTIME_FOUND)
    target_link_libraries(worker PRIVATE onnxruntime)
    target_compile_definitions(worker PRIVATE USE_ONNX_RUNTIME)
    
    if(ENABLE_GPU)
        target_compile_definitions(worker PRIVATE USE_CUDA)
    endif()
    
    message(STATUS "Worker will be built with ONNX Runtime support")
else()
    message(STATUS "Worker will be built with mock inference (no ONNX Runtime)")
endif()

install(TARGETS worker
    RUNTIME DESTINATION bin
)

# add_executable(worker
#     src/main.cpp
#     src/worker_server.cpp
#     src/inference_engine.cpp
# )

# target_include_directories(worker PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# target_link_libraries(worker PRIVATE
#     proto_lib
#     Threads::Threads
# )

# install(TARGETS worker
#     RUNTIME DESTINATION bin
# )
