add_executable(tld_server
    tldServer.cpp
)
target_link_libraries(tld_server PRIVATE server_base)