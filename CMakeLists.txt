
cmake_minimum_required(VERSION 3.16)

project(RestaurantManagement)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(httplib json)

add_executable(restaurant server.cpp)

target_link_libraries(
    restaurant
    PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
)
