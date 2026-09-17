cmake_minimum_required(VERSION 3.15)
project(DNSCascade CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_compile_options(-Wall -Wextra)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)


add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(root)
add_subdirectory(tld)
add_subdirectory(authoritative)
add_subdirectory(client)
add_subdirectory(resolver)

add_executable(dns_client main.cpp)
target_link_libraries(dns_client PRIVATE client)