cmake_minimum_required(VERSION 3.28)
project(group2project LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(prog
	src/main.cpp
   	src/screencalc.cpp
	src/gamelogic.cpp
	src/gamedraw.cpp
)
target_compile_features(prog PRIVATE cxx_std_17)
target_include_directories(prog PRIVATE include)
target_link_libraries(prog PRIVATE SFML::Graphics)