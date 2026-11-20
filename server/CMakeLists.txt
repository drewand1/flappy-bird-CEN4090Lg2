cmake_minimum_required(VERSION 3.28)
project(scoreboard_server LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/httplib.cmake)

add_executable(scoreboard_server main.cpp)

target_compile_features(scoreboard_server PRIVATE cxx_std_23)
target_link_libraries(scoreboard_server PRIVATE httplib::httplib)
