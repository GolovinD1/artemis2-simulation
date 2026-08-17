cmake_minimum_required(VERSION 3.13)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

project(artemis_sim)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(HEADERS
    include/math_types.h
    include/physics.h
    include/simulation_window.h
)

set(SOURCES
    src/main.cpp
    src/physics.cpp
    src/simulation_window.cpp
)

set(DATA_FILES
    data/moon.txt
    data/orion.txt
)


#СБОРКА ОСНОВНОЙ ПРОГРАММЫ

add_executable(artemis_sim
    ${SOURCES}
    ${HEADERS}
    ${DATA_FILES}
)

target_compile_definitions(artemis_sim PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_include_directories(artemis_sim PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(artemis_sim PRIVATE Qt5::Widgets)



#СБОРКА ТЕСТОВ

find_path(BOOST_INCLUDE_DIR
    NAMES boost/test/included/unit_test.hpp
    DOC "PATH Boost"
)

if(NOT BOOST_INCLUDE_DIR)
    message(FATAL_ERROR "Boost not found! Please set BOOST_INCLUDE_DIR in Qt Creator CMake configuration.")
endif()

add_executable(artemis_tests
    tests/test_main.cpp
    src/physics.cpp
)

target_include_directories(artemis_tests PRIVATE
    ${BOOST_INCLUDE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

include(CTest)
enable_testing()
add_test(NAME PhysicsMathTests COMMAND artemis_tests)