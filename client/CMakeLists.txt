add_library(client
    DnsClient.cpp
)
target_link_libraries(client PRIVATE common)