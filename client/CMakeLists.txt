add_library(client
    src/DnsClient.cpp
)
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(client PUBLIC common)