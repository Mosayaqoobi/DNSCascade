add_executable(tld_server
    TldServer.cpp
)
target_link_libraries(tld_server PRIVATE server_base)