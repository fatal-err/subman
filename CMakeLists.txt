cmake_minimum_required(VERSION 3.12)

project(subman)
set(exec_name subman)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.66.0 COMPONENTS program_options filesystem)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(${exec_name}
    src/main.cpp
    src/subtitle.cpp
    src/formats/subrip.cpp
    src/styledstring.cpp
    src/duration.cpp
    src/formats/subrip.cpp
    src/document.cpp
    src/utilities.cpp
    )
  target_link_libraries(${exec_name} ${Boost_LIBRARIES})
endif()


