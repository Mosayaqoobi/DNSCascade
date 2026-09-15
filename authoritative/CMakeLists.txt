add_executable(authoritative_server
    AuthoritativeServer.cpp
)
target_link_libraries(authoritative_server PRIVATE server_base)