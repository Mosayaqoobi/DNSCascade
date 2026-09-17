add_library(common STATIC
    DnsMessage.cpp
    UdpSocket.cpp
    tests/test_roundtrip.cpp
)

add_executable(test_roundtrip
    tests/test_roundtrip.cpp
)
target_link_libraries(test_roundtrip PRIVATE common)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PUBLIC nlohmann_json::nlohmann_json)