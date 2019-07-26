cmake_minimum_required(VERSION 3.12)

project(subman)
set(exec_name subman)

add_compile_options("-std=c++17")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug
       CACHE STRING "Choose the type of build : None Debug Release RelWithDebInfo MinSizeRel Coverage."
       FORCE)
endif()
message("Current build type is: ${CMAKE_BUILD_TYPE}")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SUBMAN_RELEASE_OPTIONS "-s -O3 -flto -Qy -ffunction-sections -fdata-sections -Wl,--gc-sections")
set(SUBMAN_DEBUG_OPTIONS "-g")


#add_compile_options("$<$<CONFIG:Release>:${SUBMAN_RELEASE_OPTIONS}>")
#add_compile_options("$<$<CONFIG:Debug>:${SUBMAN_DEBUG_OPTIONS}>")
if(CMAKE_BUILD_TYPE MATCHES "Release")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SUBMAN_RELEASE_OPTIONS}")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SUBMAN_DEBUG_OPTIONS}")
endif()


set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         ON) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.66.0 COMPONENTS program_options filesystem regex)

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
    src/search.cpp
    )
  target_link_libraries(${exec_name} PRIVATE ${Boost_LIBRARIES})

  # optimize the file size:
  #target_compile_options(${exec_name} PRIVATE -pthread)
  #add_custom_target(de COMMAND ${CMAKE_COMMAND} -E echo "\'$<$<CONFIG:Release>:${SUBMAN_RELEASE_OPTIONS}>\'")
  #target_compile_options(${exec_name} PRIVATE "$<$<CONFIG:Release>:${SUBMAN_RELEASE_OPTIONS}>")
  #target_compile_options(${exec_name} PRIVATE "$<$<CONFIG:Debug>:${SUBMAN_DEBUG_OPTIONS}>")

  install(TARGETS ${exec_name} DESTINATION bin)
endif()


