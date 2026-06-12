cmake_minimum_required(VERSION 3.13)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")


project(artemis_sim)
set(CMAKE_CXX_STANDARD 17)

# на случай если много cpp
#file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
#
#add_executable(artemis_sim "${MY_SOURCES}")

add_executable(artemis_sim 
    src/main.cpp 
    src/physics.cpp
)


target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/") 


target_include_directories(artemis_sim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")



