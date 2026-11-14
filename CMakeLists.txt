cmake_minimum_required(VERSION 3.28)
project(group2project LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
	cenui
	GIT_REPOSITORY https://github.com/drewand1/cenui/
	GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(cenui)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio)

add_executable(prog
	src/main.cpp
	src/screencalc.cpp
	src/gamelogic.cpp
	src/gamedraw.cpp
	src/collision.cpp
	src/background.cpp
	src/pipes.cpp
	src/statuses/birdselect.cpp
	src/statuses/menu.cpp
	src/statuses/playing.cpp
	src/statuses/gameover.cpp
)
target_compile_features(prog PRIVATE cxx_std_23)
target_include_directories(prog PRIVATE include)
target_link_libraries(prog PRIVATE SFML::Graphics)
target_link_libraries(prog PRIVATE cenui)
target_link_libraries(prog PRIVATE SFML::Audio)

# Copy the rsc folder to the output directory so the executable can find it
add_custom_command(TARGET prog POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/rsc
    $<TARGET_FILE_DIR:prog>/rsc)