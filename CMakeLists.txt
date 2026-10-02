cmake_minimum_required(VERSION 3.16)
project(GUICompressor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6 or fallback to Qt5
find_package(Qt6 QUIET COMPONENTS Core Widgets)
if(Qt6_FOUND)
    message(STATUS "Found Qt6: ${Qt6_VERSION}")
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    message(STATUS "Found Qt5: ${Qt5_VERSION}")
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets)
endif()

# Core C++17 library (PURE C++ — NO Qt allowed here)
file(GLOB CORE_SOURCES
    "src/core/*.cpp"
    "src/core/*.hpp"
)

add_library(gcore STATIC ${CORE_SOURCES})
target_include_directories(gcore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/core")

# GUI + Controller + Application
file(GLOB CONTROLLER_SOURCES "src/controller/*.cpp" "src/controller/*.hpp")
file(GLOB GUI_SOURCES "src/gui/*.cpp" "src/gui/*.hpp")

add_executable(GUICompressor
    src/main.cpp
    ${CONTROLLER_SOURCES}
    ${GUI_SOURCES}
)

target_include_directories(GUICompressor PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/controller"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/gui"
)

target_link_libraries(GUICompressor PRIVATE gcore ${QT_LIBRARIES})

if(WIN32)
    # Hide console window on Windows release builds
    set_property(TARGET GUICompressor PROPERTY WIN32_EXECUTABLE TRUE)
endif()

# Unit Testing suite
enable_testing()

file(GLOB TEST_FILES "tests/test_*.cpp")
foreach(TEST_SRC ${TEST_FILES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(${TEST_NAME} PRIVATE gcore)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
