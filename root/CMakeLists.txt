add_executable(root_server
    RootServer.cpp
)
target_link_libraries(root_server PRIVATE server_base)