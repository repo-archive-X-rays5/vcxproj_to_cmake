cmake_minimum_required(VERSION 3.20)
project(vcxproj_to_cmake)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/pugixml)

add_executable(vcxproj_to_cmake main.cpp)

target_link_libraries(vcxproj_to_cmake pugixml pugixml::pugixml)