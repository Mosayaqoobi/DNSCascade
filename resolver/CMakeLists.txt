add_executable(resolver
    LocalResolver.cpp
    DnsCache.cpp
)
target_link_libraries(resolver PRIVATE common)