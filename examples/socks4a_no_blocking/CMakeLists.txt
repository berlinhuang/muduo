include_directories(udns)
link_directories(${PROJECT_SOURCE_DIR}/examples/socks4a_no_blocking/lib)
add_executable(socks4a_no_blocking main.cpp socks4_server.cpp Resolve.cpp tunnel.h)
target_link_libraries(socks4a_no_blocking muduo_net muduo_base udns ${CMAKE_THREAD_LIBS_INIT})
