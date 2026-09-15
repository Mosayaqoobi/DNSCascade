add_library(server_base STATIC
    DnsServer.cpp
)
target_include_directories(server_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server_base PUBLIC common)