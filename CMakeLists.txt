cmake_minimum_required(VERSION 3.28)
project(group2project LANGUAGES CXX)

find_package(SFML 3 REQUIRED COMPONENTS Graphics)

add_executable(prog
	src/main.cpp
	src/screencalc.cpp
	src/gamelogic.cpp
	src/gamedraw.cpp
	src/menu.cpp
	src/collision.cpp
)
target_compile_features(prog PRIVATE cxx_std_23)
target_include_directories(prog PRIVATE include)
target_link_libraries(prog PRIVATE SFML::Graphics)
