add_library(client
    src/DnsClient.cpp
)
target_link_libraries(client PRIVATE common)